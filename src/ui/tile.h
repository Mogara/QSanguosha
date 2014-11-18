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

#ifndef TILE_H
#define TILE_H

#include "button.h"
#include "title.h"

class Tile : public Button
{
    Q_OBJECT

public:
    explicit Tile(const QString &label, qreal scale = 1.0);
    explicit Tile(const QString &label, const QSizeF &size);

    void setAutoHideTitle(bool hide) { auto_hide_title = hide; title->setVisible(!hide); }
    bool autoHideTitle() const { return auto_hide_title; }

    void setIcon(QString path);
    void addScrollTexts(const QStringList &texts);
    void setScrollText(int index, const QString &text);

protected slots:
    void scrollToNextContent();

protected:
    enum MouseArea {
        Right,
        Left,
        Top,
        Bottom,
        Center,

        Outside
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    MouseArea getMouseArea(const QPointF &pos) const;

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void init();
    void doTransform(const QPointF &pos);
    void reset();

    bool down;
    bool auto_hide_title;

    MouseArea mouse_area;

    QGraphicsRotation *rotation;
    QGraphicsScale *scale;
    Title *title;
    QPixmap icon;
    QList<QGraphicsObject *> scroll_contents;
    int current_text_id;
    QTimer *scroll_timer;

    QGraphicsDropShadowEffect *frame;
};

#endif // TILE_H
