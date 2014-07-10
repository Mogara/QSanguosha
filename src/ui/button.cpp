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

#include "button.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

static QRectF ButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale)
    : label(label), size(ButtonRect.size() * scale), mute(true), font(Config.SmallFont)
{
    init();
}

Button::Button(const QString &label, const QSizeF &size)
    : label(label), size(size), mute(true), font(Config.SmallFont)
{
    init();
}

void Button::init() {
    setFlags(ItemIsFocusable);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void Button::setMute(bool mute) {
    this->mute = mute;
}

void Button::setFont(const QFont &font) {
    this->font = font;
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    setFocus(Qt::MouseFocusReason);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
    if (!mute) Sanguosha->playSystemAudioEffect("button-down");
    emit clicked();
}

QRectF Button::boundingRect() const{
    return QRectF(QPointF(), size);
}

static QColor ReverseColor(const QColor &color) {
    int r = 0xFF - color.red();
    int g = 0xFF - color.green();
    int b = 0xFF - color.blue();

    return QColor::fromRgb(r, g, b);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
	QRectF rect = boundingRect();

	QColor textColor, edgeColor, boxColor;
    textColor = edgeColor = Qt::white;
    boxColor = Qt::black;

	if (hasFocus()) {
        textColor = ReverseColor(textColor);
        boxColor = ReverseColor(boxColor);
	}

	boxColor.setAlphaF(0.8);

	painter->fillRect(rect, boxColor);

	QPen pen(edgeColor);
	pen.setWidth(2);
	painter->setPen(pen);
	painter->drawRect(rect);

    pen.setColor(textColor);
    painter->setPen(pen);
	painter->setFont(font);
	painter->drawText(rect, Qt::AlignCenter, label);
}