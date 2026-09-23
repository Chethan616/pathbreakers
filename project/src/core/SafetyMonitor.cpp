#include "SafetyMonitor.h"

#include <QDateTime>

#include "DataManager.h"

SafetyMonitor::SafetyMonitor(DataManager* dataManager, QObject* parent)
    : QObject(parent), m_dataManager(dataManager) {
    connect(m_dataManager, &DataManager::telemetryAdded, this, &SafetyMonitor::onTelemetryAdded);
}

bool SafetyMonitor::reducedVisibility(const QString& weather) {
    return weather == "Foggy" || weather == "Rainy" || weather == "Snowy";
}

QString SafetyMonitor::proximitySeverity(double distanceM, const QString& weather) const {
    double criticalAt = reducedVisibility(weather) ? 4.0 : 2.5;
    double warningAt = criticalAt * 2.0;
    if (distanceM < criticalAt) return "critical";
    if (distanceM < warningAt) return "warning";
    return "normal";
}

QString SafetyMonitor::idlingSeverity(double idlingMin, double activeMin) const {
    double total = idlingMin + activeMin;
    double ratio = total > 0 ? idlingMin / total : 0.0;
    if (ratio > 0.5) return "critical";
    if (ratio > 0.3) return "warning";
    return "normal";
}

QString SafetyMonitor::tempSeverity(double tempC, const QString& weather) const {
    double criticalAt = weather == "Sunny" ? 92.0 : 95.0;
    double warningAt = criticalAt - 8.0;
    if (tempC >= criticalAt) return "critical";
    if (tempC >= warningAt) return "warning";
    return "normal";
}

QString SafetyMonitor::tiltSeverity(double tiltDeg) const {
    if (tiltDeg > 8.0) return "critical";
    if (tiltDeg > 4.0) return "warning";
    return "normal";
}

QString SafetyMonitor::seatbeltSeverity(const QString& status) const {
    return status.compare("Unfastened", Qt::CaseInsensitive) == 0 ? "critical" : "normal";
}

void SafetyMonitor::logManualIncident(const QString& machineId, const QString& operatorId,
                                       const QString& alertType, const QString& note) {
    IncidentEntry entry;
    entry.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    entry.machineId = machineId;
    entry.operatorId = operatorId;
    entry.alertType = alertType.isEmpty() ? QStringLiteral("Manual") : alertType;
    entry.severity = "warning";
    entry.note = note;
    entry.manual = true;
    m_dataManager->addIncident(entry);
    emit alertRaised(entry);
}

void SafetyMonitor::onTelemetryAdded(const TelemetryRecord& record) {
    if (!record.alertTriggered) return;
    IncidentEntry entry;
    entry.timestamp = record.timestamp;
    entry.machineId = record.machineId;
    entry.operatorId = record.operatorId;
    entry.alertType = record.alertType;
    entry.severity = (record.alertType == "Seatbelt" || record.alertType == "Proximity") ? "critical" : "warning";
    entry.note = QStringLiteral("Live telemetry alert.");
    entry.manual = false;
    m_dataManager->addIncident(entry);
    emit alertRaised(entry);
}
