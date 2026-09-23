import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Task time estimation: predicts completion time from a task spec using the
// regression trained on the bundled task_history dataset, shown next to the
// plain historical average for the same task type.
Item {
    id: root

    property string taskType: taskTimeEstimator.taskTypes().length > 0 ? taskTimeEstimator.taskTypes()[0] : ""
    property string weather: taskTimeEstimator.weatherConditions().length > 0 ? taskTimeEstimator.weatherConditions()[0] : ""
    property string skill: taskTimeEstimator.skillLevels().length > 0 ? taskTimeEstimator.skillLevels()[0] : ""
    property string terrain: taskTimeEstimator.terrainTypes().length > 0 ? taskTimeEstimator.terrainTypes()[0] : ""
    property string material: taskTimeEstimator.materialTypes().length > 0 ? taskTimeEstimator.materialTypes()[0] : ""
    property string timeOfDay: taskTimeEstimator.timesOfDay().length > 0 ? taskTimeEstimator.timesOfDay()[0] : ""

    property real machineAge: 3
    property real fatigueHours: 2
    property real priorDelay: 5

    property real predicted: taskTimeEstimator.predictMinutes({
        "taskType": taskType, "weather": weather, "operatorSkill": skill,
        "terrain": terrain, "material": material, "timeOfDay": timeOfDay,
        "machineAgeYears": machineAge, "shiftHoursElapsed": fatigueHours, "priorDelayMin": priorDelay
    })
    property real historical: taskTimeEstimator.historicalAverageMinutes(taskType)

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.space30
        contentHeight: content.height
        clip: true

        Column {
            id: content
            width: parent.width
            spacing: Theme.space30

            SectionHeader { text: "TASK TIME ESTIMATION" }

            Rectangle {
                width: parent.width
                radius: Theme.radiusLg
                color: Theme.surfaceCard
                border.color: Theme.borderPanel
                border.width: 2
                height: form.height + Theme.space30 * 2

                GridLayout {
                    id: form
                    x: Theme.space30; y: Theme.space30
                    width: parent.width - Theme.space30 * 2
                    columns: 3
                    columnSpacing: Theme.space20
                    rowSpacing: Theme.space15

                    // ---- categorical pickers ----------------------------
                    Repeater {
                        model: [
                            { label: "TASK TYPE", options: taskTimeEstimator.taskTypes(), get: function() { return root.taskType; }, set: function(v) { root.taskType = v; } },
                            { label: "WEATHER", options: taskTimeEstimator.weatherConditions(), get: function() { return root.weather; }, set: function(v) { root.weather = v; } },
                            { label: "OPERATOR SKILL", options: taskTimeEstimator.skillLevels(), get: function() { return root.skill; }, set: function(v) { root.skill = v; } },
                            { label: "TERRAIN", options: taskTimeEstimator.terrainTypes(), get: function() { return root.terrain; }, set: function(v) { root.terrain = v; } },
                            { label: "MATERIAL", options: taskTimeEstimator.materialTypes(), get: function() { return root.material; }, set: function(v) { root.material = v; } },
                            { label: "TIME OF DAY", options: taskTimeEstimator.timesOfDay(), get: function() { return root.timeOfDay; }, set: function(v) { root.timeOfDay = v; } }
                        ]
                        delegate: ColumnLayout {
                            spacing: 4
                            Text { text: modelData.label; color: Theme.inkTertiary; font.family: Theme.fontSans; font.pixelSize: 11; font.letterSpacing: 1 }
                            ComboBox {
                                Layout.preferredWidth: 220
                                model: modelData.options
                                currentIndex: modelData.options.indexOf(modelData.get())
                                onActivated: modelData.set(modelData.options[currentIndex])
                                background: Rectangle { color: Theme.controlBg; radius: Theme.radiusSm; border.color: Theme.borderControl }
                                contentItem: Text {
                                    text: parent.displayText
                                    color: Theme.ink
                                    font.family: Theme.fontSans
                                    font.pixelSize: Theme.bodySize
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: Theme.space10
                                }
                            }
                        }
                    }

                    // ---- numeric sliders ---------------------------------
                    Repeater {
                        model: [
                            { label: "MACHINE AGE (YRS)", value: root.machineAge, min: 1, max: 8, set: function(v) { root.machineAge = v; } },
                            { label: "HOURS INTO SHIFT", value: root.fatigueHours, min: 0, max: 10, set: function(v) { root.fatigueHours = v; } },
                            { label: "PRIOR DELAY (MIN)", value: root.priorDelay, min: 0, max: 30, set: function(v) { root.priorDelay = v; } }
                        ]
                        delegate: ColumnLayout {
                            spacing: 4
                            Text {
                                text: modelData.label + ": " + modelData.value.toFixed(1)
                                color: Theme.inkTertiary
                                font.family: Theme.fontSans
                                font.pixelSize: 11
                                font.letterSpacing: 1
                            }
                            Slider {
                                Layout.preferredWidth: 220
                                from: modelData.min; to: modelData.max; value: modelData.value
                                onMoved: modelData.set(value)
                            }
                        }
                    }
                }
            }

            SectionHeader { text: "RESULT" }

            Row {
                spacing: Theme.space20

                Rectangle {
                    width: 260; height: 160
                    radius: Theme.radiusXl
                    color: Theme.surfaceCard
                    border.color: Theme.accent
                    border.width: 3
                    Column {
                        anchors.centerIn: parent
                        spacing: Theme.space5
                        Text { text: "PREDICTED"; color: Theme.inkTertiary; font.family: Theme.fontSans; font.pixelSize: 12; font.letterSpacing: 2; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: root.predicted.toFixed(0); color: Theme.ink; font.family: Theme.fontDisplay; font.pixelSize: Theme.displayXl; font.weight: Font.Bold; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "minutes"; color: Theme.ink; font.family: Theme.fontDisplay; font.pixelSize: Theme.displayUnitSm; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
                Rectangle {
                    width: 260; height: 160
                    radius: Theme.radiusXl
                    color: Theme.surfaceCard
                    border.color: Theme.borderPanel
                    border.width: 3
                    Column {
                        anchors.centerIn: parent
                        spacing: Theme.space5
                        Text { text: "HISTORICAL AVG"; color: Theme.inkTertiary; font.family: Theme.fontSans; font.pixelSize: 12; font.letterSpacing: 2; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: root.historical.toFixed(0); color: Theme.inkSecondary; font.family: Theme.fontDisplay; font.pixelSize: Theme.displayXl; font.weight: Font.Bold; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "minutes"; color: Theme.inkSecondary; font.family: Theme.fontDisplay; font.pixelSize: Theme.displayUnitSm; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
            }
        }
    }
}
