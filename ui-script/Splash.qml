import QtQuick 2.3
import QtQuick.Particles 2.0

Rectangle {
    id: splash
    anchors.fill: parent
    color: "#bc3b3b"

    property bool loading: true

    //---------------Logo-------------------
    Image {
        id: logo
        anchors.centerIn: parent
        source: "../image/logo/logo.png"
        Behavior on anchors.horizontalCenterOffset {
            NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad }
        }
    }

    //---------------Bubbles-----------------
    ParticleSystem {
        id: particles
        anchors.fill: parent

        ImageParticle {
            id: bubble
            anchors.fill: parent
            source: "../image/system/splash/bubble.png"
            opacity: 0.25
        }

        Wander {
            xVariance: 25;
            pace: 25;
        }

        Emitter {
            width: parent.width
            height: 150
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 3
            startTime: 15000

            emitRate: 2
            lifeSpan: 15000

            acceleration: PointDirection{ y: -6; xVariation: 2; yVariation: 2 }

            size: 24
            sizeVariation: 16
        }
    }

    Age {
        system: particles
        anchors.bottom: waves1.bottom
        anchors.top: parent.top
        width: parent.width
        lifeLeft: 50
        advancePosition: false
    }

    //--------------Text-------------
    Text {
        height: 50; width: 150
        id: text
        text: qsTr("Loading...")
        color: "#ffffff"
        font.family: "微软雅黑"
        font.pointSize: 30
        horizontalAlignment: Text.AlignHCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 150
        anchors.horizontalCenter: parent.horizontalCenter
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 0; duration: 1600; easing.type: Easing.InOutQuad; }
            NumberAnimation { from: 0; to: 1; duration: 1600; easing.type: Easing.InOutQuad; }
        }
    }

    //---------------Water---------------
    Rectangle {
        id: water
        height: 0
        width: parent.width
        anchors.bottom: parent.bottom
        Behavior on height {
            SequentialAnimation {
                NumberAnimation { duration: 10000; }
                PauseAnimation { duration: 1000; }
                PropertyAction { target: text; property: "text"; value: qsTr("Press Any Key...") }

                PropertyAction { target: splash; property: "loading"; value: false }
            }
        }
        color: "#73c0d6"
        opacity: 0.5

        Component.onCompleted: height = parent.height * 0.8
    }

    //---------------Waves---------------
    Row {
        id: waves1
        height: childrenRect.height
        anchors.bottom: water.top
        anchors.bottomMargin: -20
        Image {
            id: wave
            width: splash.width
            source:"../image/system/splash/wave.png"
        }
        Image {
            width: splash.width
            source:"../image/system/splash/wave.png"
        }
        NumberAnimation on x { from: 0; to: -(wave.width); duration: 16000; loops: Animation.Infinite }
        SequentialAnimation on y {
            loops: Animation.Infinite
            NumberAnimation { from: y - 2; to: y + 2; duration: 1600; easing.type: Easing.InOutQuad }
            NumberAnimation { from: y + 2; to: y - 2; duration: 1600; easing.type: Easing.InOutQuad }
        }
    }

    Row {
        id: waves2
        anchors.bottom: water.top
        anchors.bottomMargin: -15
        opacity: 0.5
        Image {
            id: wave2
            width: splash.width
            source:"../image/system/splash/wave.png"
        }
        Image {
            width: splash.width
            source:"../image/system/splash/wave.png"
        }
        NumberAnimation on x { from: -(wave2.width); to: 0; duration: 32000; loops: Animation.Infinite }
        SequentialAnimation on y {
            loops: Animation.Infinite
            NumberAnimation { from: y + 2; to: y - 2; duration: 1600; easing.type: Easing.InOutQuad }
            NumberAnimation { from: y - 2; to: y + 2; duration: 1600; easing.type: Easing.InOutQuad }
        }
    }

    //---------------Sunlight-----------------
    Image {
        source: "../image/system/splash/sunlight.png"
        opacity: 0.02
        y: -20
        width: parent.width * 3
        anchors.horizontalCenter: parent.horizontalCenter
        transformOrigin: Item.Top
        SequentialAnimation on rotation {
            loops: Animation.Infinite
            NumberAnimation { from: -10; to: 10; duration: 8000; easing.type: Easing.InOutSine }
            NumberAnimation { from: 10; to: -10; duration: 8000; easing.type: Easing.InOutSine }
        }
    }

    Image {
        source: "../image/system/splash/sunlight.png"
        opacity: 0.04
        y: 0
        width: parent.width * 3
        anchors.horizontalCenter: parent.horizontalCenter
        transformOrigin: Item.Top
        SequentialAnimation on rotation {
            loops: Animation.Infinite
            NumberAnimation { from: 10; to: -10; duration: 8000; easing.type: Easing.InOutSine }
            NumberAnimation { from: -10; to: 10; duration: 8000; easing.type: Easing.InOutSine }
        }
    }

    //--------------------Disappear--------------
    Behavior on opacity {
        NumberAnimation { duration: 2000; easing.type: Easing.InOutQuad }
    }
    MouseArea {
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
        }
    }

    function disappear() {
        logo.anchors.horizontalCenterOffset = -root.width / 4;
        opacity = 0;
    }
}
