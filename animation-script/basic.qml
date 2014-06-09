/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

import QtQuick 1.0

Rectangle {
    id: container

    signal animationCompleted()

    Rectangle {
        id: mask
        x: 0
        width: sceneWidth
        height: sceneHeight
        color: "black"
        opacity: 0
        z: 0
    }

    Rectangle {
        id: flicker_mask
        x: 0
        width: sceneWidth
        height: sceneHeight
        color: "white"
        visible: false
        z: 1000
    }

    Image {
        id: heroPic
        x: -sceneWidth / 2
        fillMode: Image.PreserveAspectFit
        source: "../image/animate/" + hero + ".png"
        scale: 0.3
        z: 100
    }

    ParallelAnimation {
        id: step1
        running: true
        PropertyAnimation {
            target: heroPic
            properties: "x"
            to: 0
            duration: 400
            easing {type: Easing.OutQuad}
        }
        PropertyAnimation{
            target: mask
            properties: "opacity"
            to: 0.7
            duration: 880
        }
        onCompleted: {
            heroPic.visible = false
            flicker_mask.visible = true
            pause1.start()
        }
    }
    PauseAnimation {
        id: pause1
        duration: 20
        onCompleted: {
            flicker_mask.visible = false
            pause2.start()
        }
    }
    PauseAnimation {
        id: pause2
        duration: 80
        onCompleted: {
            flicker_mask.visible = true
            pause3.start()
        }
    }
    PauseAnimation {
        id: pause3
        duration: 20
        onCompleted: {
            flicker_mask.visible = false
            heroPic.visible = true
            step2.start()
        }
    }
    SequentialAnimation {
        id: step2
        onCompleted: {
            container.visible = false
            container.animationCompleted()
        }

        PropertyAnimation {
            id: zoom_in
            target: heroPic
            easing.overshoot: 6.252
            easing.type: Easing.OutBack
            properties: "scale"
            to: 1.0
            duration: 880
        }
    }
}

