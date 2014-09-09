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

#ifndef METROBUTTON_H
#define METROBUTTON_H

#include "button.h"
#include "Title.h"

class Tile : public Button
{
    Q_OBJECT

public:
    explicit Tile(const QString &label, qreal scale = 1.0);
    explicit Tile(const QString &label, const QSizeF &size);

    void setAutoHideTitle(bool hide) { auto_hide_title = hide; title->setVisible(!hide); }
    bool autoHideTitle() const { return auto_hide_title; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void init();
    void doTransform(const QPointF &pos);
    void reset();

    enum MouseArea {
        Right,
        Left,
        Top,
        Bottom,
        Center,

        Outside
    };

    bool down;
    bool auto_hide_title;

    MouseArea mouse_area;

    QGraphicsRotation *rotation;
    QGraphicsScale *scale;
    Title *title;

    QGraphicsDropShadowEffect *frame;

    QPixmap icon;

    MouseArea getMouseArea(const QPointF &pos) const;
};

#endif // METROBUTTON_H
