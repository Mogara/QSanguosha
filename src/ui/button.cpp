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
#include <QFile>

static QRectF ButtonRect(0, 0, 154, 154);
static QRectF CompactButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale, bool compact)
    : label(label), size((compact ? CompactButtonRect.size() : ButtonRect.size()) * scale),
      font_name("wqy-microhei"), font_size(Config.TinyFont.pixelSize()),
      compact(compact), rotation(NULL), scale(NULL), title(NULL)
{
    init();
}

Button::Button(const QString &label, const QSizeF &size, bool compact)
    : label(label), size(size),
      font_name("wqy-microhei"), font_size(Config.TinyFont.pixelSize()),
      compact(compact), rotation(NULL), scale(NULL), title(NULL)
{
    init();
}

void Button::init() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif

    setAcceptedMouseButtons(Qt::LeftButton);

    if (!compact) {
        const QString path = QString("image/system/button/icon/%1.png").arg(label);
        if (QFile::exists(path)) {
            icon.load(path);
        }
        title = new Title(this, label, font_name, font_size);
        title->setPos(8, boundingRect().height() - title->boundingRect().height() - 8);
        title->hide();
    } else
        setFlags(ItemIsFocusable);
}

void Button::setFontName(const QString &name) {
    this->font_name = name;
}

void Button::setFontSize(const int &size) {
    this->font_size = size;
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (compact) return;
    qreal width = boundingRect().width(), height = boundingRect().height();
    QVector3D axis(0, 0, 0), origin(width / 2.0, height / 2.0, 0);
    qreal angle = 0;
    QPointF pos = event->pos();
    QList<QGraphicsTransform *> transformations;

    if (pos.x() > width - 30) {
        origin.setX(0);
        axis.setY(1);
        angle = 15;
    } else if (pos.x() < 30) {
        origin.setX(width);
        axis.setY(1);
        angle = -15;
    } else if (pos.y() < 30) {
        origin.setY(height);
        axis.setX(1);
        angle = 15;
    } else if (pos.y() > height - 30) {
        origin.setY(0);
        axis.setX(1);
        angle = -15;
    } else {
        scale = new QGraphicsScale;
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

    rotation = new QGraphicsRotation;
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
    if (!compact) reset();
    if (boundingRect().contains(event->pos()))
        emit clicked();
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    if (compact)
        setFocus(Qt::MouseFocusReason);
    else
        title->show();
}

void Button::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
    if (compact)
        clearFocus();
    else
        title->hide();
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
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    QRectF rect = boundingRect();

    QColor edgeColor = Qt::white, boxColor;
    int edgeWidth = 1;
    if (compact) {
        boxColor = Qt::black;
        if (hasFocus()) boxColor = ReverseColor(boxColor);
        boxColor.setAlphaF(0.8);
        edgeWidth = 2;
    }
    else {
        boxColor = QColor(120, 212, 120);
        edgeColor.setAlphaF(0.3);
    }

    painter->fillRect(rect, boxColor);

    QPen pen(edgeColor);
    pen.setWidth(edgeWidth);
    painter->setPen(pen);
    painter->drawRect(rect);

    if (compact) {
        QColor textColor = Qt::white;

        if (hasFocus())
            textColor = ReverseColor(textColor);

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
    else
        painter->drawPixmap(rect.toRect(), icon);
}

void Button::reset() {
    QList<QGraphicsTransform *> transformations;

    if (scale) {
        QPropertyAnimation *xScale_animation = new QPropertyAnimation(scale, "xScale", this);
        xScale_animation->setDuration(100);
        xScale_animation->setEndValue(1);
        QPropertyAnimation *yScale_animation = new QPropertyAnimation(scale, "yScale", this);
        yScale_animation->setDuration(100);
        yScale_animation->setEndValue(1);

        transformations << scale;

        setTransformations(transformations);
        xScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        yScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    if (rotation) {
        QPropertyAnimation *rotation_animation = new QPropertyAnimation(rotation, "angle", this);
        rotation_animation->setDuration(100);
        rotation_animation->setEndValue(0);

        transformations << rotation;

        setTransformations(transformations);
        rotation_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

Title::Title(QGraphicsObject *parent, const QString &text, const QString &font_name, const int &font_size)
: QGraphicsObject(parent), text(text), font_name(font_name), font_size(font_size)
{
}

QRectF Title::boundingRect() const {
    return QRectF(0, 0, font_size * text.length(), font_size + 1);
}

void Title::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QColor textColor = Qt::white;
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
    ft.paintText(painter, boundingRect().toRect(), Qt::AlignCenter, text);
}
