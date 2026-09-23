import QtQuick

// Live metric tile: big numeral + unit, trend sparkline, status-colored glow,
// trend badge, and two secondary stat sub-tiles. Adapted from the reference
// design's card pattern (there: a heart-rate/breath-rate panel) - the outer
// border and section banner stay fixed, only the waveform/numeral glow/trend
// arrow shift with status.
Rectangle {
    id: root
    property string label: "METRIC"
    property real value: 0
    property string unit: ""
    property string status: "normal" // normal / warning / critical
    property string trendLabel: "Stable"
    property var history: []
    property string subLabel1: ""
    property string subValue1: ""
    property string subLabel2: ""
    property string subValue2: ""

    readonly property color statusColor: Theme.statusColor(status)

    width: 260
    height: 300
    radius: Theme.radiusXl
    color: Theme.surfaceCard
    border.color: Theme.borderPanel
    border.width: 3

    Column {
        anchors.fill: parent
        anchors.margins: Theme.space20
        spacing: Theme.space10

        Text {
            text: root.label
            color: Theme.inkTertiary
            font.family: Theme.fontSans
            font.pixelSize: Theme.headingSection * 0.6
            font.weight: Font.DemiBold
            font.letterSpacing: Theme.headingSectionTracking
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Sparkline {
            width: parent.width
            points: root.history
            lineColor: root.statusColor
            visible: root.history.length > 1
        }

        Item {
            width: parent.width
            height: Theme.displayXl + 10
            Text {
                anchors.centerIn: parent
                text: root.value.toFixed(1)
                color: Theme.ink
                font.family: Theme.fontDisplay
                font.pixelSize: Theme.displayXl
                font.weight: Font.Bold
                style: Text.Outline
                styleColor: Qt.rgba(root.statusColor.r, root.statusColor.g, root.statusColor.b, 0.35)
            }
        }
        Text {
            text: root.unit
            color: Theme.ink
            font.family: Theme.fontDisplay
            font.pixelSize: Theme.displayUnitSm
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.space5
            Rectangle {
                width: trendRow.width + Theme.space10 * 2
                height: 26
                radius: Theme.radiusPill
                color: Theme.surfaceCard
                border.color: Theme.borderPanel
                border.width: 2
                Row {
                    id: trendRow
                    anchors.centerIn: parent
                    spacing: Theme.space5
                    IconGlyph {
                        name: "chevron"
                        color: root.statusColor
                        width: 12
                        height: 12
                        rotation: root.trendLabel === "Rising" ? 180 : root.trendLabel === "Falling" ? 0 : 90
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: root.trendLabel
                        color: Theme.ink
                        font.family: Theme.fontSans
                        font.pixelSize: Theme.captionSize
                        font.weight: Font.DemiBold
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: Theme.space5
            visible: root.subLabel1 !== ""
            Rectangle {
                width: (parent.width - Theme.space5) / 2
                height: 50
                radius: Theme.radiusSm
                color: Theme.controlBg
                Column {
                    anchors.centerIn: parent
                    Text { text: root.subLabel1; color: Theme.inkTertiary; font.pixelSize: 10; font.family: Theme.fontSans; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: root.subValue1; color: Theme.ink; font.pixelSize: 18; font.family: Theme.fontDisplay; anchors.horizontalCenter: parent.horizontalCenter }
                }
            }
            Rectangle {
                width: (parent.width - Theme.space5) / 2
                height: 50
                radius: Theme.radiusSm
                color: Theme.controlBg
                Column {
                    anchors.centerIn: parent
                    Text { text: root.subLabel2; color: Theme.inkTertiary; font.pixelSize: 10; font.family: Theme.fontSans; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: root.subValue2; color: Theme.ink; font.pixelSize: 18; font.family: Theme.fontDisplay; anchors.horizontalCenter: parent.horizontalCenter }
                }
            }
        }
    }
}
