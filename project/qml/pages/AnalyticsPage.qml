import QtQuick
import QtQuick.Layouts

// Unusual-behavior detection (excessive idling / unsafe operation patterns)
// plus a per-machine idling/fuel trend, from AnomalyDetector's rule + z-score
// pass over the bundled telemetry dataset.
Item {
    id: root
    property var anomalies: anomalyDetector.anomalyList
    property var machines: dataManager.machineList()
    property string selectedMachine: machines.length > 0 ? machines[0].id : ""
    property var history: selectedMachine ? dataManager.telemetryHistoryFor(selectedMachine, 40) : []

    function idlingSeries() {
        var out = [];
        for (var i = 0; i < history.length; i++) out.push(history[i].idlingMin);
        return out;
    }
    function fuelSeries() {
        var out = [];
        for (var i = 0; i < history.length; i++) out.push(history[i].fuelUsedL);
        return out;
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

            SectionHeader { text: "UNUSUAL BEHAVIOR" }

            Text {
                visible: root.anomalies.length === 0
                text: "No anomalies flagged against the current fleet baseline."
                color: Theme.inkTertiary
                font.family: Theme.fontSans
                font.pixelSize: Theme.bodySize
            }

            Column {
                width: parent.width
                spacing: Theme.space10
                Repeater {
                    model: root.anomalies
                    delegate: Rectangle {
                        width: content.width
                        height: 72
                        radius: Theme.radiusMd
                        color: Theme.surfaceCard
                        border.color: Theme.statusWarning
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.space15
                            spacing: Theme.space15

                            IconGlyph { name: "warning"; color: Theme.statusWarning; width: 24; height: 24 }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    text: modelData.kind + " — " + modelData.machineId + " / " + modelData.operatorId
                                    color: Theme.ink
                                    font.family: Theme.fontSans
                                    font.pixelSize: Theme.bodyStrong
                                    font.weight: Font.DemiBold
                                }
                                Text {
                                    text: modelData.detail
                                    color: Theme.inkSecondary
                                    font.family: Theme.fontSans
                                    font.pixelSize: Theme.captionSize
                                }
                            }
                        }
                    }
                }
            }

            RowLayout {
                width: parent.width
                SectionHeader { text: "MACHINE TREND"; Layout.fillWidth: true }
                Repeater {
                    model: root.machines
                    delegate: Rectangle {
                        width: 70; height: 30
                        radius: Theme.radiusSm
                        color: modelData.id === root.selectedMachine ? Theme.accent : Theme.controlBg
                        Text {
                            anchors.centerIn: parent
                            text: modelData.id
                            font.family: Theme.fontMono
                            font.pixelSize: 10
                            color: modelData.id === root.selectedMachine ? Theme.surfaceBase : Theme.inkSecondary
                        }
                        MouseArea { anchors.fill: parent; onClicked: root.selectedMachine = modelData.id }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 220
                radius: Theme.radiusLg
                color: Theme.surfaceCard
                border.color: Theme.borderPanel
                border.width: 2

                Column {
                    anchors.fill: parent
                    anchors.margins: Theme.space20
                    spacing: Theme.space10

                    Text { text: "IDLING (MIN) - LAST " + root.history.length + " READINGS"; color: Theme.inkTertiary; font.family: Theme.fontSans; font.pixelSize: 11; font.letterSpacing: 1 }
                    Sparkline { width: parent.width; height: 60; points: root.idlingSeries(); lineColor: Theme.statusWarning }

                    Text { text: "FUEL USED (L)"; color: Theme.inkTertiary; font.family: Theme.fontSans; font.pixelSize: 11; font.letterSpacing: 1 }
                    Sparkline { width: parent.width; height: 60; points: root.fuelSeries(); lineColor: Theme.accent }
                }
            }
        }
    }
}
