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

#include "LobbyScene.h"
#include "client.h"
#include "settings.h"
#include "SkinBank.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>

LobbyScene::LobbyScene(QMainWindow *parent) :
    QGraphicsScene(parent)
{
    chatBox = new QPlainTextEdit;
    chatBox->setObjectName("chat_log");
    chatBox->setReadOnly(true);
    chatLineEdit = new QLineEdit;
    chatLineEdit->setObjectName("chat_input");
    QVBoxLayout *chatLayout = new QVBoxLayout;
    chatLayout->addWidget(chatBox);
    chatLayout->addWidget(chatLineEdit);
    chatWidget = new QWidget;
    chatWidget->setObjectName("chat_box");
    chatWidget->setLayout(chatLayout);
    addWidget(chatWidget);

    //connect(chatSendButton, SIGNAL(clicked()), SLOT(speakToServer()));
    //connect(ClientInstance, SIGNAL(lineSpoken(const QString &)), chatBox, SLOT(append(const QString &)));

    userAvatarItem = addPixmap(G_ROOM_SKIN.getGeneralPixmap(Config.UserAvatar, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY));
    userNameItem = addText(Config.UserName);
    userNameItem->setDefaultTextColor(Qt::white);

    refreshButton = new QPushButton(tr("Refresh"));
    exitButton = new QPushButton(tr("Exit"));
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(exitButton);
    buttonBox = new QWidget;
    buttonBox->setObjectName("lobby_button_box");
    buttonBox->setLayout(buttonLayout);
    addWidget(buttonBox);

    QGridLayout *roomLayout = new QGridLayout;
    static int rowCount = 4;
    for (int i = 0; i < 11; i++) {
        QPushButton *label = new QPushButton(tr("Room #%1").arg(i));
        roomLayout->addWidget(label, i / rowCount, i % rowCount);
    }
    createRoomButton = new QPushButton(tr("+"));
    createRoomButton->setObjectName("create_room_button");
    roomLayout->addWidget(createRoomButton);
    roomGrid = new QWidget;
    roomGrid->setObjectName("lobby_rooms");
    roomGrid->setLayout(roomLayout);
    addWidget(roomGrid);
}

void LobbyScene::adjustItems()
{
    static int padding = 20;
    static int sceneTopMargin = 30;
    QRectF displayRegion = sceneRect();
    qreal scale = displayRegion.width() / 1366;

    chatWidget->resize(displayRegion.width() * 0.73, displayRegion.height() * 0.3);
    chatWidget->move(padding, displayRegion.height() - chatWidget->height() - padding);

    userAvatarItem->setScale(scale);
    userAvatarItem->setPos(displayRegion.right() - userAvatarItem->boundingRect().width() * scale - padding, sceneTopMargin + padding);
    userNameItem->setPos(userAvatarItem->x(), userAvatarItem->y() + userAvatarItem->boundingRect().height() * scale);

    buttonBox->resize(displayRegion.width() * 0.23, displayRegion.height() * 0.3);
    buttonBox->move(displayRegion.width() - buttonBox->width() - padding, displayRegion.height() - buttonBox->height() - padding);

    roomGrid->resize(displayRegion.width() * 0.82, displayRegion.height() * 0.6);
    roomGrid->move(padding, padding);
}

void LobbyScene::speakToServer()
{
    QString message = chatLineEdit->text();
    if (!message.isEmpty()) {
        chatLineEdit->clear();
        chatBox->appendPlainText(message);
        //ClientInstance->speakToServer(message);
    }
}
