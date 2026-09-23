import QtQuick

// Monospace, green-on-dark scrolling log panel - the reference design's raw
// telemetry console, reused here for the live incident/safety-alert feed.
Rectangle {
    id: root
    property var lines: []
    color: "transparent"
    border.color: Theme.borderPanel
    border.width: 1
    radius: Theme.radiusSm

    ListView {
        id: list
        anchors.fill: parent
        anchors.margins: Theme.space10
        clip: true
        model: root.lines
        verticalLayoutDirection: ListView.TopToBottom
        delegate: Text {
            width: list.width
            text: modelData
            color: Theme.consoleText
            font.family: Theme.fontMono
            font.pixelSize: Theme.consoleSize
            wrapMode: Text.WrapAnywhere
        }
        onCountChanged: positionViewAtBeginning()
    }
}
