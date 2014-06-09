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
	 
	Image {
		id: heroPic
        x: -sceneWidth / 2
        fillMode: Image.PreserveAspectFit
        source: "../image/animate/" + hero + ".png"
		z: 100
    }
	
	SequentialAnimation {
        id: anim
        running: true
        onCompleted: {
            container.visible = false
            container.animationCompleted()
        }
		ParallelAnimation {
			id: step1
            PropertyAnimation {
				target: heroPic
				properties: "x"
                to: 0
                duration: 880
				easing {type: Easing.OutQuad}
            }
			PropertyAnimation {
				target: heroPic
                easing.overshoot: 1.802
                easing.type: Easing.OutBack
                properties: "scale"
				from: 0.5
				to: 1.0
                duration: 880
			}
            PropertyAnimation{
                target: mask
                properties: "opacity"
                to: 0.7
                duration: 880
            }
		}
	}
}
			
