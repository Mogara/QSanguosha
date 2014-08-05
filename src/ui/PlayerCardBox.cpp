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

#include "PlayerCardBox.h"
#include "clientplayer.h"
#include "SkinBank.h"
#include "engine.h"
#include "carditem.h"
#include "client.h"
#include "TimedProgressBar.h"

#include <QGraphicsProxyWidget>

static QChar handcardFlag('h');
static QChar equipmentFlag('e');
static QChar judgingFlag('j');

PlayerCardBox::PlayerCardBox()
    : player(NULL), progressBar(NULL),
      rowCount(0), intervalsBetweenAreas(-1), intervalsBetweenRows(0), maxCardsInOneRow(0)
{
}

void PlayerCardBox::chooseCard(const QString &reason, const ClientPlayer *player,
                          const QString &flags, bool handcardVisible,
                          Card::HandlingMethod method, const QList<int> &disabledIds)
{
    this->player = player;
    this->title = reason;
    bool handcard = false;
    bool equip = false;
    bool judging = false;

    if (flags.contains(handcardFlag) && !player->isKongcheng()) {
        updateNumbers(player->getHandcardNum());
        handcard = true;
    }

    if (flags.contains(equipmentFlag) && !player->getEquips().isEmpty()) {
        updateNumbers(player->getEquips().length());
        equip = true;
    }

    if (flags.contains(judgingFlag) && !player->getJudgingArea().isEmpty()) {
        updateNumbers(player->getJudgingArea().length());
        judging = true;
    }

    maxCardsInOneRow = qMin(maxCardsInOneRow, maxCardNumberInOneRow);

    update();

    show();

    this->handcardVisible = handcardVisible;
    this->method = method;
    this->disabledIds = disabledIds;

    const int startX = verticalBlankWidth + placeNameAreaWidth + intervalBetweenNameAndCard;
    int index = 0;

    if (handcard) {
        if (Self == player || handcardVisible) {
            arrangeCards(player->getHandcards(), QPoint(startX, nameRects.at(index).y()));
        } else {
            const int handcardNumber = player->getHandcardNum();
            CardList cards;
            for(int i = 0; i < handcardNumber; ++ i)
                cards << NULL;
            arrangeCards(cards, QPoint(startX, nameRects.at(index).y()));
        }

        ++ index;
    }

    if (equip) {
        arrangeCards(player->getEquips(), QPoint(startX, nameRects.at(index).y()));

        ++ index;
    }

    if (judging)
        arrangeCards(player->getJudgingArea(), QPoint(startX, nameRects.at(index).y()));

    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar();
            progressBar->setMaximumWidth(qMin(boundingRect().width() - 16, (qreal) 150));
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progressBarItem = new QGraphicsProxyWidget(this);
            progressBarItem->setWidget(progressBar);
            progressBarItem->setPos(boundingRect().center().x() - progressBarItem->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, SIGNAL(timedOut()), this, SLOT(reply()));
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_CHOOSE_CARD);
        progressBar->show();
    }
}

QRectF PlayerCardBox::boundingRect() const
{
    if (player == NULL)
        return QRectF();

    if (rowCount == 0)
        return QRectF();

    const int cardWidth = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int cardHeight = G_COMMON_LAYOUT.m_cardNormalHeight;

    int width = verticalBlankWidth * 2 + placeNameAreaWidth + intervalBetweenNameAndCard;

    if (maxCardsInOneRow > maxCardNumberInOneRow / 2)
        width += cardWidth * maxCardNumberInOneRow / 2;
    else
        width += cardWidth * maxCardsInOneRow;

    int height = topBlankWidth + bottomBlankWidth + cardHeight * rowCount
            + intervalsBetweenAreas * qMax(intervalBetweenAreas, 0)
            + intervalsBetweenRows * intervalBetweenRows;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    return QRectF(0, 0, width, height);
}

void PlayerCardBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicsBox::paint(painter, option, widget);

    if (nameRects.isEmpty())
        return;

    foreach (const QRect &rect, nameRects)
        painter->drawRoundedRect(rect, 3, 3);

    int index = 0;

    if (flags.contains(handcardFlag)) {
        G_COMMON_LAYOUT.playerCardBoxPlaceNameText.paintText(painter, nameRects.at(index),
                                                             Qt::AlignCenter,
                                                             tr("Handcard area"));
        ++ index;
    }
    if (flags.contains(equipmentFlag)) {
        G_COMMON_LAYOUT.playerCardBoxPlaceNameText.paintText(painter, nameRects.at(index),
                                                             Qt::AlignCenter,
                                                             tr("Equip area"));
        ++ index;
    }
    if (flags.contains(judgingFlag)) {
        G_COMMON_LAYOUT.playerCardBoxPlaceNameText.paintText(painter, nameRects.at(index),
                                                             Qt::AlignCenter,
                                                             tr("Judging area"));
    }
}

void PlayerCardBox::clear()
{
    hide();

    if (progressBar != NULL){
        progressBar->hide();
        progressBar->deleteLater();
        progressBar = NULL;

        progressBarItem->deleteLater();
    }

    player = NULL;
    flags.clear();

    foreach(CardItem *item, items)
        item->deleteLater();
    items.clear();

    nameRects.clear();
    rowCount = 0;
    intervalsBetweenAreas = -1;
    intervalsBetweenRows = 0;
    maxCardsInOneRow = 0;
}

int PlayerCardBox::getRowCount(const int &cardNumber) const
{
    return (cardNumber + maxCardNumberInOneRow - 1) / maxCardNumberInOneRow;
}

void PlayerCardBox::updateNumbers(const int &cardNumber)
{
    ++ intervalsBetweenAreas;
    if (cardNumber > maxCardsInOneRow)
        maxCardsInOneRow = cardNumber;

    const int cardHeight = G_COMMON_LAYOUT.m_cardNormalHeight;
    const int y = topBlankWidth + rowCount * cardHeight
            + intervalsBetweenAreas * intervalBetweenAreas
            + intervalsBetweenRows * intervalBetweenRows;

    const int count = getRowCount(cardNumber);
    rowCount += count;
    intervalsBetweenRows += count - 1;

    const int height = count * cardHeight
            + (count - 1) * intervalsBetweenRows;

    nameRects << QRect(verticalBlankWidth, y, placeNameAreaWidth, height);
}

void PlayerCardBox::arrangeCards(const CardList &cards, const QPoint &topLeft)
{
    foreach(const Card *card, cards) {
        CardItem *item = new CardItem(card);
        item->resetTransform();
        item->setParentItem(this);
        item->setFlag(ItemIsMovable, false);
        item->setEnabled(!disabledIds.contains(item->getId())
                         && (method != Card::MethodDiscard
                             || Self->canDiscard(player, item->getId())));
        connect(item, SIGNAL(clicked()), this, SLOT(reply()));
        items << item;
    }

    int n = items.size();
    if (n == 0)
        return;

    const int rows = (n + maxCardNumberInOneRow - 1) / maxCardNumberInOneRow;
    QList<CardItem *> itemsCopy = items;
    const int cardWidth = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int cardHeight = G_COMMON_LAYOUT.m_cardNormalHeight;
    const int maxWidth = qMin(maxCardsInOneRow, maxCardNumberInOneRow / 2) * cardWidth;
    for(int row = 0; row < rows; ++ row) {
        int count = 0;
        if (row != rows - 1)
            count = maxCardNumberInOneRow;
        else
            count = itemsCopy.size();
        const double step = qMin((double)cardWidth, (double)(maxWidth - cardWidth) / (count - 1));
        for(int i = 0; i < count; ++ i) {
            CardItem *item = itemsCopy.takeFirst();
            const double x = topLeft.x() + step * i;
            const double y = topLeft.y() + (cardHeight + intervalBetweenRows) * row;
            item->setPos(x, y);
        }
    }
}

void PlayerCardBox::reply()
{
    QString name;
    if (sender()->inherits("CardItem")) {
        CardItem *asender = qobject_cast<CardItem *>(sender());
        int id = asender->getId();
        ClientInstance->onPlayerChooseCard(id);
    } else
        ClientInstance->onPlayerChooseCard();
    clear();
}
/*    QString name = sender()->objectName();
    if (name.isEmpty())
        return ClientInstance->onPlayerChooseCard();

    bool ok = true;
    int id = sender()->objectName().toInt(&ok);

    Q_ASSERT(ok);
    ClientInstance->onPlayerChooseCard(id);
}*/
