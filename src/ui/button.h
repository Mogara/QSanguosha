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

#ifndef _BUTTON_H
#define _BUTTON_H

#include "settings.h"

#include <QGraphicsObject>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>

class Button : public QGraphicsObject{
    Q_OBJECT

public:
    explicit Button(const QString &label, qreal scale = 1.0);
    explicit Button(const QString &label, const QSizeF &size);
    void setMute(bool mute);
    void setFont(const QFont &font);

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void timerEvent(QTimerEvent *);

private:
    QString label;
    QSizeF size;
    bool mute;
    QFont font;
    QPixmap title;
    QImage outimg;
    QGraphicsPixmapItem *title_item;
    int glow;
    int timer_id;

    QGraphicsDropShadowEffect *de;
    QGraphicsDropShadowEffect *effect;

    void init();

signals:
    void clicked();
};

#endif

