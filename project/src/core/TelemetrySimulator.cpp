#include "TelemetrySimulator.h"

#include <QDateTime>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

#include "DataManager.h"

namespace {
template <typename T>
const T& pick(const std::vector<T>& v, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> dist(0, v.size() - 1);
    return v[dist(rng)];
}
double uniformD(double lo, double hi, std::mt19937& rng) {
    return std::uniform_real_distribution<double>(lo, hi)(rng);
}
double gaussD(double mean, double sd, std::mt19937& rng) {
    return std::normal_distribution<double>(mean, sd)(rng);
}
bool chanceD(double p, std::mt19937& rng) {
    return uniformD(0.0, 1.0, rng) < p;
}
} // namespace

TelemetrySimulator::TelemetrySimulator(DataManager* dataManager, QObject* parent)
    : QObject(parent), m_dataManager(dataManager), m_rng(QRandomGenerator::global()->generate()) {
    m_timer.setInterval(4000);
    connect(&m_timer, &QTimer::timeout, this, &TelemetrySimulator::tick);
}

void TelemetrySimulator::setRunning(bool running) {
    if (running == isRunning()) return;
    if (running) m_timer.start(); else m_timer.stop();
    emit runningChanged();
}

void TelemetrySimulator::setIntervalMs(int ms) {
    if (ms == m_timer.interval()) return;
    m_timer.setInterval(ms);
    emit intervalMsChanged();
}

void TelemetrySimulator::tick() {
    const auto& machines = m_dataManager->machines();
    const auto& operators = m_dataManager->operators();
    if (machines.empty() || operators.empty()) return;

    const MachineInfo& m = pick(machines, m_rng);
    const OperatorInfo& op = pick(operators, m_rng);

    static const std::vector<QString> weathers = {"Sunny", "Cloudy", "Rainy", "Windy", "Foggy"};
    static const std::vector<QString> zones = {"Zone-A", "Zone-B", "Zone-C", "Zone-D", "Zone-E"};

    TelemetryRecord r;
    r.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    r.machineId = m.id;
    r.machineType = m.type;
    r.machineAgeYears = m.ageYears;
    r.totalEngineHours = m.engineHoursStart + uniformD(0, 5000, m_rng);
    r.operatorId = op.id;
    r.operatorSkill = op.skill;
    r.operatorExperienceYears = op.experienceYears;
    int hour = QDateTime::currentDateTime().time().hour();
    r.shift = (hour >= 6 && hour < 18) ? "Day" : "Night";
    r.taskId = QString();
    r.weather = pick(weathers, m_rng);
    r.zone = pick(zones, m_rng);
    r.ambientTempC = 18.0 + gaussD(0, 6, m_rng);

    double skillIdleFactor = op.skill == "Beginner" ? 1.3 : op.skill == "Expert" ? 0.8 : 1.0;
    r.idlingMin = std::max(0.0, gaussD(18.0 * skillIdleFactor, 6.0, m_rng));
    r.activeMin = std::max(5.0, 120.0 - r.idlingMin + gaussD(0, 8, m_rng));
    r.loadCycles = std::max(0, int(gaussD(op.skill == "Expert" ? 11 : 8, 3.0, m_rng)));
    r.fuelUsedL = std::max(0.2, (r.activeMin / 60.0) * (3.5 + 0.15 * m.ageYears) + gaussD(0, 0.4, m_rng));
    r.fuelLevelPercent = uniformD(20.0, 100.0, m_rng);
    r.engineTempC = 75.0 + 0.4 * r.loadCycles + gaussD(0, 3, m_rng);
    r.hydraulicBar = gaussD(210.0, 12.0, m_rng) + r.loadCycles * 0.8;
    r.vibration = std::clamp(gaussD(3.0, 1.2, m_rng), 0.0, 10.0);
    r.tiltDeg = std::abs(gaussD(0, 1.5, m_rng)) + (chanceD(0.03, m_rng) ? uniformD(8, 15, m_rng) : 0.0);
    r.proximityM = std::max(0.2, gaussD(12.0, 6.0, m_rng));
    r.proximityHazard = r.proximityM < 2.5;
    bool unfastened = chanceD(0.05, m_rng);
    r.seatbeltStatus = unfastened ? "Unfastened" : "Fastened";

    bool tempAlert = r.engineTempC > 95.0;
    bool tiltAlert = r.tiltDeg > 8.0;
    r.alertTriggered = unfastened || r.proximityHazard || tempAlert || tiltAlert;
    r.alertType = unfastened ? "Seatbelt" : r.proximityHazard ? "Proximity" : tiltAlert ? "Tilt" : tempAlert ? "HighTemp" : "None";

    m_dataManager->addTelemetry(r);
}
