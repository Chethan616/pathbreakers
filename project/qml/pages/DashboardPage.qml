import QtQuick
import QtQuick.Layouts

// Daily task dashboard: fleet summary tiles + today's schedule.
Item {
    id: root
    property var summary: dataManager.dashboardSummary()
    property var schedule: dataManager.scheduleList()

    Component.onCompleted: refresh()
    function refresh() {
        summary = dataManager.dashboardSummary();
        schedule = dataManager.scheduleList();
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.space30
        contentHeight: content.height
        clip: true

        Column {
            id: content
            width: parent.width
            spacing: Theme.space30

            SectionHeader { text: "FLEET SUMMARY" }

            Row {
                spacing: Theme.space15
                Repeater {
                    model: [
                        { label: "MACHINES", value: root.summary.machineCount, unit: "" },
                        { label: "OPERATORS", value: root.summary.operatorCount, unit: "" },
                        { label: "ALERTS LOGGED", value: root.summary.totalAlerts, unit: "" },
                        { label: "AVG IDLING", value: root.summary.avgIdlingMin ? root.summary.avgIdlingMin.toFixed(1) : "0.0", unit: "min" }
                    ]
                    delegate: Rectangle {
                        width: 190; height: 100
                        radius: Theme.radiusLg
                        color: Theme.surfaceCard
                        border.color: Theme.borderPanel
                        border.width: 2
                        Column {
                            anchors.centerIn: parent
                            spacing: Theme.space5
                            Text {
                                text: modelData.label
                                color: Theme.inkTertiary
                                font.family: Theme.fontSans
                                font.pixelSize: 11
                                font.letterSpacing: 1.5
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Row {
                                spacing: 4
                                anchors.horizontalCenter: parent.horizontalCenter
                                Text {
                                    text: modelData.value
                                    color: Theme.accent
                                    font.family: Theme.fontDisplay
                                    font.pixelSize: 30
                                    font.weight: Font.Bold
                                }
                                Text {
                                    text: modelData.unit
                                    color: Theme.ink
                                    font.family: Theme.fontDisplay
                                    font.pixelSize: 16
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 4
                                }
                            }
                        }
                    }
                }
            }

            SectionHeader { text: "TODAY'S SCHEDULE" }

            Column {
                width: parent.width
                spacing: Theme.space10

                Repeater {
                    model: root.schedule
                    delegate: Rectangle {
                        width: content.width
                        height: 64
                        radius: Theme.radiusMd
                        color: Theme.surfaceCard
                        border.color: modelData.priority === "High" ? Theme.statusWarning : Theme.borderPanel
                        border.width: modelData.priority === "High" ? 2 : 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Theme.space20
                            anchors.rightMargin: Theme.space20
                            spacing: Theme.space20

                            Text {
                                text: modelData.time
                                color: Theme.inkTertiary
                                font.family: Theme.fontMono
                                font.pixelSize: Theme.captionSize
                                Layout.preferredWidth: 60
                            }
                            Text {
                                text: modelData.taskType
                                color: Theme.ink
                                font.family: Theme.fontSans
                                font.pixelSize: Theme.bodyStrong
                                font.weight: Font.DemiBold
                                Layout.fillWidth: true
                            }
                            Text {
                                text: modelData.machineId + " · " + modelData.operatorId
                                color: Theme.inkSecondary
                                font.family: Theme.fontSans
                                font.pixelSize: Theme.bodySize
                                Layout.preferredWidth: 150
                            }
                            Text {
                                text: modelData.estimatedMin.toFixed(0) + " min"
                                color: Theme.inkTertiary
                                font.family: Theme.fontSans
                                font.pixelSize: Theme.bodySize
                                Layout.preferredWidth: 70
                            }
                            Rectangle {
                                Layout.preferredWidth: statusLabel.width + Theme.space10 * 2
                                Layout.preferredHeight: 28
                                radius: Theme.radiusPill
                                color: modelData.status === "Completed" ? Theme.controlBg
                                     : modelData.status === "InProgress" ? Qt.rgba(0, 0.73, 0.98, 0.18)
                                     : "transparent"
                                border.color: modelData.status === "Scheduled" ? Theme.borderControl : "transparent"
                                border.width: 1
                                Text {
                                    id: statusLabel
                                    anchors.centerIn: parent
                                    text: modelData.status.toUpperCase()
                                    color: modelData.status === "InProgress" ? Theme.accent : Theme.inkSecondary
                                    font.family: Theme.fontSans
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    font.letterSpacing: 1
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
