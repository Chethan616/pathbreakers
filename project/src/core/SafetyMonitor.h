#pragma once

#include <QObject>

#include "Records.h"

class DataManager;

// Turns raw telemetry into safety severities and incident-log entries.
// Thresholds shift with weather/ambient conditions (reduced visibility in
// rain/fog/snow tightens the safe proximity distance; hot weather tightens
// the safe engine-temperature ceiling) - the PDF's "working conditions to be
// considered" note for the safety features.
class SafetyMonitor : public QObject {
    Q_OBJECT
public:
    explicit SafetyMonitor(DataManager* dataManager, QObject* parent = nullptr);

    // "normal" / "warning" / "critical"
    Q_INVOKABLE QString proximitySeverity(double distanceM, const QString& weather) const;
    Q_INVOKABLE QString idlingSeverity(double idlingMin, double activeMin) const;
    Q_INVOKABLE QString tempSeverity(double tempC, const QString& weather) const;
    Q_INVOKABLE QString tiltSeverity(double tiltDeg) const;
    Q_INVOKABLE QString seatbeltSeverity(const QString& status) const;

    Q_INVOKABLE void logManualIncident(const QString& machineId, const QString& operatorId,
                                        const QString& alertType, const QString& note);

signals:
    void alertRaised(const IncidentEntry& entry);

private slots:
    void onTelemetryAdded(const TelemetryRecord& record);

private:
    static bool reducedVisibility(const QString& weather);

    DataManager* m_dataManager;
};
