/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _START_SCENE_H
#define _START_SCENE_H

#include "button.h"
#include "qsanselectableitem.h"
#include "server.h"

#include <QGraphicsScene>
#include <QAction>
#include <QTextEdit>

class StartScene : public QGraphicsScene {
    Q_OBJECT

public:
    StartScene(QObject *parent = 0);

    void addButton(QAction *action);
    void setServerLogBackground();
    void switchToServer(Server *server);

    void showOrganization();

private slots:
    void onSceneRectChanged(const QRectF &rect);

private:
    void printServerInfo();

    QSanSelectableItem *logo;
    QTextEdit *serverLog;
    QList<Button *> buttons;
    bool shouldMourn;
};

#endif

