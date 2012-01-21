import QtQuick 1.0

Rectangle {
    id: blah
    width: 1000
    height: 660
    color: "#00000000"

    signal go_back()

    Image {
        id: image1
        x: 0
        y: 0
        source: "list.png"


        Image {
            MouseArea {
                anchors.fill: parent

                onPressed :{
                    blah.go_back();
                }
            }
            id: image2
            x: 834
            y: 531
            source: "back.png"
        }
    }
}
