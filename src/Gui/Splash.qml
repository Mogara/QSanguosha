import QtQuick 2.4
import Cardirector.Gui 1.0
import Cardirector.Device 1.0

Rectangle {
    id: splash
    anchors.fill: parent
    color: "#bc3b3b"
    focus: true
    z: 100

    property bool loading: true

    signal disappearing
    signal disappeared

    //---------------Logo-------------------
    Image {
        id: logo
        anchors.centerIn: parent
        source: "image://mogara/logo"
    }

    //--------------Text-------------
    Text {
        id: qsan
        text: qsTr("QSanguosha")
        color: "#ffffff"
        font.family: "微软雅黑"
        font.pointSize: 30
        anchors.left: logo.right
        anchors.leftMargin: Device.gu(30)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -Device.gu(20)
        opacity: 0
    }

    Row {
        height: childrenRect.height
        width: childrenRect.width
        anchors.horizontalCenter: qsan.horizontalCenter
        anchors.top: qsan.bottom
        anchors.verticalCenterOffset: Device.gu(40)
        spacing: Device.gu(8)

        Text {
            id: free
            text: qsTr("Free")
            color: "#ffffff"
            font.family: "微软雅黑"
            font.pointSize: 15
            opacity: 0
        }

        Text {
            id: open
            text: qsTr("Open")
            color: "#ffffff"
            font.family: "微软雅黑"
            font.pointSize: 15
            opacity: 0
        }

        Text {
            id: flexible
            text: qsTr("Flexible")
            color: "#ffffff"
            font.family: "微软雅黑"
            font.pointSize: 15
            opacity: 0
        }
    }

    Text {
        id: text
        text: qsTr("Press Any Key...")
        color: "#ffffff"
        opacity: 0
        font.family: "微软雅黑"
        font.pointSize: 15
        horizontalAlignment: Text.AlignHCenter
        anchors.top: logo.bottom
        anchors.topMargin: Device.gu(65)
        anchors.horizontalCenter: parent.horizontalCenter
        SequentialAnimation on opacity {
            id: textAni
            running: false
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: 1; duration: 1600; easing.type: Easing.InOutQuad; }
            NumberAnimation { from: 1; to: 0; duration: 1600; easing.type: Easing.InOutQuad; }
        }
    }

    SequentialAnimation {
        running: true
        NumberAnimation {
            target: logo
            property: "anchors.horizontalCenterOffset"
            to: -splash.width / 8
            duration: 1000
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            target: qsan
            property: "opacity"
            duration: 500
            easing.type: Easing.InOutQuad
            to: 1
        }

        NumberAnimation {
            target: free
            property: "opacity"
            duration: 400
            easing.type: Easing.InOutQuad
            to: 1
        }

        NumberAnimation {
            target: open
            property: "opacity"
            duration: 400
            easing.type: Easing.InOutQuad
            to: 1
        }

        NumberAnimation {
            target: flexible
            property: "opacity"
            duration: 400
            easing.type: Easing.InOutQuad
            to: 1
        }


        ScriptAction { script: textAni.start(); }

        PropertyAction { target: splash; property: "loading"; value: false }
    }

    Text {
        text: qsTr("Powered by Mogara")
        color: "#f39292"
        font.family: "Segoe Script"
        font.pointSize: 8
        anchors.bottom: parent.bottom
        anchors.right: parent.right
    }

    //--------------------Disappear--------------
    Behavior on opacity {
        SequentialAnimation {
            NumberAnimation { duration: 2000; easing.type: Easing.InOutQuad }
            ScriptAction { script: disappeared() }
        }
    }
    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent
        onClicked: {
            if (!loading) {
                disappear();
            }
        }
    }

    Keys.onPressed: {
        if (!loading) {
            disappear();
            event.accepted = true
        }
    }

    NumberAnimation {
        id: logoMover
        target: logo
        property: "anchors.horizontalCenterOffset"
        to: -splash.width
        duration: 1000
        easing.type: Easing.InOutQuad
    }

    function disappear() {
        disappearing();
        logoMover.start();
        opacity = 0;
    }
}
