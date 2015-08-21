import QtQuick 2.2

Rectangle {
    id: tooltip
    width: holder.width + horizontalPadding * 2
    height: holder.height + verticalPadding * 2
    color: "#aa999999"
    visible: false

    property alias text: holder.text
    property int verticalPadding: 1
    property int horizontalPadding: 5


    Text {
        anchors.centerIn: parent
        id: holder
        text: ""
        wrapMode: Text.WordWrap
        onTextChanged: tooltip.resetHolderSize()
    }

    NumberAnimation {
        id: appear
        target: tooltip
        property: "opacity"
        easing.type: Easing.InOutQuad
        duration: 300
        from: 0
        to: 1
    }

    NumberAnimation {
        id: disappear
        target: tooltip
        property: "opacity"
        easing.type: Easing.InOutQuad
        from: 1
        to: 0
        onStopped: visible = false;
    }

    onVisibleChanged: {
        if (visible) {
            appear.start();
        }
    }

    function show(posX, posY) {
        resetHolderSize();

        if (holder.width > 100) {
            holder.width = Math.sqrt(1.618 * holder.width * holder.height);
            holder.doLayout();
        }

        if (width > Root.width || height > Root.height) {
            holder.width = Root.width - horizontalPadding * 2;
            holder.doLayout();
            if (height > Root.height) {
                scale = Math.min(Root.width / width, Root.height / height);
            }
        } else {
            scale = 1;
        }

        var position = parent.mapToItem(Root, posX, posX);

        if (position.x + width * scale > Root.width) {
             posX += Root.width - (position.x + width * scale);
        }
        if (position.y + height * scale > Root.height) {
            posY += Root.height - (position.y + height * scale);
        }

        var oo = parent.mapFromItem(Root, 0, 0);
        x = Math.max(oo.x, posX);
        y = Math.max(oo.y, posY);

        visible = true;
    }

    function hide() {
        disappear.start();
    }

    function resetHolderSize() {
        holder.width = undefined;
        holder.height = undefined;
        holder.doLayout();
    }
}
