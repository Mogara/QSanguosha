import QtQuick 2.4

Image {
    id: startScene
    source: "file:///" + Config.BackgroundImage
    anchors.fill: parent

    Image {
        id: logo
        source: "../image/logo/logo.png"
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: -parent.width / 4
        opacity: 0

        Behavior on opacity {
            NumberAnimation { duration: 1000 }
        }
    }

    Timer {
        interval: 1000
        running: true
        onTriggered: logo.opacity = 1
    }
}

