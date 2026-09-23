#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <vector>

#include "Records.h"

// Loads the bundled synthetic dataset (Qt resources, written by
// tools/datagen) into memory once at startup, and keeps the mutable
// session-only records (incident log, training bookings) persisted to a
// small JSON file under the app's data directory so they survive restarts.
class DataManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList incidentList READ incidentList NOTIFY incidentsChanged)
    Q_PROPERTY(QVariantList latestTelemetryList READ latestTelemetryList NOTIFY telemetryChanged)
public:
    explicit DataManager(QObject* parent = nullptr);

    const std::vector<TelemetryRecord>& telemetry() const { return m_telemetry; }
    const std::vector<TaskRecord>& tasks() const { return m_tasks; }
    const std::vector<ScheduleItem>& schedule() const { return m_schedule; }
    const std::vector<OperatorInfo>& operators() const { return m_operators; }
    const std::vector<MachineInfo>& machines() const { return m_machines; }
    const std::vector<TrainingModule>& trainingModules() const { return m_trainingModules; }
    const std::vector<IncidentEntry>& incidents() const { return m_incidents; }

    void addIncident(const IncidentEntry& entry);
    void addTelemetry(const TelemetryRecord& record); // used by the live simulator

    // Bookkeeping for the training hub's instructor-booking calendar:
    // a booked slot id persists across runs.
    Q_INVOKABLE bool isSlotBooked(const QString& slotId) const;
    Q_INVOKABLE void bookSlot(const QString& slotId);

    // QML-friendly list/summary accessors (small dataset - plain QVariantList
    // conversions are simpler here than a full QAbstractListModel per list).
    QVariantList incidentList() const;
    QVariantList latestTelemetryList() const; // most recent reading per machine
    Q_INVOKABLE QVariantList scheduleList() const;
    Q_INVOKABLE QVariantList trainingModuleList() const;
    Q_INVOKABLE QVariantList machineList() const;
    Q_INVOKABLE QVariantList operatorList() const;
    Q_INVOKABLE QVariantList telemetryHistoryFor(const QString& machineId, int maxPoints = 24) const;
    Q_INVOKABLE QVariantMap dashboardSummary() const;

signals:
    void incidentAdded(const IncidentEntry& entry);
    void telemetryAdded(const TelemetryRecord& record);
    void bookingsChanged();
    void incidentsChanged();
    void telemetryChanged();

private:
    void loadOperators();
    void loadMachines();
    void loadTelemetry();
    void loadTasks();
    void loadSchedule();
    void loadTrainingModules();
    void loadPersistedState();
    void savePersistedState() const;

    std::vector<TelemetryRecord> m_telemetry;
    std::vector<TaskRecord> m_tasks;
    std::vector<ScheduleItem> m_schedule;
    std::vector<OperatorInfo> m_operators;
    std::vector<MachineInfo> m_machines;
    std::vector<TrainingModule> m_trainingModules;
    std::vector<IncidentEntry> m_incidents;
    QStringList m_bookedSlots;

    QString m_stateFilePath;
};
