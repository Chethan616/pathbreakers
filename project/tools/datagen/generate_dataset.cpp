// Standalone synthetic dataset generator for the Smart Operator Assistant.
// No Qt dependency - plain C++17. Run once (or whenever the dataset needs
// regenerating) to (re)populate resources/data/*.csv with a large, internally
// consistent dataset built from realistic correlations (skill -> speed,
// weather/terrain -> delay, machine age -> wear/fuel, fatigue -> risk, etc.)
// so the anomaly detector and task-time estimator have real signal to learn.
//
// Build:   g++ -std=c++17 -O2 -o datagen generate_dataset.cpp
// Run:     ./datagen   (writes into ../../resources/data relative to the exe,
//                       or pass an output directory as argv[1])

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::mt19937 rng(20250501);

static double uniform(double lo, double hi) {
    return std::uniform_real_distribution<double>(lo, hi)(rng);
}
static int uniformInt(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}
static double gauss(double mean, double sd) {
    return std::normal_distribution<double>(mean, sd)(rng);
}
static bool chance(double p) {
    return uniform(0.0, 1.0) < p;
}
template <typename T>
static const T& pick(const std::vector<T>& v) {
    return v[uniformInt(0, static_cast<int>(v.size()) - 1)];
}

struct Operator {
    std::string id;
    std::string name;
    std::string skill;      // Beginner / Intermediate / Expert
    int experienceYears;
};

struct Machine {
    std::string id;
    std::string type;       // Excavator / Loader / Bulldozer / Grader / DumpTruck / Crane
    int ageYears;
    double engineHoursStart;
};

static std::string csvEscape(const std::string& s) {
    if (s.find(',') == std::string::npos && s.find('"') == std::string::npos) return s;
    std::string out = "\"";
    for (char c : s) { if (c == '"') out += "\"\""; else out += c; }
    out += "\"";
    return out;
}

static std::string fmt(double v, int prec = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << v;
    return oss.str();
}

static std::string timestamp(int year, int month, int day, int hour, int minute) {
    std::ostringstream oss;
    oss << year << "-" << std::setw(2) << std::setfill('0') << month << "-"
        << std::setw(2) << std::setfill('0') << day << " "
        << std::setw(2) << std::setfill('0') << hour << ":"
        << std::setw(2) << std::setfill('0') << minute << ":00";
    return oss.str();
}

