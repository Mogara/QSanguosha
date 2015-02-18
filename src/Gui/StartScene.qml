import QtQuick 2.4
import Cardirector.Gui 1.0
import "../Client"

Image {
    id: startScene
    source: config.backgroundImage
    anchors.fill: parent

    property bool isFeteDay: isInFeteDays()

    Image {
        id: logo
        source: "image://mogara/logo" + (isFeteDay ? "-moxuan" : "")
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: -parent.width / 4
        opacity: 0

        ToolTipArea {
            text: isFeteDay ? qsTr("At 10:40 a.m., August 19, 2014, Moxuanyanyun, a developer of QSanguosha, passed away peacefully in Dalian Medical College. He was 18 and had struggled with leukemia for more than 4 years. May there is no pain in Heaven.") : ""
        }
    }

    NumberAnimation {
        id: logoAni
        target: logo
        property: "opacity"
        duration: 1000
        to: 1
        onStopped: {
            var btns = [
                        {"name": qsTr("start_game"), "icon": "start_game"},
                        {"name": qsTr("start_server"), "icon": "start_server"},
                        {"name": qsTr("pc_console_start"), "icon": "pc_console_start"},
                        {"name": qsTr("replay"), "icon": "replay"},
                        {"name": qsTr("configure"), "icon": "configure"},
                        {"name": qsTr("general_overview"), "icon": "general_overview"},
                        {"name": qsTr("card_overview"), "icon": "card_overview"},
                        {"name": qsTr("about"), "icon": "about"}
                    ];
            for (var i = 0; i < btns.length; ++ i) {
                buttons.append(btns[i]);
            }
        }
    }

    GridView {
        id: grid
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: parent.width / 4
        flow: GridView.FlowTopToBottom
        cellHeight: 162; cellWidth: 162
        height: cellHeight * 4; width: cellWidth * 2
        delegate: buttonDelegate
        model: buttons
    }

    Component {
        id: buttonDelegate

        TileButton {
            text: name
            iconSource: "image://tileicon/" + icon
        }
    }

    ListModel {
        id: buttons
    }

    Timer {
        interval: 1000
        running: true
        onTriggered: logoAni.start()
    }

    function isInFeteDays() {
        var date = new Date();
        if (date.getMonth() == 7 && date.getDate() >= 19 && date.getDate() <= 26) {
            return true;
        }
        return false;
    }
}

