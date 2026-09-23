#pragma once

#include <QObject>
#include <QVariantList>
#include <vector>

#include "Records.h"

class DataManager;

// Flags machine/operator pairs whose idling ratio or safety-alert rate sits
// well outside the fleet baseline (rule + z-score on the bundled dataset) -
// "identify unusual behavior... excessive idling or unsafe operation
// patterns" from the brief.
class AnomalyDetector : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList anomalyList READ anomalyList NOTIFY anomaliesChanged)
public:
    explicit AnomalyDetector(DataManager* dataManager, QObject* parent = nullptr);

    const std::vector<AnomalyEntry>& anomalies() const { return m_anomalies; }
    QVariantList anomalyList() const;
    Q_INVOKABLE void recompute();

signals:
    void anomaliesChanged();

private:
    DataManager* m_dataManager;
    std::vector<AnomalyEntry> m_anomalies;
};
