#include "DataManager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QStandardPaths>
#include <QTextStream>
#include <algorithm>

namespace {

// Resources declared via qt_add_qml_module(... RESOURCES resources/...) land
// under :/qt/qml/<uri>/<path-relative-to-the-module-dir>.
const QString kResourceBase = ":/qt/qml/project/resources";

QStringList splitCsvLine(const QString& line) {
    // Minimal RFC4180-ish splitter: handles quoted fields with embedded commas.
    QStringList fields;
    QString current;
    bool inQuotes = false;
    for (int i = 0; i < line.size(); ++i) {
        QChar c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { current += '"'; ++i; }
                else inQuotes = false;
            } else current += c;
        } else {
            if (c == '"') inQuotes = true;
            else if (c == ',') { fields << current; current.clear(); }
            else current += c;
        }
    }
    fields << current;
    return fields;
}

struct CsvTable {
    QStringList headers;
    QList<QStringList> rows;
};

CsvTable readCsv(const QString& resourcePath) {
    CsvTable table;
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return table;
    QTextStream stream(&file);
    bool first = true;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.isEmpty()) continue;
        if (first) { table.headers = splitCsvLine(line); first = false; continue; }
        table.rows << splitCsvLine(line);
    }
    return table;
}

// Index lookup helper: value of `col` on `row`, using the header list built once per file.
QString field(const QStringList& headers, const QStringList& row, const char* name) {
    int idx = headers.indexOf(QLatin1String(name));
    return (idx >= 0 && idx < row.size()) ? row[idx] : QString();
}

double fieldD(const QStringList& headers, const QStringList& row, const char* name) {
    return field(headers, row, name).toDouble();
}
int fieldI(const QStringList& headers, const QStringList& row, const char* name) {
    return field(headers, row, name).toInt();
}
bool fieldYesNo(const QStringList& headers, const QStringList& row, const char* name) {
    return field(headers, row, name).compare(QLatin1String("Yes"), Qt::CaseInsensitive) == 0;
}

} // namespace

DataManager::DataManager(QObject* parent) : QObject(parent) {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    m_stateFilePath = dir + "/operator_assistant_state.json";

    loadOperators();
    loadMachines();
    loadTelemetry();
    loadTasks();
    loadSchedule();
    loadTrainingModules();
    loadPersistedState();
}

void DataManager::loadOperators() {
    CsvTable t = readCsv(kResourceBase + "/data/operators.csv");
    for (auto& row : t.rows) {
        OperatorInfo op;
        op.id = field(t.headers, row, "OperatorID");
        op.name = field(t.headers, row, "Name");
        op.skill = field(t.headers, row, "SkillLevel");
        op.experienceYears = fieldI(t.headers, row, "ExperienceYears");
        m_operators.push_back(op);
    }
}

void DataManager::loadMachines() {
    CsvTable t = readCsv(kResourceBase + "/data/machines.csv");
    for (auto& row : t.rows) {
        MachineInfo m;
        m.id = field(t.headers, row, "MachineID");
        m.type = field(t.headers, row, "MachineType");
        m.ageYears = fieldI(t.headers, row, "AgeYears");
        m.engineHoursStart = fieldD(t.headers, row, "EngineHoursStart");
        m_machines.push_back(m);
    }
}

