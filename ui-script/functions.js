.pragma library
.import QtQuick 2.2 as QQ

function createObject(parent, fileName) {
    var component = Qt.createComponent(fileName);
    if (component.status === QQ.Component.Ready) {
        return component.createObject(parent);
    }
    return null;
}
