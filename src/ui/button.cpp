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
#include "SkinBank.h"
#include "jsonutils.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>

static QRectF ButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale)
    : label(label), size(ButtonRect.size() * scale), mute(true),
      font_name("wqy-microhei"), font_size(Config.SmallFont.pixelSize()),
      rotation(NULL), scale(NULL)
{
    init();
}

Button::Button(const QString &label, const QSizeF &size)
    : label(label), size(size), mute(true),
      font_name("wqy-microhei"), font_size(Config.SmallFont.pixelSize()),
      rotation(NULL), scale(NULL)
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

void Button::setFontName(const QString &name) {
    this->font_name = name;
}

void Button::setFontSize(const int &size) {
    this->font_size = size;
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    setFocus(Qt::MouseFocusReason);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    qreal width = boundingRect().width(), height = boundingRect().height();
    QVector3D axis(0, 0, 0), origin(width / 2.0, height / 2.0, 0);
    qreal angle = 0;
    QPointF pos = event->pos();
    QList<QGraphicsTransform *> transformations;

    if (pos.x() > width - 20) {
        origin.setX(0);
        axis.setY(1);
        angle = 15;
    } else if (pos.x() < 20) {
        origin.setX(width);
        axis.setY(1);
        angle = -15;
    } else if (pos.y() < 10) {
        origin.setY(height);
        axis.setX(1);
        angle = 15;
    } else if (pos.y() > height - 10) {
        origin.setY(0);
        axis.setX(1);
        angle = -15;
    } else {
        scale = new QGraphicsScale(this);
        QPropertyAnimation *xScale_animation = new QPropertyAnimation(scale, "xScale", this);
        xScale_animation->setDuration(100);
        xScale_animation->setStartValue(1);
        xScale_animation->setEndValue(0.95);
        QPropertyAnimation *yScale_animation = new QPropertyAnimation(scale, "yScale", this);
        yScale_animation->setDuration(100);
        yScale_animation->setStartValue(1);
        yScale_animation->setEndValue(0.95);

        scale->setOrigin(QVector3D(width / 2.0, height / 2.0, 0));
        transformations << scale;

        setTransformations(transformations);
        xScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        yScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    rotation = new QGraphicsRotation(this);
    QPropertyAnimation *rotation_animation = new QPropertyAnimation(rotation, "angle", this);
    rotation_animation->setDuration(100);
    rotation_animation->setStartValue(0);
    rotation_animation->setEndValue(angle);

    rotation->setAxis(axis);
    rotation->setOrigin(origin);
    transformations << rotation;

    setTransformations(transformations);
    rotation_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (boundingRect().contains(event->pos())) {
        if (!mute) Sanguosha->playSystemAudioEffect("button-down");
        emit clicked();
    }
    reset();
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

    using namespace QSanProtocol::Utils;
    IQSanComponentSkin::QSanSimpleTextFont ft;
    Json::Value val(Json::arrayValue);
    val[0] = toJsonString(font_name);
    val[1] = font_size;
    val[2] = 2;

    val[3] = Json::Value(Json::arrayValue);
    val[3][0] = textColor.red();
    val[3][1] = textColor.green();
    val[3][2] = textColor.blue();

    ft.tryParse(val);
    ft.paintText(painter, rect.toRect(), Qt::AlignCenter, label);
}

void Button::reset() {
    QList<QGraphicsTransform *> transformations;
    qreal width = boundingRect().width(), height = boundingRect().height();
    QVector3D origin(width / 2.0, height / 2.0, 0);

    if (scale) {
        QPropertyAnimation *xScale_animation = new QPropertyAnimation(scale, "xScale", this);
        xScale_animation->setDuration(100);
        xScale_animation->setEndValue(1);
        QPropertyAnimation *yScale_animation = new QPropertyAnimation(scale, "yScale", this);
        yScale_animation->setDuration(100);
        yScale_animation->setEndValue(1);

        scale->setOrigin(origin);
        transformations << scale;

        setTransformations(transformations);
        xScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        yScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    if (rotation) {
        QPropertyAnimation *rotation_animation = new QPropertyAnimation(rotation, "angle", this);
        rotation_animation->setDuration(100);
        rotation_animation->setEndValue(0);

        rotation->setAxis(QVector3D(0, 0, 0));
        rotation->setOrigin(origin);
        transformations << rotation;

        setTransformations(transformations);
        rotation_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
