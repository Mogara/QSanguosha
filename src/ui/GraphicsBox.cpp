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

#include "GraphicsBox.h"
#include "SkinBank.h"
#include "roomscene.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>

static int roundedRectRadius = 5;

GraphicsBox::GraphicsBox(const QString &title)
    : title(title)
{
    stylize(this);
}

GraphicsBox::~GraphicsBox()
{

}

void GraphicsBox::paintGraphicsBoxStyle(QPainter *painter, const QString &boxTitle, const QRectF &rect)
{
    painter->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    painter->save();
    painter->setBrush(QBrush(G_COMMON_LAYOUT.graphicsBoxBackgroundColor));
    const int x = rect.x();
    const int y = rect.y();
    const int w = rect.width();
    const int h = rect.height();
    painter->drawRoundedRect(x, y, w, h, roundedRectRadius, roundedRectRadius);
    painter->drawRoundedRect(x, y, w, 27, roundedRectRadius, roundedRectRadius);
    G_COMMON_LAYOUT.graphicsBoxTitleFont.paintText(painter, QRect(x, y, w, 27), Qt::AlignCenter, boxTitle);
    painter->restore();
    painter->setPen(G_COMMON_LAYOUT.graphicsBoxBorderColor);
    painter->drawRoundedRect(x + 1, y + 1, w - 2, h - 2, roundedRectRadius,
                             roundedRectRadius);
}

void GraphicsBox::stylize(QGraphicsObject *target)
{
    target->setFlags(ItemIsFocusable | ItemIsMovable);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(target);
    shadow->setOffset(4);
    shadow->setBlurRadius(5);
    shadow->setColor(QColor(0, 0, 0, 180));
    target->setGraphicsEffect(shadow);
}

void GraphicsBox::moveToCenter(QGraphicsObject *target)
{
    const QRectF rect = target->boundingRect();
    target->setPos(RoomSceneInstance->tableCenterPos()
                   - QPointF(rect.width() / 2, rect.height() / 2));
}

void GraphicsBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    paintGraphicsBoxStyle(painter, title, boundingRect());
}

void GraphicsBox::moveToCenter()
{
    moveToCenter(this);
}

void GraphicsBox::disappear()
{
    prepareGeometryChange();
    hide();
}
