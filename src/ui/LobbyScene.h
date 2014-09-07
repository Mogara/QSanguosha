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

#ifndef LOBBYSCENE_H
#define LOBBYSCENE_H

#include <QGraphicsScene>

class QMainWindow;

class LobbyScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LobbyScene(QMainWindow *parent = 0);

signals:

public slots:

};

#endif // LOBBYSCENE_H
