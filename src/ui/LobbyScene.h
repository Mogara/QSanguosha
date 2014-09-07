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
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

class QMainWindow;

class LobbyScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LobbyScene(QMainWindow *parent = 0);
    void adjustItems();

signals:

protected:

public slots:

private slots:
    void speakToServer();

private:
    QWidget *chatWidget;
    QLineEdit *chatLineEdit;
    QPlainTextEdit *chatBox;

    QGraphicsPixmapItem *userAvatarItem;
    QGraphicsTextItem *userNameItem;

    QWidget *buttonBox;
    QPushButton *refreshButton;
    QPushButton *createRoomButton;
    QPushButton *exitButton;

    QWidget *roomGrid;
};

#endif // LOBBYSCENE_H
