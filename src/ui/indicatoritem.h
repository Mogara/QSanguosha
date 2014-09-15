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

#ifndef _INDICATOR_ITEM_H
#define _INDICATOR_ITEM_H

#include "player.h"

#include <QGraphicsObject>

class IndicatorItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF finish READ getFinish WRITE setFinish)

public:
    IndicatorItem(const QPointF &start, const QPointF &real_finish, Player *from);
    void doAnimation();

    QPointF getFinish() const;
    void setFinish(const QPointF &finish);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual QRectF boundingRect() const;

private:
    QPointF start, finish, real_finish;
    QColor color;
    qreal width;
};

#endif

