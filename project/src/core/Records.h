#pragma once

#include <QString>

// Plain records mirroring the generated CSVs (see tools/datagen). Kept as
// simple structs - the dataset is small enough (a few thousand rows) to hold
// entirely in memory, so no database layer is needed.

struct TelemetryRecord {
    QString timestamp;
    QString machineId;
    QString machineType;
    int machineAgeYears = 0;
    double totalEngineHours = 0;
    QString operatorId;
    QString operatorSkill;
    int operatorExperienceYears = 0;
    QString shift;
    QString taskId;
    double fuelUsedL = 0;
    double fuelLevelPercent = 0;
    int loadCycles = 0;
    double idlingMin = 0;
    double activeMin = 0;
    double engineTempC = 0;
    double hydraulicBar = 0;
    double vibration = 0;
    double tiltDeg = 0;
    double proximityM = 0;
    bool proximityHazard = false;
    QString seatbeltStatus;
    bool alertTriggered = false;
    QString alertType;
    QString weather;
    double ambientTempC = 0;
    QString zone;
};

struct TaskRecord {
    QString taskId;
    QString taskType;
    QString machineId;
    QString machineType;
    int machineAgeYears = 0;
    QString operatorId;
    QString operatorSkill;
    int operatorExperienceYears = 0;
    QString weather;
    double ambientTempC = 0;
    double humidityPercent = 0;
    double windKph = 0;
    QString terrain;
    QString material;
    double loadVolumeM3 = 0;
    double distanceM = 0;
    QString timeOfDay;
    double shiftHoursElapsed = 0;
    int breaks = 0;
    double priorDelayMin = 0;
    double idlingMin = 0;
    double fuelL = 0;
    double estimatedMin = 0;
    double actualMin = 0;
    bool incident = false;
};

struct ScheduleItem {
    QString id;
    QString time;
    QString taskType;
    QString machineId;
    QString operatorId;
    QString priority;
    QString status;
    double estimatedMin = 0;
};

struct OperatorInfo {
    QString id;
    QString name;
    QString skill;
    int experienceYears = 0;
};

struct MachineInfo {
    QString id;
    QString type;
    int ageYears = 0;
    double engineHoursStart = 0;
};

struct IncidentEntry {
    QString timestamp;
    QString machineId;
    QString operatorId;
    QString alertType;
    QString severity;   // normal / warning / critical
    QString note;
    bool manual = false;
};

struct AnomalyEntry {
    QString machineId;
    QString operatorId;
    QString kind;        // "Excessive idling" / "Unsafe operation pattern"
    QString detail;
    double score = 0;    // how far past the fleet baseline, for sorting
};

struct TrainingModule {
    QString id;
    QString title;
    QString type;        // Video / Instructor / Simulation
    QString meta;         // duration or slot label
    QString description;
    int progressPercent = 0;
};
