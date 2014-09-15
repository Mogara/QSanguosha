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

#ifndef SKINITEM_H
#define SKINITEM_H

#include <QGraphicsObject>

const QRect SKIN_ITEM_AREA = QRect(4, 4, 125, 146);
const QRect SKIN_ITEM_RECT = QRect(0, 0, 133, 154);

class SkinItem : public QGraphicsObject
{
    Q_OBJECT

public:
    SkinItem(const QString &generalName, int skinId, bool used,
             QGraphicsItem *parent = 0);

    void setUsed(bool used) { m_used = used; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

private:
    static const QPixmap &getUsedIcon();
    static const QPixmap &getSelectFrameIcon();

private:
    const QPixmap m_skinPixmap;
    const int m_skinId;
    bool m_used;
    bool m_hoverEnter;

signals:
    void clicked(int skinId);
};

#endif // SKINITEM_H
