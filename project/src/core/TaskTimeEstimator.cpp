#include "TaskTimeEstimator.h"

#include <QSet>
#include <cmath>

#include "DataManager.h"

namespace {
QStringList sortedUnique(const QSet<QString>& set) {
    QStringList list(set.begin(), set.end());
    list.sort();
    return list;
}
int dummyCount(const QStringList& levels) { return std::max(0, static_cast<int>(levels.size()) - 1); }
} // namespace

TaskTimeEstimator::TaskTimeEstimator(DataManager* dataManager, QObject* parent)
    : QObject(parent), m_dataManager(dataManager) {
    train();
}

int TaskTimeEstimator::totalColumns() const {
    return 1 // intercept
         + dummyCount(m_taskTypeLevels) + dummyCount(m_weatherLevels) + dummyCount(m_skillLevels)
         + dummyCount(m_terrainLevels) + dummyCount(m_materialLevels) + dummyCount(m_timeOfDayLevels)
         + kNumericCount;
}

linalg::Vector TaskTimeEstimator::buildFeatureVector(const QString& taskType, const QString& weather,
                                                      const QString& skill, const QString& terrain,
                                                      const QString& material, const QString& timeOfDay,
                                                      const std::array<double, kNumericCount>& numeric) const {
    linalg::Vector v(totalColumns(), 0.0);
    int idx = 0;
    v[idx++] = 1.0; // intercept

    auto encode = [&](const QStringList& levels, const QString& value) {
        for (int i = 1; i < levels.size(); ++i) v[idx++] = (levels[i] == value) ? 1.0 : 0.0;
    };
    encode(m_taskTypeLevels, taskType);
    encode(m_weatherLevels, weather);
    encode(m_skillLevels, skill);
    encode(m_terrainLevels, terrain);
    encode(m_materialLevels, material);
    encode(m_timeOfDayLevels, timeOfDay);

    for (int i = 0; i < kNumericCount; ++i) {
        double std = m_numericStd[i] > 1e-9 ? m_numericStd[i] : 1.0;
        v[idx++] = (numeric[i] - m_numericMean[i]) / std;
    }
    return v;
}

void TaskTimeEstimator::train() {
    const auto& tasks = m_dataManager->tasks();
    if (tasks.empty()) { m_trained = false; return; }

    QSet<QString> taskTypeSet, weatherSet, skillSet, terrainSet, materialSet, timeOfDaySet;
    for (const auto& t : tasks) {
        taskTypeSet << t.taskType; weatherSet << t.weather; skillSet << t.operatorSkill;
        terrainSet << t.terrain; materialSet << t.material; timeOfDaySet << t.timeOfDay;
    }
    m_taskTypeLevels = sortedUnique(taskTypeSet);
    m_weatherLevels = sortedUnique(weatherSet);
    m_skillLevels = sortedUnique(skillSet);
    m_terrainLevels = sortedUnique(terrainSet);
    m_materialLevels = sortedUnique(materialSet);
    m_timeOfDayLevels = sortedUnique(timeOfDaySet);

    // Numeric feature order must match buildFeatureVector's `numeric` array.
    auto numericOf = [](const TaskRecord& t) -> std::array<double, kNumericCount> {
        return { double(t.machineAgeYears), double(t.operatorExperienceYears), t.ambientTempC,
                 t.humidityPercent, t.windKph, t.loadVolumeM3, t.distanceM,
                 t.shiftHoursElapsed, double(t.breaks), t.priorDelayMin };
    };

    for (int i = 0; i < kNumericCount; ++i) {
        double sum = 0;
        for (const auto& t : tasks) sum += numericOf(t)[i];
        m_numericMean[i] = sum / tasks.size();
    }
    for (int i = 0; i < kNumericCount; ++i) {
        double sq = 0;
        for (const auto& t : tasks) { double d = numericOf(t)[i] - m_numericMean[i]; sq += d * d; }
        m_numericStd[i] = std::sqrt(sq / tasks.size());
    }

    int cols = totalColumns();
    linalg::Matrix x(static_cast<int>(tasks.size()), cols, 0.0);
    linalg::Vector y(tasks.size(), 0.0);
    for (size_t r = 0; r < tasks.size(); ++r) {
        const TaskRecord& t = tasks[r];
        linalg::Vector row = buildFeatureVector(t.taskType, t.weather, t.operatorSkill, t.terrain,
                                                 t.material, t.timeOfDay, numericOf(t));
        for (int c = 0; c < cols; ++c) x.at(static_cast<int>(r), c) = row[c];
        y[r] = t.actualMin;
    }

    m_coefficients = linalg::fitLeastSquares(x, y);
    m_trained = !m_coefficients.empty();
}

double TaskTimeEstimator::predictMinutes(const QVariantMap& spec) const {
    if (!m_trained)
        return historicalAverageMinutes(spec.value("taskType").toString());

    std::array<double, kNumericCount> numeric = {
        spec.value("machineAgeYears", 3).toDouble(),
        spec.value("operatorExperienceYears", 5).toDouble(),
        spec.value("ambientTempC", 22).toDouble(),
        spec.value("humidityPercent", 50).toDouble(),
        spec.value("windKph", 10).toDouble(),
        spec.value("loadVolumeM3", 25).toDouble(),
        spec.value("distanceM", 150).toDouble(),
        spec.value("shiftHoursElapsed", 2).toDouble(),
        spec.value("breaks", 1).toDouble(),
        spec.value("priorDelayMin", 5).toDouble(),
    };
    linalg::Vector features = buildFeatureVector(
        spec.value("taskType").toString(), spec.value("weather").toString(),
        spec.value("operatorSkill").toString(), spec.value("terrain").toString(),
        spec.value("material").toString(), spec.value("timeOfDay").toString(), numeric);

    double predicted = 0;
    for (size_t i = 0; i < features.size(); ++i) predicted += features[i] * m_coefficients[i];
    return std::max(1.0, predicted);
}

double TaskTimeEstimator::historicalAverageMinutes(const QString& taskType) const {
    const auto& tasks = m_dataManager->tasks();
    double sum = 0; int count = 0;
    for (const auto& t : tasks) {
        if (taskType.isEmpty() || t.taskType == taskType) { sum += t.actualMin; count++; }
    }
    return count > 0 ? sum / count : 45.0;
}
