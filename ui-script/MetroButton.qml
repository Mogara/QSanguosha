import QtQuick 2.2

Item {
    id: button
    width: 189
    height: 46

    Rectangle {
        id: bg
        anchors.fill: parent
        color: "black"
        border.width: 2
        border.color: "white"
        opacity: 0.8
    }

    property bool enabled: true

    property alias text: title.text
    property alias iconSource: icon.source

    signal clicked

    states: [
        State {
            name: "hovered"; when: mouse.containsMouse
            PropertyChanges { target: bg; color: "white" }
            PropertyChanges { target: title; color: "black" }
        },
        State {
            name: "disabled"; when: !enabled
            PropertyChanges { target: button; opacity: 0.2 }
        }
    ]

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: parent.enabled
        onReleased: parent.clicked()
    }

    Row {
        anchors.centerIn: parent
        spacing: 5
        Image {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: title
            font.pixelSize: 18
            font.family: "WenQuanYi Micro Hei"
            anchors.verticalCenter: parent.verticalCenter
            text: ""
            color: "white"
        }
    }
}

