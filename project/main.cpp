#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "src/core/AnomalyDetector.h"
#include "src/core/DataManager.h"
#include "src/core/SafetyMonitor.h"
#include "src/core/TaskTimeEstimator.h"
#include "src/core/TelemetrySimulator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName("PathBreakers");
    QGuiApplication::setApplicationName("OperatorAssistant");

    DataManager dataManager;
    SafetyMonitor safetyMonitor(&dataManager);
    AnomalyDetector anomalyDetector(&dataManager);
    TaskTimeEstimator taskTimeEstimator(&dataManager);
    TelemetrySimulator telemetrySimulator(&dataManager);

    QQmlApplicationEngine engine;
    QQmlContext* ctx = engine.rootContext();
    ctx->setContextProperty("dataManager", &dataManager);
    ctx->setContextProperty("safetyMonitor", &safetyMonitor);
    ctx->setContextProperty("anomalyDetector", &anomalyDetector);
    ctx->setContextProperty("taskTimeEstimator", &taskTimeEstimator);
    ctx->setContextProperty("telemetrySimulator", &telemetrySimulator);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("project", "Main");

    telemetrySimulator.setRunning(true);

    return QGuiApplication::exec();
}
