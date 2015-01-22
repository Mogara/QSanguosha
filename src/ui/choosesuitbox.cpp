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

#include "choosesuitbox.h"
#include "skinbank.h"
#include "timedprogressbar.h"
#include "clientstruct.h"
#include "client.h"
#include "button.h"

#include <QGraphicsProxyWidget>

const int ChooseSuitBox::outerBlankWidth = 37;
const int ChooseSuitBox::buttonWidth = 35;
const int ChooseSuitBox::buttonHeight = 30;
const int ChooseSuitBox::interval = 15;
const int ChooseSuitBox::topBlankWidth = 42;
const int ChooseSuitBox::bottomBlankWidth = 25;

ChooseSuitBox::ChooseSuitBox()
    : progressBar(NULL)
{
    title = tr("Please choose a suit");
}

QRectF ChooseSuitBox::boundingRect() const
{
    const int width = buttonWidth * suitNumber
            + outerBlankWidth * 2
            + interval * (suitNumber - 1);

    int height = topBlankWidth + buttonHeight + bottomBlankWidth;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    return QRectF(0, 0, width, height);
}

void ChooseSuitBox::chooseSuit(const QStringList &suits)
{
    suitNumber = suits.size();
    m_suits = suits;
    prepareGeometryChange();

    foreach (const QString &suit, suits) {
        QPixmap icon = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_SUIT, suit);
        Button *button = new Button(icon, QSizeF(buttonWidth, buttonHeight));
        button->setObjectName(suit);
        buttons << button;
        button->setParentItem(this);

        connect(button, &Button::clicked, this, &ChooseSuitBox::reply);
    }

    moveToCenter();
    show();

    for (int i = 0; i < buttons.length(); ++i) {
        Button *button = buttons.at(i);

        QPointF pos;
        pos.setX(outerBlankWidth + (buttonWidth + interval) * i);
        pos.setY(topBlankWidth);

        button->setPos(pos);
    }

    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar();
            progressBar->setMaximumWidth(boundingRect().width() - 16);
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progressBarItem = new QGraphicsProxyWidget(this);
            progressBarItem->setWidget(progressBar);
            progressBarItem->setPos(boundingRect().center().x() - progressBarItem->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, &QSanCommandProgressBar::timedOut, this, &ChooseSuitBox::reply);
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_MULTIPLE_CHOICE);
        progressBar->show();
    }
}

void ChooseSuitBox::reply()
{
    QString suit = sender()->objectName();
    if (suit.isEmpty())
        suit = m_suits.at(qrand() % suitNumber);
    ClientInstance->onPlayerChooseSuit(suit);
}

void ChooseSuitBox::clear()
{
    if (progressBar != NULL){
        progressBar->hide();
        progressBar->deleteLater();
        progressBar = NULL;
    }

    foreach (Button *button, buttons)
        button->deleteLater();

    buttons.clear();

    disappear();
}
