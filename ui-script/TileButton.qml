import QtQuick 2.2
//import QtGraphicalEffects 1.0

Item {
    id: button
    width: 154
    height: 154

//    RectangularGlow {
//        anchors.fill: rect
//        glowRadius: 1
//        spread: 1.0
//        visible: mouse.containsMouse || parent.focus
//        antialiasing: true
//    }

    Rectangle {
        id: rect
        anchors.fill: parent
        color: "#78D478"
        antialiasing: true
        border.width: 1
        border.color: "#8CDA8C"
    }

    signal clicked

    property alias text: labelText.text

    property string iconSource: ""
    property bool autoHideText: true

    antialiasing: true

    transform: [
        Rotation {
            id: rotationTransform

            angle: 0

            axis.x: 0
            axis.y: 0
            axis.z: 0

            origin.x: button.width / 2.0
            origin.y: button.height / 2.0

            Behavior on angle {
                NumberAnimation { duration: 100 }
            }
        },

        Scale {
            id: scaleTransform

            xScale: 1
            yScale: 1

            origin.x: button.width / 2.0
            origin.y: button.height / 2.0

            Behavior on xScale {
                NumberAnimation { duration: 100 }
            }

            Behavior on yScale {
                NumberAnimation { duration: 100 }
            }
        }

    ]

    onIconSourceChanged: {
        var src = iconSource;
        var fileName = /[\w\d_]+/;
        if (fileName.test(src)) {
            src = "../image/system/button/icon/" + src + ".png"
        }
        icon.source = src;
    }

    Image {
        id: icon
        anchors.centerIn: parent
    }

    Text {
        id: labelText

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 3
        anchors.left: parent.left
        anchors.leftMargin: 3

        visible: !autoHideText || mouse.containsMouse

        color: "white"
        font.pointSize: 8
        font.family: "WenQuanYi Micro Hei"

        text: "Button"
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true

        property bool down: false

        onPressed: {
            down = true;

            rotationTransform.axis.x = 0;
            rotationTransform.axis.y = 0;
            rotationTransform.origin.x = button.width / 2.0
            rotationTransform.origin.y = button.height / 2.0

            if (mouseX > parent.width - 30)
            {
                rotationTransform.origin.x = 0;
                rotationTransform.axis.y = 1;
                rotationTransform.angle = 15;
                return;
            }

            if (mouseX < 30) {
                rotationTransform.origin.x = button.width;
                rotationTransform.axis.y = 1;
                rotationTransform.angle = -15;
                return;
            }

            if (mouseY < 30) {
                rotationTransform.origin.y = button.height;
                rotationTransform.axis.x = 1;
                rotationTransform.angle = 15;
                return;
            }

            if (mouseY > parent.height - 30) {
                rotationTransform.origin.y = 0;
                rotationTransform.axis.x = 1;
                rotationTransform.angle = -15;
                return;
            }

            scaleTransform.xScale = 0.95;
            scaleTransform.yScale = 0.95;
        }

        onCanceled: {
            reset();
            down = false;
        }

        onReleased: {
            reset();
            if (down) {
                button.clicked();
            }
        }

        onExited: {
            reset();
            down = false;
        }

        function reset() {
            scaleTransform.xScale = 1;
            scaleTransform.yScale = 1;
            rotationTransform.angle = 0;
        }
    }

    Keys.onReturnPressed: {
        button.clicked();
    }

    NumberAnimation on scale {
        id: scaleAni
        from: 0.5
        to: 1
        running: false
    }

    Component.onCompleted: scaleAni.start()
}
