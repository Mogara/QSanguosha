import QtQuick 2.2

Image {
    id: startScene
    source: "file:///" + Config.BackgroundImage
    anchors.fill: parent

    property bool isFeteDay: isInFeteDays()

    Image {
        id: logo
        source: "../image/logo/logo" + (isFeteDay ? "-moxuan" : "") + ".png"
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: -parent.width / 4
        opacity: 0

        Behavior on opacity {
            NumberAnimation { duration: 1000 }
        }

        ToolTipArea {
            text: isFeteDay ? qsTr("At 10:40 a.m., August 19, 2014, Moxuanyanyun, a developer of QSanguosha, passed away peacefully in Dalian Medical College. He was 18 and had struggled with leukemia for more than 4 years. May there is no pain in Heaven.") : ""
        }
    }

    Timer {
        interval: 1000
        running: true
        onTriggered: logo.opacity = 1
    }

    function isInFeteDays() {
        var date = new Date();
        if (date.getMonth() == 7 && date.getDate() >= 19 && date.getDate() <= 26) {
            return true;
        }
        return false;
    }
}

