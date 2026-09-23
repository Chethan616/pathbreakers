import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Real-time safety features: seatbelt compliance, proximity hazards and
// incident logging, with thresholds that shift with weather ("working
// conditions to be considered" in the brief) via safetyMonitor.
Item {
    id: root
    property var telemetryRows: dataManager.latestTelemetryList
    property var incidentRows: dataManager.incidentList

    function incidentLines() {
        var lines = [];
        for (var i = 0; i < root.incidentRows.length; i++) {
            var e = root.incidentRows[i];
            lines.push("[" + e.timestamp + "] " + e.machineId + "/" + e.operatorId +
                       " - " + e.alertType + " (" + e.severity + ")" + (e.manual ? " [manual]" : ""));
        }
        return lines;
    }

    AlertDialog { id: alertDialog }

    Connections {
        target: safetyMonitor
        function onAlertRaised(entry) {
            alertDialog.show("Safety alert: " + entry.alertType,
                              entry.machineId + " / " + entry.operatorId + " - " + entry.note);
        }
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

            RowLayout {
                width: parent.width
                SectionHeader { text: "LIVE MACHINE STATUS"; Layout.fillWidth: true }
                PillButton {
                    label: telemetrySimulator.running ? "LIVE FEED ON" : "LIVE FEED OFF"
                    icon: telemetrySimulator.running ? "record" : "play"
                    accentColor: telemetrySimulator.running ? Theme.statusNormal : Theme.controlBg
                    labelColor: telemetrySimulator.running ? Theme.surfaceBase : Theme.ink
                    onClicked: telemetrySimulator.running = !telemetrySimulator.running
                }
            }

            Flow {
                width: parent.width
                spacing: Theme.space15

                Repeater {
                    model: root.telemetryRows
                    delegate: MetricCard {
                        label: modelData.machineId + " · PROXIMITY"
                        value: modelData.proximityM
                        unit: "m"
                        status: safetyMonitor.proximitySeverity(modelData.proximityM, modelData.weather)
                        trendLabel: modelData.alertTriggered ? "Rising" : "Stable"
                        subLabel1: "IDLING"
                        subValue1: modelData.idlingMin.toFixed(0) + "m"
                        subLabel2: "SEATBELT"
                        subValue2: modelData.seatbeltStatus === "Fastened" ? "OK" : "OFF"
                    }
                }
            }

            SectionHeader { text: "LOG A MANUAL INCIDENT" }

            Rectangle {
                width: parent.width
                height: 90
                radius: Theme.radiusLg
                color: Theme.surfaceCard
                border.color: Theme.borderPanel
                border.width: 2

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.space15
                    spacing: Theme.space10

                    TextField {
                        id: machineField
                        placeholderText: "Machine ID (e.g. EXC001)"
                        Layout.preferredWidth: 180
                        color: Theme.ink
                        background: Rectangle { color: Theme.controlBg; radius: Theme.radiusSm; border.color: Theme.borderControl }
                    }
                    TextField {
                        id: operatorField
                        placeholderText: "Operator ID (e.g. OP001)"
                        Layout.preferredWidth: 180
                        color: Theme.ink
                        background: Rectangle { color: Theme.controlBg; radius: Theme.radiusSm; border.color: Theme.borderControl }
                    }
                    TextField {
                        id: noteField
                        placeholderText: "What happened?"
                        Layout.fillWidth: true
                        color: Theme.ink
                        background: Rectangle { color: Theme.controlBg; radius: Theme.radiusSm; border.color: Theme.borderControl }
                    }
                    PillButton {
                        label: "LOG INCIDENT"
                        icon: "warning"
                        onClicked: {
                            if (machineField.text.length === 0 || operatorField.text.length === 0) return;
                            safetyMonitor.logManualIncident(machineField.text, operatorField.text, "Manual", noteField.text);
                            machineField.text = ""; operatorField.text = ""; noteField.text = "";
                        }
                    }
                }
            }

            SectionHeader { text: "INCIDENT LOG" }
            LogConsole {
                width: parent.width
                height: 260
                lines: root.incidentLines()
            }
        }
    }
}