void DataManager::loadTelemetry() {
    CsvTable t = readCsv(kResourceBase + "/data/machine_telemetry.csv");
    m_telemetry.reserve(t.rows.size());
    for (auto& row : t.rows) {
        TelemetryRecord r;
        r.timestamp = field(t.headers, row, "TimestampUtc");
        r.machineId = field(t.headers, row, "MachineID");
        r.machineType = field(t.headers, row, "MachineType");
        r.machineAgeYears = fieldI(t.headers, row, "MachineAgeYears");
        r.totalEngineHours = fieldD(t.headers, row, "MachineTotalEngineHours");
        r.operatorId = field(t.headers, row, "OperatorID");
        r.operatorSkill = field(t.headers, row, "OperatorSkillLevel");
        r.operatorExperienceYears = fieldI(t.headers, row, "OperatorExperienceYears");
        r.shift = field(t.headers, row, "ShiftType");
        r.taskId = field(t.headers, row, "TaskID");
        r.fuelUsedL = fieldD(t.headers, row, "FuelUsedL");
        r.fuelLevelPercent = fieldD(t.headers, row, "FuelLevelPercent");
        r.loadCycles = fieldI(t.headers, row, "LoadCycles");
        r.idlingMin = fieldD(t.headers, row, "IdlingTimeMin");
        r.activeMin = fieldD(t.headers, row, "ActiveTimeMin");
        r.engineTempC = fieldD(t.headers, row, "EngineTemperatureC");
        r.hydraulicBar = fieldD(t.headers, row, "HydraulicPressureBar");
        r.vibration = fieldD(t.headers, row, "VibrationLevel");
        r.tiltDeg = fieldD(t.headers, row, "TiltAngleDeg");
        r.proximityM = fieldD(t.headers, row, "ProximityDistanceM");
        r.proximityHazard = fieldYesNo(t.headers, row, "ProximityHazardTriggered");
        r.seatbeltStatus = field(t.headers, row, "SeatbeltStatus");
        r.alertTriggered = fieldYesNo(t.headers, row, "SafetyAlertTriggered");
        r.alertType = field(t.headers, row, "AlertType");
        r.weather = field(t.headers, row, "WeatherCondition");
        r.ambientTempC = fieldD(t.headers, row, "AmbientTemperatureC");
        r.zone = field(t.headers, row, "SiteZoneID");
        m_telemetry.push_back(r);

        if (r.alertTriggered) {
            IncidentEntry inc;
            inc.timestamp = r.timestamp;
            inc.machineId = r.machineId;
            inc.operatorId = r.operatorId;
            inc.alertType = r.alertType;
            inc.severity = (r.alertType == "Seatbelt" || r.alertType == "Proximity") ? "critical" : "warning";
            inc.note = QStringLiteral("Auto-detected from telemetry.");
            inc.manual = false;
            m_incidents.push_back(inc);
        }
    }
}

void DataManager::loadTasks() {
    CsvTable t = readCsv(kResourceBase + "/data/task_history.csv");
    m_tasks.reserve(t.rows.size());
    for (auto& row : t.rows) {
        TaskRecord r;
        r.taskId = field(t.headers, row, "TaskID");
        r.taskType = field(t.headers, row, "TaskType");
        r.machineId = field(t.headers, row, "MachineID");
        r.machineType = field(t.headers, row, "MachineType");
        r.machineAgeYears = fieldI(t.headers, row, "MachineAgeYears");
        r.operatorId = field(t.headers, row, "OperatorID");
        r.operatorSkill = field(t.headers, row, "OperatorSkillLevel");
        r.operatorExperienceYears = fieldI(t.headers, row, "OperatorExperienceYears");
        r.weather = field(t.headers, row, "WeatherCondition");
        r.ambientTempC = fieldD(t.headers, row, "AmbientTemperatureC");
        r.humidityPercent = fieldD(t.headers, row, "HumidityPercent");
        r.windKph = fieldD(t.headers, row, "WindSpeedKph");
        r.terrain = field(t.headers, row, "SiteTerrainType");
        r.material = field(t.headers, row, "MaterialType");
        r.loadVolumeM3 = fieldD(t.headers, row, "LoadVolumeM3");
        r.distanceM = fieldD(t.headers, row, "DistanceCoveredM");
        r.timeOfDay = field(t.headers, row, "TimeOfDay");
        r.shiftHoursElapsed = fieldD(t.headers, row, "ShiftHoursElapsed");
        r.breaks = fieldI(t.headers, row, "NumberOfBreaks");
        r.priorDelayMin = fieldD(t.headers, row, "PriorTaskDelayMin");
        r.idlingMin = fieldD(t.headers, row, "IdlingDuringTaskMin");
        r.fuelL = fieldD(t.headers, row, "FuelConsumedDuringTaskL");
        r.estimatedMin = fieldD(t.headers, row, "EstimatedTimeMin");
        r.actualMin = fieldD(t.headers, row, "ActualTimeMin");
        r.incident = fieldYesNo(t.headers, row, "SafetyIncidentDuringTask");
        m_tasks.push_back(r);
    }
}

