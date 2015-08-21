import QtQuick 2.2

MouseArea {
    id: mouseArea
    acceptedButtons: Qt.NoButton
    anchors.fill: parent
    hoverEnabled: true

    property alias tip: tip
    property alias text: tip.text
    property alias hideDelay: hideTimer.interval
    property alias showDelay: showTimer.interval

    Timer {
        id: showTimer
        interval: 1000
        running: text.length !== 0 && mouseArea.containsMouse && !tip.visible
        onTriggered: {
            tip.show(mouseX, mouseY);
        }
    }

    Timer {
        id: hideTimer
        interval: 100
        running: !mouseArea.containsMouse && tip.visible
        onTriggered: tip.hide();
    }

    ToolTip {
        id: tip
    }
}
