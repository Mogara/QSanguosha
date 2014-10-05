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

#ifndef PLAYERCARDBOX_H
#define PLAYERCARDBOX_H

#include "graphicsbox.h"
#include "card.h"
#include "player.h"

class ClientPlayer;
class QGraphicsProxyWidget;
class QSanCommandProgressBar;

class PlayerCardBox : public GraphicsBox {
    Q_OBJECT

public:
    explicit PlayerCardBox();

    void chooseCard(const QString &reason, const ClientPlayer *player,
               const QString &flags = "hej", bool handcardVisible = false,
               Card::HandlingMethod method = Card::MethodNone,
               const QList<int> &disabledIds = QList<int>());
    void clear();

protected:
    // GraphicsBox interface
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    void paintArea(const QString &name, QPainter *painter);
    int getRowCount(const int &cardNumber) const;
    void updateNumbers(const int &cardNumber);
    void arrangeCards(const CardList &cards, const QPoint &topLeft);

    const ClientPlayer *player;
    QString flags;
    bool handcardVisible;
    Card::HandlingMethod method;
    QList<int> disabledIds;
    QList<CardItem *> items;

    QGraphicsProxyWidget *progressBarItem;
    QSanCommandProgressBar *progressBar;

    QList<QRect> nameRects;

    int rowCount;
    int intervalsBetweenAreas;
    int intervalsBetweenRows;
    int maxCardsInOneRow;

    static const int maxCardNumberInOneRow = 10;

    static const int verticalBlankWidth = 37;
    static const int placeNameAreaWidth = 15;
    static const int intervalBetweenNameAndCard = 20;
    static const int topBlankWidth = 42;
    static const int bottomBlankWidth = 25;
    static const int intervalBetweenAreas = 10;
    static const int intervalBetweenRows = 5;
    static const int intervalBetweenCards = 3;

public slots:
    void reply();
};

#endif // PLAYERCARDBOX_H
