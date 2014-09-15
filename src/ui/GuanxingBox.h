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

#ifndef GUANXINGBOX_H
#define GUANXINGBOX_H

#include "cardcontainer.h"

class GuanxingBox : public CardContainer {
    Q_OBJECT

public:
    GuanxingBox();
    void reply();
    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
    void doGuanxing(const QList<int> &card_ids, bool up_only);
    void clear();

    void mirrorGuanxingStart(const QString &who, bool up_only, const QList<int> &cards);
    void mirrorGuanxingMove(int from, int to);

private slots:
    void onItemReleased();
    void onItemClicked();

private:
    QList<CardItem *> upItems, downItems;
    bool up_only;
    void adjust();
    int itemNumberOfFirstRow() const;
    bool isOneRow() const;
    QString zhuge;
};

#endif // GUANXINGBOX_H
