import QtQuick
import QtQuick.Controls.Basic

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("Operator Assistant")
    color: Theme.surfaceBase

    background: Rectangle { color: Theme.surfaceBase }

    TopBar {
        id: topBar
        width: parent.width
        anchors.top: parent.top
        onPageSelected: function(page) { stack.replace(pageFor(page)); }
        onInfoClicked: infoDialog.show(
            "Smart Operator Assistant",
            "Daily task dashboard, real-time safety monitoring, training hub, " +
            "unusual-behavior detection and task-time estimation for CAT machine " +
            "operators - running on a bundled synthetic dataset for this demo.")
    }

    AlertDialog { id: infoDialog }

    function pageFor(name) {
        if (name === "Safety") return "qml/pages/SafetyPage.qml";
        if (name === "Training") return "qml/pages/TrainingPage.qml";
        if (name === "Analytics") return "qml/pages/AnalyticsPage.qml";
        if (name === "Estimation") return "qml/pages/EstimationPage.qml";
        return "qml/pages/DashboardPage.qml";
    }

    StackView {
        id: stack
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        initialItem: "qml/pages/DashboardPage.qml"
    }
}
