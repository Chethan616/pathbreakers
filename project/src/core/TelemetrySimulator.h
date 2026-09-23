#pragma once

#include <QObject>
#include <QTimer>
#include <random>

class DataManager;

// Stands in for a live machine-sensor feed: on each tick it synthesizes one
// new telemetry reading (same shape/correlations as tools/datagen, just
// evaluated live) for a random machine/operator pair and hands it to
// DataManager, so the Safety and Analytics pages update in real time during
// a demo without needing real hardware.
class TelemetrySimulator : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int intervalMs READ intervalMs WRITE setIntervalMs NOTIFY intervalMsChanged)
public:
    explicit TelemetrySimulator(DataManager* dataManager, QObject* parent = nullptr);

    bool isRunning() const { return m_timer.isActive(); }
    int intervalMs() const { return m_timer.interval(); }

public slots:
    void setRunning(bool running);
    void setIntervalMs(int ms);
    void tick();

signals:
    void runningChanged();
    void intervalMsChanged();

private:
    DataManager* m_dataManager;
    QTimer m_timer;
    std::mt19937 m_rng;
};
