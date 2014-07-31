import QtQuick 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"

    Text {
        id: title
        anchors.fill: parent

        visible: false

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        font.pointSize: 36
        font.family: "Arial"

        color: "white"

        text: "powered by qsanguosha.org"
    }

    FastBlur {
        id: blur
        anchors.fill: parent
        source: title
        radius: 100

        transform: [
            Scale { id: scale; origin.x: parent.width / 2; origin.y: parent.height / 2; xScale: 0.2; yScale: 0.2 }
        ]
    }

    ParallelAnimation {
        id: introTransation

        PauseAnimation { duration: 1200 }

        NumberAnimation { target: blur; property: "radius"; to: 0; duration: 800; easing.type: Easing.InOutQuad }
        NumberAnimation { target: scale; property: "xScale"; to: 1; duration: 1000; easing.type: Easing.OutQuad }
        NumberAnimation { target: scale; property: "yScale"; to: 1; duration: 1000; easing.type: Easing.OutQuad }
        NumberAnimation { target: blur; property: "opacity"; from: 0; to: 1; duration: 500 }

        SequentialAnimation {
            PauseAnimation { duration: 1200 }
            NumberAnimation { target: blur; property: "opacity"; to: 0; duration: 500 }
            ScriptAction {
                script: {
                    root.destroy();
                }
            }
        }
    }

    Component.onCompleted: {
        introTransation.start();
    }
}
