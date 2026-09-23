#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <array>

#include "LinearAlgebra.h"

class DataManager;

// Predicts task completion time from a multi-feature linear regression
// trained at startup on the bundled task_history dataset (categoricals
// one-hot encoded, numeric features standardized, fit by ordinary least
// squares via LinearAlgebra's normal-equation solver - no external ML
// dependency needed at this dataset size).
class TaskTimeEstimator : public QObject {
    Q_OBJECT
public:
    explicit TaskTimeEstimator(DataManager* dataManager, QObject* parent = nullptr);

    bool isTrained() const { return m_trained; }

    // spec keys: taskType, weather, operatorSkill, terrain, material, timeOfDay (strings),
    // machineAgeYears, operatorExperienceYears, ambientTempC, humidityPercent, windKph,
    // loadVolumeM3, distanceM, shiftHoursElapsed, breaks, priorDelayMin (numbers).
    Q_INVOKABLE double predictMinutes(const QVariantMap& spec) const;
    Q_INVOKABLE double historicalAverageMinutes(const QString& taskType) const;

    Q_INVOKABLE QStringList taskTypes() const { return m_taskTypeLevels; }
    Q_INVOKABLE QStringList weatherConditions() const { return m_weatherLevels; }
    Q_INVOKABLE QStringList skillLevels() const { return m_skillLevels; }
    Q_INVOKABLE QStringList terrainTypes() const { return m_terrainLevels; }
    Q_INVOKABLE QStringList materialTypes() const { return m_materialLevels; }
    Q_INVOKABLE QStringList timesOfDay() const { return m_timeOfDayLevels; }

private:
    static constexpr int kNumericCount = 10;

    void train();
    linalg::Vector buildFeatureVector(const QString& taskType, const QString& weather,
                                       const QString& skill, const QString& terrain,
                                       const QString& material, const QString& timeOfDay,
                                       const std::array<double, kNumericCount>& numeric) const;
    int totalColumns() const;

    DataManager* m_dataManager;
    QStringList m_taskTypeLevels, m_weatherLevels, m_skillLevels, m_terrainLevels, m_materialLevels, m_timeOfDayLevels;
    std::array<double, kNumericCount> m_numericMean{};
    std::array<double, kNumericCount> m_numericStd{};
    linalg::Vector m_coefficients;
    bool m_trained = false;
};
