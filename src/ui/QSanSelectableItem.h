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

#ifndef _QSAN_SELECTABLE_ITEM_H
#define _QSAN_SELECTABLE_ITEM_H

#include <QGraphicsObject>
#include <QPixmap>

class QSanSelectableItem : public QGraphicsObject {
    Q_OBJECT

public:
    QSanSelectableItem(const QString &filename, bool center_as_origin = false);
    QSanSelectableItem(bool center_as_origin = false);
    virtual QRectF boundingRect() const;
    bool load(const QString &filename, bool center_as_origin = false);
    bool load(const QString &filename, QSize newSize, bool center_as_origin = false);
    void setPixmap(const QPixmap &pixmap);
    void makeGray();
    void scaleSmoothly(qreal ratio);

    bool isMarked() const;
    bool isMarkable() const;
    void mark(bool marked = true);
    void setMarkable(bool selectable);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int _m_width, _m_height;
    QPixmap _m_mainPixmap;

private:
    bool _load(const QString &filename, QSize newSize, bool useNewSize, bool center_as_origin);
    bool markable, marked;

signals:
    void mark_changed();
    void selected_changed();
    void enable_changed();
};

#endif

