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
    id: mask
	x: 0
    width: sceneWidth
    height: sceneHeight
    color: "black"
    opacity: 0
	 
	Image {
		id: heroPic
        fillMode: Image.PreserveAspectFit
        source: "../image/animate/" + hero + ".png"
    }
	
	SequentialAnimation {
		running: true
		ParallelAnimation {
            running: true
			id: step1
            PropertyAnimation {
                running: true
				target: heroPic
				properties: "x"
				to: sceneWidth / 2
				duration: 880
				easing {type: Easing.OutQuad}
			}
			PropertyAnimation {
                running: true
				target: heroPic
                easing.overshoot: 1.802
                easing.type: Easing.OutBack
                properties: "scale"
				from: 0.5
				to: 1.0
				duration: 880
			}
            PropertyAnimation{
                running: true
                target: mask
                properties: "opacity"
				to: 0.7
				duration: 880
			}
		}
	}
}
			
