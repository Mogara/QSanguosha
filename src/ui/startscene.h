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

#ifndef _START_SCENE_H
#define _START_SCENE_H

#include "button.h"
#include "QSanSelectableItem.h"
#include "server.h"

#include <QGraphicsScene>
#include <QAction>
#include <QTextEdit>

class MainWindow;

#ifndef Q_OS_WINRT
class QDeclarativeComponent;
#endif

class StartScene : public QGraphicsScene {
    Q_OBJECT

public:
    StartScene(MainWindow *main_window);
    ~StartScene();

    void addButton(QAction *action);
    void setServerLogBackground();
    void switchToServer(Server *server);

    void showOrganization();

private:
    void printServerInfo();

    QSanSelectableItem *logo;
    QTextEdit *server_log;
    QList<Button *> buttons;

    MainWindow *main_window;

#ifndef Q_OS_WINRT
    QDeclarativeComponent *animationComponent;
#endif
};

#endif

