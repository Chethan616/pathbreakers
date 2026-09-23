import QtQuick
import QtQuick.Controls.Basic

// Header bar: wordmark, page switcher dropdown, icon actions - adapted from
// the reference design's NavBar (there: a fixed page list of "Vital Signs /
// 3D View / Logs"; here the five operator-assistant sections). The open list
// is a Popup so it renders into the window's Overlay.overlay layer - always
// above the page content below it, regardless of StackView's paint order.
Rectangle {
    id: root
    height: 80
    color: Theme.surfaceBase

    property var pages: ["Dashboard", "Safety", "Training", "Analytics", "Estimation"]
    property string currentPage: "Dashboard"
    property bool nightMode: false
    property bool alertsEnabled: true

    signal pageSelected(string page)
    signal infoClicked()

    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.borderControl }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.space20
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space30

        Text {
            text: "OPERATOR ASSISTANT"
            color: Theme.accent
            font.family: Theme.fontSans
            font.pixelSize: Theme.headingWordmark
            font.weight: Font.DemiBold
            anchors.verticalCenter: parent.verticalCenter
        }

        // ---- Page switcher --------------------------------------------
        Item {
            id: switcher
            width: 220
            height: 40
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                id: closedBox
                anchors.fill: parent
                color: Theme.controlBg
                border.color: Theme.borderControl
                border.width: 1
                radius: Theme.radiusMd

                Row {
                    anchors.fill: parent
                    anchors.margins: Theme.space10
                    Text {
                        width: parent.width - 16
                        text: root.currentPage
                        color: Theme.ink
                        font.family: Theme.fontSans
                        font.pixelSize: Theme.bodySize
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    IconGlyph {
                        name: "chevron"
                        color: Theme.ink
                        width: 14
                        height: 14
                        rotation: pageList.visible ? 180 : 0
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                MouseArea { anchors.fill: parent; onClicked: pageList.visible ? pageList.close() : pageList.open() }
            }

            Popup {
                id: pageList
                y: closedBox.height + 6
                width: switcher.width
                padding: 0
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                background: Rectangle {
                    color: Theme.controlBg
                    border.color: Theme.borderControl
                    border.width: 1
                }

                contentItem: Column {
                    Repeater {
                        model: root.pages
                        delegate: Rectangle {
                            width: pageList.width
                            height: 40
                            color: modelData === root.currentPage ? Theme.borderControl : "transparent"
                            Text {
                                anchors.fill: parent
                                anchors.leftMargin: Theme.space10
                                verticalAlignment: Text.AlignVCenter
                                text: modelData
                                color: Theme.ink
                                font.family: Theme.fontSans
                                font.pixelSize: Theme.bodySize
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.currentPage = modelData;
                                    root.pageSelected(modelData);
                                    pageList.close();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: Theme.space20
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space10

        Rectangle {
            width: 40; height: 40; radius: Theme.radiusMd
            color: "transparent"
            IconGlyph {
                anchors.centerIn: parent
                name: root.alertsEnabled ? "bell" : "bellOff"
                color: Theme.accent
                width: 22; height: 22
            }
            MouseArea { anchors.fill: parent; onClicked: root.alertsEnabled = !root.alertsEnabled }
        }
        Rectangle {
            width: 40; height: 40; radius: Theme.radiusMd
            color: "transparent"
            IconGlyph {
                anchors.centerIn: parent
                name: root.nightMode ? "moon" : "sun"
                color: Theme.accent
                width: 22; height: 22
            }
            MouseArea { anchors.fill: parent; onClicked: root.nightMode = !root.nightMode }
        }
        Rectangle {
            width: 40; height: 40; radius: Theme.radiusMd
            color: "transparent"
            IconGlyph { anchors.centerIn: parent; name: "info"; color: Theme.accent; width: 22; height: 22 }
            MouseArea { anchors.fill: parent; onClicked: root.infoClicked() }
        }
    }
}
