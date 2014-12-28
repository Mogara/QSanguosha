import QtQuick 2.4
import "functions.js" as QSan

Item {
    id: root
    width: winSize.width
    height: winSize.height

    property Item splash: createObject("Splash")

    Connections {
        target: splash
        onDisappeared: splash.destroy()
        onDisappearing: createObject("StartScene")
    }

    function createObject(componentName) {
        return QSan.createObject(root, componentName + ".qml");
    }
}
