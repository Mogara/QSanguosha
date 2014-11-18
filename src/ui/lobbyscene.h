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

#ifndef LOBBYSCENE_H
#define LOBBYSCENE_H

#include "clientstruct.h"

#include <QGraphicsScene>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

class QMainWindow;
class Tile;
class Title;
class Client;

class LobbyScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LobbyScene(QMainWindow *parent = 0);
    ~LobbyScene();
    void adjustRoomTiles();

signals:
    void roomSelected(int room_id);
    void roomSelected();
    void createRoomClicked();
    void roomListRequested(int page);
    void exit();

public slots:
    void setRoomList(const QList<RoomInfoStruct> *data);

private slots:
    void speakToServer();

    void refreshRoomList();
    void prevPage();
    void nextPage();

    void onRoomTileClicked();
    void onCreateRoomClicked();
    void onClientDestroyed();
    void onSceneRectChanged(const QRectF &rect);
    void onRoomChanged(int index);

private:
    QWidget *chatWidget;
    QLineEdit *chatLineEdit;
    QTextEdit *chatBox;

    QGraphicsPixmapItem *userAvatarItem;
    Title *userNameItem;

    QWidget *buttonBox;
    QPushButton *refreshButton;
    QPushButton *exitButton;

    Title *roomTitle;

    const QList<RoomInfoStruct> *rooms;
    QList<Tile *> roomTiles;
    Tile *createRoomTile;
    int currentPage;

    Client *client;

    static int SCENE_PADDING;
    static int SCENE_MARGIN_TOP;
};

#endif // LOBBYSCENE_H
