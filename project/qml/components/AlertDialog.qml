import QtQuick
import QtQuick.Controls.Basic

// Scrim + card + Dismiss popup, adapted from the reference design's modal.
// Built on Qt Quick Controls' Popup, which renders into the window's
// Overlay.overlay layer - guaranteed to paint above all page content
// regardless of sibling declaration order (a plain Item + manual z-index
// does NOT reliably win against a StackView's batched rendering).
// That source's Dismiss button used a separate "action-blue" fill reserved
// for that one screen; its own README says to prefer the accent color for
// any *new* primary action, which is what this generalized dialog does.
Popup {
    id: root
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    width: 380
    padding: Theme.space30
    x: Overlay.overlay ? (Overlay.overlay.width - width) / 2 : 0
    y: Overlay.overlay ? (Overlay.overlay.height - height) / 2 : 0

    property string dialogTitle: ""
    property string body: ""
    signal dismissed()

    function show(t, b) { dialogTitle = t; body = b; open(); }

    Overlay.modal: Rectangle { color: Theme.scrim }

    background: Rectangle {
        color: Theme.surfaceBase
        radius: Theme.radiusLg
    }

    onClosed: root.dismissed()

    contentItem: Column {
        spacing: Theme.space15
        width: root.availableWidth

        Text {
            text: root.dialogTitle
            color: Theme.ink
            font.family: Theme.fontSans
            font.pixelSize: Theme.popupTitle
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
            width: parent.width
        }
        Text {
            text: root.body
            color: Theme.inkSecondary
            font.family: Theme.fontSans
            font.pixelSize: Theme.popupBody
            wrapMode: Text.WordWrap
            width: parent.width
        }
        Row {
            anchors.right: parent.right
            Rectangle {
                width: dismissLabel.width + Theme.space20 * 2
                height: 40
                radius: 6
                color: Theme.accent
                Text {
                    id: dismissLabel
                    anchors.centerIn: parent
                    text: "DISMISS"
                    color: Theme.surfaceBase
                    font.family: Theme.fontSans
                    font.pixelSize: Theme.actionLabelSize
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.close()
                }
            }
        }
    }
}
