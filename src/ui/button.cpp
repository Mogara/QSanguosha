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

#include "button.h"
#include "title.h"
#include "skinbank.h"
#include "stylehelper.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

static QRectF ButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale)
    : label(label), size(ButtonRect.size() * scale),
      font_name("wqy-microhei"), font_size(Config.TinyFont.pixelSize())
{
    init();
}

Button::Button(const QString &label, const QSizeF &size)
    : label(label), size(size),
      font_name("wqy-microhei"), font_size(Config.TinyFont.pixelSize())
{
    init();
}

void Button::init()
{
    setAcceptHoverEvents(true);

    setAcceptedMouseButtons(Qt::LeftButton);
    setFlags(ItemIsFocusable);
    connect(this, &Button::enabledChanged, this, &Button::onEnabledChanged);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *)
{
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit clicked();
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    setFocus(Qt::MouseFocusReason);
}

void Button::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    clearFocus();
}

QRectF Button::boundingRect() const
{
    return QRectF(QPointF(), size);
}

QFont Button::defaultFont()
{
    QFont font = StyleHelper::getFontByFileName("wqy-microhei.ttc");
    font.setPixelSize(Config.TinyFont.pixelSize());
    return font;
}

static QColor ReverseColor(const QColor &color)
{
    int r = 0xFF - color.red();
    int g = 0xFF - color.green();
    int b = 0xFF - color.blue();

    return QColor::fromRgb(r, g, b);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    QRectF rect = boundingRect();

    QColor edgeColor = Qt::white, boxColor;
    int edgeWidth = 1;
    boxColor = Qt::black;
    if (hasFocus()) boxColor = ReverseColor(boxColor);
    boxColor.setAlphaF(0.8);
    edgeWidth = 2;

    painter->fillRect(rect, boxColor);

    QPen pen(edgeColor);
    pen.setWidth(edgeWidth);
    painter->setPen(pen);
    painter->drawRect(rect);

    QColor textColor = Qt::white;

    if (hasFocus())
        textColor = ReverseColor(textColor);

    IQSanComponentSkin::QSanSimpleTextFont ft;
    JsonArray val;
    val << font_name;
    val << font_size;
    val << 2;

    JsonArray val3;
    val3 << textColor.red();
    val3 << textColor.green();
    val3 << textColor.blue();
    val << QVariant(val3);

    ft.tryParse(val);
    ft.paintText(painter, rect.toRect(), Qt::AlignCenter, label);
}

void Button::onEnabledChanged()
{
    setOpacity(isEnabled() ? 1.0 : 0.2);
}