// Days since 2025-05-01 -> (year, month, day), simple fixed-days-per-month model
// good enough for synthetic data generation (not calendar-accurate across Feb).
static void addDays(int startYear, int startMonth, int startDay, int offset,
                     int& y, int& m, int& d) {
    static const int dim[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    y = startYear; m = startMonth; d = startDay + offset;
    while (d > dim[m]) { d -= dim[m]; m++; if (m > 12) { m = 1; y++; } }
}

int main(int argc, char** argv) {
    fs::path outDir = (argc > 1) ? fs::path(argv[1]) : fs::path("../../resources/data");
    std::error_code ec;
    fs::create_directories(outDir, ec);

    // ---- Operators ----------------------------------------------------
    const std::vector<std::string> skills = {"Beginner", "Intermediate", "Expert"};
    const std::vector<double> skillWeights = {0.30, 0.45, 0.25};
    std::discrete_distribution<int> skillDist(skillWeights.begin(), skillWeights.end());

    const std::vector<std::string> firstNames = {
        "Alex","Sam","Jordan","Casey","Riley","Morgan","Taylor","Jamie","Dana","Robin",
        "Chris","Pat","Drew","Reese","Skyler","Avery","Quinn","Rowan","Sasha","Blair"
    };

    std::vector<Operator> operators;
    for (int i = 1; i <= 20; ++i) {
        Operator op;
        std::ostringstream id; id << "OP" << std::setw(3) << std::setfill('0') << i;
        op.id = id.str();
        op.name = firstNames[i - 1] + " " + std::string(1, char('A' + (i % 26))) + ".";
        op.skill = skills[skillDist(rng)];
        op.experienceYears = op.skill == "Beginner" ? uniformInt(0, 2)
                            : op.skill == "Intermediate" ? uniformInt(3, 7)
                            : uniformInt(8, 20);
        operators.push_back(op);
    }

    // ---- Machines -------------------------------------------------------
    const std::vector<std::string> machineTypes = {
        "Excavator", "Loader", "Bulldozer", "Grader", "DumpTruck", "Crane"
    };
    const std::vector<std::string> machinePrefix = {
        "EXC", "LDR", "BLD", "GRD", "DMP", "CRN"
    };
    std::vector<Machine> machines;
    for (int i = 0; i < 15; ++i) {
        int typeIdx = i % static_cast<int>(machineTypes.size());
        Machine m;
        std::ostringstream id;
        id << machinePrefix[typeIdx] << std::setw(3) << std::setfill('0') << (i / static_cast<int>(machineTypes.size()) + 1);
        m.id = id.str();
        m.type = machineTypes[typeIdx];
        m.ageYears = uniformInt(1, 8);
        m.engineHoursStart = uniform(500.0, 4000.0);
        machines.push_back(m);
    }

    // Deliberately-unusual machine/operator pairs for the anomaly detector to find.
    std::vector<std::pair<std::string, std::string>> problematicPairs;
    for (int i = 0; i < 4; ++i) {
        problematicPairs.emplace_back(pick(machines).id, pick(operators).id);
    }
    auto isProblematic = [&](const std::string& machineId, const std::string& opId) {
        for (auto& p : problematicPairs) if (p.first == machineId && p.second == opId) return true;
        return false;
    };

    // ---- Write operators.csv / machines.csv -----------------------------
    {
        std::ofstream f(outDir / "operators.csv");
        f << "OperatorID,Name,SkillLevel,ExperienceYears\n";
        for (auto& op : operators)
            f << op.id << "," << csvEscape(op.name) << "," << op.skill << "," << op.experienceYears << "\n";
    }
    {
        std::ofstream f(outDir / "machines.csv");
        f << "MachineID,MachineType,AgeYears,EngineHoursStart\n";
        for (auto& m : machines)
            f << m.id << "," << m.type << "," << m.ageYears << "," << fmt(m.engineHoursStart) << "\n";
    }

    // ---- Weather model: one condition + ambient temp per (day, zone) ----
    const std::vector<std::string> weatherConditions = {"Sunny","Cloudy","Rainy","Windy","Foggy","Snowy"};
    const std::vector<double> weatherWeights =           {0.35,   0.25,    0.15,   0.12,   0.08,   0.05};
    std::discrete_distribution<int> weatherDist(weatherWeights.begin(), weatherWeights.end());
    const std::vector<std::string> zones = {"Zone-A","Zone-B","Zone-C","Zone-D","Zone-E"};

    const int startYear = 2025, startMonth = 5, startDay = 1;
    const int totalDays = 60;

    struct DayWeather { std::string condition; double ambientTemp; };
    std::vector<std::array<DayWeather, 5>> weatherByDay(totalDays);
    for (int d = 0; d < totalDays; ++d) {
        for (size_t z = 0; z < zones.size(); ++z) {
            std::string cond = weatherConditions[weatherDist(rng)];
            double baseTemp = 18.0 + 10.0 * std::sin(2 * 3.14159265358979 * d / 30.0); // seasonal-ish wobble
            if (cond == "Snowy") baseTemp -= 15.0;
            else if (cond == "Rainy") baseTemp -= 4.0;
            else if (cond == "Sunny") baseTemp += 4.0;
            weatherByDay[d][z] = {cond, baseTemp + gauss(0.0, 2.0)};
        }
    }

    // ---- machine_telemetry.csv ------------------------------------------
    // One reading every 2 hours per machine, over `totalDays`.
    {
        std::ofstream f(outDir / "machine_telemetry.csv");
        f << "TimestampUtc,MachineID,MachineType,MachineAgeYears,MachineTotalEngineHours,"
             "OperatorID,OperatorSkillLevel,OperatorExperienceYears,ShiftType,TaskID,"
             "FuelUsedL,FuelLevelPercent,LoadCycles,IdlingTimeMin,ActiveTimeMin,"
             "EngineTemperatureC,HydraulicPressureBar,VibrationLevel,TiltAngleDeg,"
             "ProximityDistanceM,ProximityHazardTriggered,SeatbeltStatus,"
             "SafetyAlertTriggered,AlertType,WeatherCondition,AmbientTemperatureC,SiteZoneID\n";

        long taskCounter = 1;
        for (auto& m : machines) {
            double engineHours = m.engineHoursStart;
            double fuelLevel = uniform(70.0, 100.0);
            int zoneIdx = uniformInt(0, static_cast<int>(zones.size()) - 1);

            for (int d = 0; d < totalDays; ++d) {
                const DayWeather& w = weatherByDay[d][zoneIdx];
                for (int hour = 0; hour < 24; hour += 2) {
                    const Operator& op = pick(operators);
                    bool problematic = isProblematic(m.id, op.id);
                    std::string shift = (hour >= 6 && hour < 18) ? "Day" : "Night";

                    double skillIdleFactor = op.skill == "Beginner" ? 1.3 : op.skill == "Expert" ? 0.8 : 1.0;
                    double ageFactor = 1.0 + 0.03 * m.ageYears;

                    double idling = std::max(0.0, gauss(18.0 * skillIdleFactor * ageFactor, 6.0)
                                                    + (problematic ? 25.0 : 0.0)
                                                    + (shift == "Night" ? 5.0 : 0.0));
                    double active = std::max(5.0, 120.0 - idling + gauss(0.0, 8.0));
                    int loadCycles = std::max(0, static_cast<int>(gauss(op.skill == "Expert" ? 11 : op.skill == "Beginner" ? 6 : 8.5, 3.0)));

                    double fuelUsed = std::max(0.2, (active / 60.0) * (3.5 + 0.15 * m.ageYears)
                                                      + (idling / 60.0) * 1.2 + gauss(0.0, 0.4));
                    fuelLevel -= fuelUsed * 0.6;
                    if (fuelLevel < 15.0) fuelLevel = uniform(70.0, 100.0); // refuel event

                    double engineTemp = 75.0 + 0.4 * (loadCycles) + 0.2 * std::max(0.0, w.ambientTemp - 20.0)
                                         + gauss(0.0, 3.0) + (problematic ? uniform(0.0, 6.0) : 0.0);
                    double hydraulicPressure = gauss(210.0, 12.0) + loadCycles * 0.8;
                    double vibration = std::clamp(gauss(3.0 + (m.type == "Bulldozer" ? 1.5 : 0.0), 1.2), 0.0, 10.0);
                    double tilt = std::abs(gauss(0.0, 1.5)) + (problematic && chance(0.05) ? uniform(8.0, 15.0) : 0.0);

                    double proximity = std::max(0.2, gauss(problematic ? 6.0 : 12.0, 6.0));
                    bool proximityHazard = proximity < 2.5;

                    double unfastenedProb = 0.03 + (problematic ? 0.35 : 0.0) + (op.skill == "Beginner" ? 0.05 : 0.0);
                    bool unfastened = chance(unfastenedProb);

                    bool tempAlert = engineTemp > 95.0;
                    bool tiltAlert = tilt > 8.0;
                    bool alertTriggered = unfastened || proximityHazard || tempAlert || tiltAlert;
                    std::string alertType = "None";
                    if (unfastened) alertType = "Seatbelt";
                    else if (proximityHazard) alertType = "Proximity";
                    else if (tiltAlert) alertType = "Tilt";
                    else if (tempAlert) alertType = "HighTemp";

                    engineHours += 2.0;

                    int yy, mm, dd;
                    addDays(startYear, startMonth, startDay, d, yy, mm, dd);

                    f << timestamp(yy, mm, dd, hour, 0) << ","
                      << m.id << "," << m.type << "," << m.ageYears << "," << fmt(engineHours) << ","
                      << op.id << "," << op.skill << "," << op.experienceYears << "," << shift << ","
                      << "T" << std::setw(5) << std::setfill('0') << (taskCounter++ % 2000 + 1) << ","
                      << fmt(fuelUsed) << "," << fmt(std::clamp(fuelLevel, 0.0, 100.0)) << ","
                      << loadCycles << "," << fmt(idling) << "," << fmt(active) << ","
                      << fmt(engineTemp) << "," << fmt(hydraulicPressure) << "," << fmt(vibration) << "," << fmt(tilt, 2) << ","
                      << fmt(proximity) << "," << (proximityHazard ? "Yes" : "No") << ","
                      << (unfastened ? "Unfastened" : "Fastened") << ","
                      << (alertTriggered ? "Yes" : "No") << "," << alertType << ","
                      << w.condition << "," << fmt(w.ambientTemp) << "," << zones[zoneIdx] << "\n";
                }
            }
        }
    }

    // ---- task_history.csv ------------------------------------------------
    const std::vector<std::string> taskTypes = {
        "Earth Excavation","Trenching","Material Loading","Grading","Demolition","Digging","Lifting","Compacting"
    };
    const std::vector<double> baseTimeMin = {60, 45, 30, 35, 90, 50, 40, 55};
    const std::vector<std::string> terrains = {"Flat","Rocky","Muddy","Sandy","Hilly"};
    const std::vector<std::string> materials = {"Soil","Rock","Sand","Concrete","Debris"};
    const std::vector<std::string> timesOfDay = {"Morning","Afternoon","Evening","Night"};

    auto weatherMultiplier = [](const std::string& w) {
        if (w == "Sunny") return 1.0; if (w == "Cloudy") return 1.05; if (w == "Rainy") return 1.15;
        if (w == "Windy") return 1.10; if (w == "Foggy") return 1.20; return 1.30; // Snowy
    };
    auto terrainMultiplier = [](const std::string& t) {
        if (t == "Flat") return 1.0; if (t == "Rocky") return 1.15; if (t == "Muddy") return 1.20;
        if (t == "Sandy") return 1.10; return 1.15; // Hilly
    };
    auto skillSpeedFactor = [](const std::string& s) {
        if (s == "Beginner") return 0.80; if (s == "Expert") return 1.20; return 1.0;
    };

    {
        std::ofstream f(outDir / "task_history.csv");
        f << "TaskID,TaskType,MachineID,MachineType,MachineAgeYears,OperatorID,OperatorSkillLevel,"
             "OperatorExperienceYears,WeatherCondition,AmbientTemperatureC,HumidityPercent,WindSpeedKph,"
             "SiteTerrainType,MaterialType,LoadVolumeM3,DistanceCoveredM,TimeOfDay,ShiftHoursElapsed,"
             "NumberOfBreaks,PriorTaskDelayMin,IdlingDuringTaskMin,FuelConsumedDuringTaskL,"
             "EstimatedTimeMin,ActualTimeMin,SafetyIncidentDuringTask\n";

        for (int i = 1; i <= 2000; ++i) {
            int typeIdx = uniformInt(0, static_cast<int>(taskTypes.size()) - 1);
            const Machine& m = pick(machines);
            const Operator& op = pick(operators);
            std::string weather = weatherConditions[weatherDist(rng)];
            double ambientTemp = 18.0 + gauss(0.0, 6.0) + (weather == "Snowy" ? -15.0 : weather == "Sunny" ? 4.0 : 0.0);
            double humidity = std::clamp(gauss(weather == "Rainy" ? 80.0 : 50.0, 12.0), 10.0, 100.0);
            double wind = std::max(0.0, gauss(weather == "Windy" ? 35.0 : 10.0, 6.0));
            std::string terrain = pick(terrains);
            std::string material = pick(materials);
            double loadVolume = uniform(5.0, 60.0);
            double distance = uniform(20.0, 500.0);
            std::string timeOfDay = pick(timesOfDay);
            double fatigue = uniform(0.0, 10.0); // hours since shift start
            int breaks = uniformInt(0, 3);
            double priorDelay = std::max(0.0, gauss(5.0, 6.0));

            double base = baseTimeMin[typeIdx];
            double wMul = weatherMultiplier(weather);
            double tMul = terrainMultiplier(terrain);
            double ageMul = 1.0 + 0.02 * m.ageYears;
            double skillDiv = skillSpeedFactor(op.skill);

            double estimated = base * wMul * tMul;
            double actual = (base * wMul * tMul * ageMul / skillDiv) + fatigue * 1.5 + gauss(0.0, base * 0.08);
            actual = std::max(5.0, actual);

            double idling = std::max(0.0, gauss(op.skill == "Beginner" ? 12.0 : 6.0, 4.0));
            double fuelUsed = std::max(0.3, actual / 60.0 * (3.0 + 0.1 * m.ageYears) + gauss(0.0, 0.3));

            double incidentProb = 0.03
                + (op.skill == "Beginner" ? 0.05 : 0.0)
                + (fatigue > 6.0 ? 0.04 : 0.0)
                + ((weather == "Rainy" || weather == "Foggy" || weather == "Snowy") ? 0.03 : 0.0)
                + ((terrain == "Muddy" || terrain == "Rocky") ? 0.05 : 0.0);
            bool incident = chance(incidentProb);

            f << "T" << std::setw(5) << std::setfill('0') << i << ","
              << taskTypes[typeIdx] << "," << m.id << "," << m.type << "," << m.ageYears << ","
              << op.id << "," << op.skill << "," << op.experienceYears << ","
              << weather << "," << fmt(ambientTemp) << "," << fmt(humidity) << "," << fmt(wind) << ","
              << terrain << "," << material << "," << fmt(loadVolume) << "," << fmt(distance) << ","
              << timeOfDay << "," << fmt(fatigue) << "," << breaks << "," << fmt(priorDelay) << ","
              << fmt(idling) << "," << fmt(fuelUsed) << ","
              << fmt(estimated) << "," << fmt(actual) << "," << (incident ? "Yes" : "No") << "\n";
        }
    }

    // ---- daily_schedule.csv: "today's" task board for the dashboard -----
    {
        const std::vector<std::string> statuses = {"Scheduled", "InProgress", "Completed"};
        const std::vector<std::string> priorities = {"Normal", "High"};
        std::ofstream f(outDir / "daily_schedule.csv");
        f << "ScheduleID,ScheduledTime,TaskType,MachineID,OperatorID,Priority,Status,EstimatedTimeMin\n";
        int hour = 6;
        for (int i = 1; i <= 9; ++i) {
            const Machine& m = pick(machines);
            const Operator& op = pick(operators);
            std::string type = pick(taskTypes);
            std::string status = i <= 3 ? "Completed" : (i <= 5 ? "InProgress" : "Scheduled");
            std::string priority = chance(0.25) ? "High" : "Normal";
            double est = baseTimeMin[std::find(taskTypes.begin(), taskTypes.end(), type) - taskTypes.begin()];
            std::ostringstream sid; sid << "SCH" << std::setw(3) << std::setfill('0') << i;
            f << sid.str() << "," << std::setw(2) << std::setfill('0') << hour << ":00,"
              << type << "," << m.id << "," << op.id << "," << priority << "," << status << "," << fmt(est) << "\n";
            hour += uniformInt(1, 2);
        }
    }

    std::printf("Generated dataset into %s\n", outDir.string().c_str());
    std::printf("  operators.csv          : %d rows\n", static_cast<int>(operators.size()));
    std::printf("  machines.csv           : %d rows\n", static_cast<int>(machines.size()));
    std::printf("  machine_telemetry.csv  : %d rows\n", static_cast<int>(machines.size() * totalDays * 12));
    std::printf("  task_history.csv       : 2000 rows\n");
    std::printf("  injected anomalous machine/operator pairs: %d\n", static_cast<int>(problematicPairs.size()));
    for (auto& p : problematicPairs) std::printf("    %s / %s\n", p.first.c_str(), p.second.c_str());
    return 0;
}
