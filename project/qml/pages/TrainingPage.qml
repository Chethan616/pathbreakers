import QtQuick
import QtQuick.Layouts

// Operator training hub: e-learning videos, instructor booking slots and
// simulation modules, all in one browsable list.
Item {
    id: root
    property var modules: dataManager.trainingModuleList()

    function refresh() { modules = dataManager.trainingModuleList(); }

    function iconFor(type) {
        if (type === "Video") return "play";
        if (type === "Instructor") return "info";
        return "record";
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.space30
        contentHeight: content.height
        clip: true

        Column {
            id: content
            width: parent.width
            spacing: Theme.space20

            SectionHeader { text: "TRAINING HUB" }

            Flow {
                width: parent.width
                spacing: Theme.space15

                Repeater {
                    model: root.modules
                    delegate: Rectangle {
                        width: 320
                        height: 190
                        radius: Theme.radiusLg
                        color: Theme.surfaceCard
                        border.color: Theme.borderPanel
                        border.width: 2

                        Column {
                            anchors.fill: parent
                            anchors.margins: Theme.space15
                            spacing: Theme.space10

                            RowLayout {
                                width: parent.width
                                IconGlyph { name: root.iconFor(modelData.type); color: Theme.accent; width: 20; height: 20 }
                                Text {
                                    text: modelData.type.toUpperCase()
                                    color: Theme.inkTertiary
                                    font.family: Theme.fontSans
                                    font.pixelSize: 11
                                    font.letterSpacing: 1.5
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: modelData.meta
                                    color: Theme.inkTertiary
                                    font.family: Theme.fontSans
                                    font.pixelSize: 11
                                }
                            }

                            Text {
                                text: modelData.title
                                color: Theme.ink
                                font.family: Theme.fontSans
                                font.pixelSize: Theme.bodyStrong
                                font.weight: Font.DemiBold
                                width: parent.width
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                text: modelData.description
                                color: Theme.inkSecondary
                                font.family: Theme.fontSans
                                font.pixelSize: Theme.captionSize
                                width: parent.width
                                wrapMode: Text.WordWrap
                                maximumLineCount: 3
                                elide: Text.ElideRight
                            }

                            Item { width: 1; height: Theme.space5 }

                            // Video / Simulation -> progress bar. Instructor -> Book button.
                            Rectangle {
                                visible: modelData.type !== "Instructor"
                                width: parent.width
                                height: 8
                                radius: 4
                                color: Theme.controlBg
                                Rectangle {
                                    width: parent.width * (modelData.progressPercent / 100.0)
                                    height: parent.height
                                    radius: 4
                                    color: modelData.progressPercent >= 100 ? Theme.statusNormal : Theme.accent
                                }
                            }
                            Text {
                                visible: modelData.type !== "Instructor"
                                text: modelData.progressPercent + "% complete"
                                color: Theme.inkTertiary
                                font.family: Theme.fontSans
                                font.pixelSize: 11
                            }

                            PillButton {
                                visible: modelData.type === "Instructor"
                                label: modelData.booked ? "BOOKED" : "BOOK SLOT"
                                icon: modelData.booked ? "check" : "info"
                                accentColor: modelData.booked ? Theme.controlBg : Theme.accent
                                labelColor: modelData.booked ? Theme.inkSecondary : Theme.surfaceBase
                                enabled: !modelData.booked
                                onClicked: { dataManager.bookSlot(modelData.id); root.refresh(); }
                            }
                        }
                    }
                }
            }
        }
    }
}
