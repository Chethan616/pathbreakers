cmake_minimum_required(VERSION 3.16)

project(project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appproject
    main.cpp
    src/core/DataManager.cpp
    src/core/DataManager.h
    src/core/Records.h
    src/core/SafetyMonitor.cpp
    src/core/SafetyMonitor.h
    src/core/AnomalyDetector.cpp
    src/core/AnomalyDetector.h
    src/core/TaskTimeEstimator.cpp
    src/core/TaskTimeEstimator.h
    src/core/TelemetrySimulator.cpp
    src/core/TelemetrySimulator.h
    src/core/LinearAlgebra.cpp
    src/core/LinearAlgebra.h
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(appproject
    URI project
    QML_FILES
        Main.qml
        qml/Theme.qml
        qml/components/TopBar.qml
        qml/components/MetricCard.qml
        qml/components/AlertDialog.qml
        qml/components/PillButton.qml
        qml/components/LogConsole.qml
        qml/components/Sparkline.qml
        qml/components/IconGlyph.qml
        qml/components/SectionHeader.qml
        qml/pages/DashboardPage.qml
        qml/pages/SafetyPage.qml
        qml/pages/TrainingPage.qml
        qml/pages/AnalyticsPage.qml
        qml/pages/EstimationPage.qml
    RESOURCES
        resources/data/operators.csv
        resources/data/machines.csv
        resources/data/machine_telemetry.csv
        resources/data/task_history.csv
        resources/data/daily_schedule.csv
        resources/training_modules.json
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appproject PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appproject
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appproject
    PRIVATE Qt6::Quick
)

install(TARGETS appproject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
