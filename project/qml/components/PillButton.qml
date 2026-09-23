import QtQuick

// Large accent pill button with icon + uppercase tracked label - the
// reference design's footer action buttons (there: MONITOR / RECORD).
// Theme is a module-local singleton (qt_add_qml_module registers it from
// Theme.qml's "pragma Singleton"), so it's usable here with no import.
Rectangle {
    id: root
    signal clicked()
    property string label: "ACTION"
    property string icon: "play"
    property color accentColor: Theme.accent
    property color labelColor: Theme.surfaceBase
    property bool enabled: true

    implicitWidth: row.width + Theme.space30
    implicitHeight: 52
    radius: Theme.radiusMd
    color: enabled ? (mouse.pressed ? Qt.darker(accentColor, 1.15) : accentColor) : Theme.controlBg
    opacity: enabled ? 1.0 : 0.5

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Theme.space5

        IconGlyph {
            name: root.icon
            color: root.labelColor
            width: 20
            height: 20
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: root.label
            color: root.labelColor
            font.family: Theme.fontSans
            font.pixelSize: Theme.bodyStrong
            font.weight: Font.DemiBold
            font.letterSpacing: 1
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
    }
}
