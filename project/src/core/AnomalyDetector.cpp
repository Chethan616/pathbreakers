#include "AnomalyDetector.h"

#include <QMap>
#include <QString>
#include <QVariantMap>
#include <algorithm>
#include <cmath>

#include "DataManager.h"

namespace {
struct PairStats {
    QString machineId;
    QString operatorId;
    int count = 0;
    double idlingSum = 0;
    double idlingSumSq = 0;
    int alertCount = 0;
};
} // namespace

AnomalyDetector::AnomalyDetector(DataManager* dataManager, QObject* parent)
    : QObject(parent), m_dataManager(dataManager) {
    recompute();
    connect(m_dataManager, &DataManager::telemetryAdded, this, &AnomalyDetector::recompute);
}

void AnomalyDetector::recompute() {
    m_anomalies.clear();
    const auto& telemetry = m_dataManager->telemetry();
    if (telemetry.empty()) { emit anomaliesChanged(); return; }

    QMap<QString, PairStats> byPair;
    double fleetIdlingSum = 0;
    int fleetAlertCount = 0;

    for (const auto& r : telemetry) {
        QString key = r.machineId + "|" + r.operatorId;
        PairStats& s = byPair[key];
        s.machineId = r.machineId;
        s.operatorId = r.operatorId;
        s.count++;
        s.idlingSum += r.idlingMin;
        s.idlingSumSq += r.idlingMin * r.idlingMin;
        if (r.alertTriggered) s.alertCount++;

        fleetIdlingSum += r.idlingMin;
        if (r.alertTriggered) fleetAlertCount++;
    }

    double fleetIdlingAvg = fleetIdlingSum / telemetry.size();
    double fleetAlertRate = static_cast<double>(fleetAlertCount) / telemetry.size();

    // Fleet-wide std-dev of per-pair idling averages, for a z-score-ish cutoff.
    double sumAvg = 0, sumAvgSq = 0;
    int pairCount = 0;
    for (auto it = byPair.constBegin(); it != byPair.constEnd(); ++it) {
        if (it->count < 5) continue;
        double avg = it->idlingSum / it->count;
        sumAvg += avg; sumAvgSq += avg * avg; pairCount++;
    }
    double meanOfAvgs = pairCount ? sumAvg / pairCount : fleetIdlingAvg;
    double variance = pairCount ? std::max(0.0, sumAvgSq / pairCount - meanOfAvgs * meanOfAvgs) : 1.0;
    double stddevOfAvgs = std::sqrt(variance);

    for (auto it = byPair.constBegin(); it != byPair.constEnd(); ++it) {
        const PairStats& s = *it;
        if (s.count < 5) continue; // not enough samples to judge

        double idlingAvg = s.idlingSum / s.count;
        double alertRate = static_cast<double>(s.alertCount) / s.count;

        bool excessiveIdling = idlingAvg > meanOfAvgs + 1.5 * std::max(stddevOfAvgs, 1.0);
        bool unsafePattern = alertRate > std::max(fleetAlertRate * 2.5, 0.15);

        if (excessiveIdling) {
            AnomalyEntry a;
            a.machineId = s.machineId;
            a.operatorId = s.operatorId;
            a.kind = "Excessive idling";
            a.detail = QString("Averages %1 min idling per reading vs. fleet average %2 min (%3 readings).")
                           .arg(idlingAvg, 0, 'f', 1)
                           .arg(fleetIdlingAvg, 0, 'f', 1)
                           .arg(s.count);
            a.score = idlingAvg - fleetIdlingAvg;
            m_anomalies.push_back(a);
        }
        if (unsafePattern) {
            AnomalyEntry a;
            a.machineId = s.machineId;
            a.operatorId = s.operatorId;
            a.kind = "Unsafe operation pattern";
            a.detail = QString("Safety alerts on %1% of readings vs. fleet average %2% (%3 readings).")
                           .arg(alertRate * 100.0, 0, 'f', 0)
                           .arg(fleetAlertRate * 100.0, 0, 'f', 0)
                           .arg(s.count);
            a.score = alertRate * 100.0;
            m_anomalies.push_back(a);
        }
    }

    std::sort(m_anomalies.begin(), m_anomalies.end(),
              [](const AnomalyEntry& a, const AnomalyEntry& b) { return a.score > b.score; });

    emit anomaliesChanged();
}

QVariantList AnomalyDetector::anomalyList() const {
    QVariantList list;
    for (const auto& a : m_anomalies) {
        QVariantMap m;
        m["machineId"] = a.machineId;
        m["operatorId"] = a.operatorId;
        m["kind"] = a.kind;
        m["detail"] = a.detail;
        m["score"] = a.score;
        list << m;
    }
    return list;
}