void DataManager::loadSchedule() {
    CsvTable t = readCsv(kResourceBase + "/data/daily_schedule.csv");
    for (auto& row : t.rows) {
        ScheduleItem s;
        s.id = field(t.headers, row, "ScheduleID");
        s.time = field(t.headers, row, "ScheduledTime");
        s.taskType = field(t.headers, row, "TaskType");
        s.machineId = field(t.headers, row, "MachineID");
        s.operatorId = field(t.headers, row, "OperatorID");
        s.priority = field(t.headers, row, "Priority");
        s.status = field(t.headers, row, "Status");
        s.estimatedMin = fieldD(t.headers, row, "EstimatedTimeMin");
        m_schedule.push_back(s);
    }
}

void DataManager::loadTrainingModules() {
    QFile file(kResourceBase + "/training_modules.json");
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    for (const QJsonValue& v : doc.array()) {
        QJsonObject o = v.toObject();
        TrainingModule m;
        m.id = o.value("id").toString();
        m.title = o.value("title").toString();
        m.type = o.value("type").toString();
        m.meta = o.value("meta").toString();
        m.description = o.value("description").toString();
        m.progressPercent = o.value("progressPercent").toInt();
        m_trainingModules.push_back(m);
    }
}

void DataManager::loadPersistedState() {
    QFile file(m_stateFilePath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    for (const QJsonValue& v : root.value("manualIncidents").toArray()) {
        QJsonObject o = v.toObject();
        IncidentEntry inc;
        inc.timestamp = o.value("timestamp").toString();
        inc.machineId = o.value("machineId").toString();
        inc.operatorId = o.value("operatorId").toString();
        inc.alertType = o.value("alertType").toString();
        inc.severity = o.value("severity").toString();
        inc.note = o.value("note").toString();
        inc.manual = true;
        m_incidents.push_back(inc);
    }
    for (const QJsonValue& v : root.value("bookedSlots").toArray())
        m_bookedSlots << v.toString();
}

void DataManager::savePersistedState() const {
    QJsonArray manualIncidents;
    for (const auto& inc : m_incidents) {
        if (!inc.manual) continue;
        QJsonObject o;
        o["timestamp"] = inc.timestamp;
        o["machineId"] = inc.machineId;
        o["operatorId"] = inc.operatorId;
        o["alertType"] = inc.alertType;
        o["severity"] = inc.severity;
        o["note"] = inc.note;
        manualIncidents.append(o);
    }
    QJsonArray bookedSlots;
    for (const auto& s : m_bookedSlots) bookedSlots.append(s);

    QJsonObject root;
    root["manualIncidents"] = manualIncidents;
    root["bookedSlots"] = bookedSlots;

    QFile file(m_stateFilePath);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void DataManager::addIncident(const IncidentEntry& entry) {
    m_incidents.push_back(entry);
    savePersistedState();
    emit incidentAdded(entry);
    emit incidentsChanged();
}

void DataManager::addTelemetry(const TelemetryRecord& record) {
    m_telemetry.push_back(record);
    emit telemetryAdded(record);
    emit telemetryChanged();
}

QVariantList DataManager::incidentList() const {
    QVariantList list;
    for (auto it = m_incidents.rbegin(); it != m_incidents.rend(); ++it) {
        QVariantMap m;
        m["timestamp"] = it->timestamp;
        m["machineId"] = it->machineId;
        m["operatorId"] = it->operatorId;
        m["alertType"] = it->alertType;
        m["severity"] = it->severity;
        m["note"] = it->note;
        m["manual"] = it->manual;
        list << m;
    }
    return list;
}

QVariantList DataManager::latestTelemetryList() const {
    QMap<QString, const TelemetryRecord*> latestByMachine;
    for (const auto& r : m_telemetry) latestByMachine[r.machineId] = &r;

    QVariantList list;
    for (const auto& m : m_machines) {
        auto it = latestByMachine.find(m.id);
        if (it == latestByMachine.end()) continue;
        const TelemetryRecord* r = it.value();
        QVariantMap v;
        v["machineId"] = r->machineId;
        v["machineType"] = r->machineType;
        v["operatorId"] = r->operatorId;
        v["operatorSkill"] = r->operatorSkill;
        v["timestamp"] = r->timestamp;
        v["idlingMin"] = r->idlingMin;
        v["activeMin"] = r->activeMin;
        v["fuelLevelPercent"] = r->fuelLevelPercent;
        v["engineTempC"] = r->engineTempC;
        v["proximityM"] = r->proximityM;
        v["tiltDeg"] = r->tiltDeg;
        v["seatbeltStatus"] = r->seatbeltStatus;
        v["weather"] = r->weather;
        v["alertTriggered"] = r->alertTriggered;
        v["alertType"] = r->alertType;
        list << v;
    }
    return list;
}

QVariantList DataManager::scheduleList() const {
    QVariantList list;
    for (const auto& s : m_schedule) {
        QVariantMap m;
        m["id"] = s.id;
        m["time"] = s.time;
        m["taskType"] = s.taskType;
        m["machineId"] = s.machineId;
        m["operatorId"] = s.operatorId;
        m["priority"] = s.priority;
        m["status"] = s.status;
        m["estimatedMin"] = s.estimatedMin;
        list << m;
    }
    return list;
}

QVariantList DataManager::trainingModuleList() const {
    QVariantList list;
    for (const auto& t : m_trainingModules) {
        QVariantMap m;
        m["id"] = t.id;
        m["title"] = t.title;
        m["type"] = t.type;
        m["meta"] = t.meta;
        m["description"] = t.description;
        m["progressPercent"] = t.progressPercent;
        m["booked"] = isSlotBooked(t.id);
        list << m;
    }
    return list;
}

QVariantList DataManager::machineList() const {
    QVariantList list;
    for (const auto& m : m_machines) {
        QVariantMap v;
        v["id"] = m.id;
        v["type"] = m.type;
        v["ageYears"] = m.ageYears;
        list << v;
    }
    return list;
}

QVariantList DataManager::operatorList() const {
    QVariantList list;
    for (const auto& o : m_operators) {
        QVariantMap v;
        v["id"] = o.id;
        v["name"] = o.name;
        v["skill"] = o.skill;
        v["experienceYears"] = o.experienceYears;
        list << v;
    }
    return list;
}

QVariantList DataManager::telemetryHistoryFor(const QString& machineId, int maxPoints) const {
    QVariantList list;
    int start = std::max(0, static_cast<int>(m_telemetry.size()) - 1);
    std::vector<const TelemetryRecord*> matches;
    for (const auto& r : m_telemetry) if (r.machineId == machineId) matches.push_back(&r);
    int from = std::max(0, static_cast<int>(matches.size()) - maxPoints);
    for (int i = from; i < static_cast<int>(matches.size()); ++i) {
        QVariantMap v;
        v["idlingMin"] = matches[i]->idlingMin;
        v["fuelUsedL"] = matches[i]->fuelUsedL;
        v["engineTempC"] = matches[i]->engineTempC;
        list << v;
    }
    Q_UNUSED(start);
    return list;
}

QVariantMap DataManager::dashboardSummary() const {
    QVariantMap m;
    m["machineCount"] = static_cast<int>(m_machines.size());
    m["operatorCount"] = static_cast<int>(m_operators.size());

    int alertCount = 0;
    double idlingSum = 0;
    for (const auto& r : m_telemetry) {
        if (r.alertTriggered) alertCount++;
        idlingSum += r.idlingMin;
    }
    m["totalAlerts"] = alertCount;
    m["avgIdlingMin"] = m_telemetry.empty() ? 0.0 : idlingSum / m_telemetry.size();

    int completed = 0, inProgress = 0, scheduled = 0;
    for (const auto& s : m_schedule) {
        if (s.status == "Completed") completed++;
        else if (s.status == "InProgress") inProgress++;
        else scheduled++;
    }
    m["tasksCompleted"] = completed;
    m["tasksInProgress"] = inProgress;
    m["tasksScheduled"] = scheduled;
    return m;
}

bool DataManager::isSlotBooked(const QString& slotId) const {
    return m_bookedSlots.contains(slotId);
}

void DataManager::bookSlot(const QString& slotId) {
    if (m_bookedSlots.contains(slotId)) return;
    m_bookedSlots << slotId;
    savePersistedState();
    emit bookingsChanged();
}
