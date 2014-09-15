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
#include "Title.h"
#include "SkinBank.h"
#include "StyleHelper.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QFile>
#include <QGraphicsDropShadowEffect>

static QRectF ButtonRect(0, 0, 154, 154);
static QRectF CompactButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale, bool compact)
    : label(label), size((compact ? CompactButtonRect.size() : ButtonRect.size()) * scale),
      font_name("wqy-microhei"), font_size(Config.TinyFont.pixelSize()),
      compact(compact), down(false), mouse_area(Outside),
      rotation(NULL), scale(NULL), title(NULL), frame(NULL)
{
    init();
}

Button::Button(const QString &label, const QSizeF &size, bool compact)
    : label(label), size(size),
      font_name("wqy-microhei"), font_size(Config.TinyFont.pixelSize()),
      compact(compact), down(false), mouse_area(Outside),
      rotation(NULL), scale(NULL), title(NULL), frame(NULL)
{
    init();
}

void Button::init()
{
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

        frame = new QGraphicsDropShadowEffect(this);
        frame->setOffset(0);
        frame->setColor(Qt::white);
        frame->setBlurRadius(12);
        frame->setEnabled(false);
        setGraphicsEffect(frame);
    } else {
        setFlags(ItemIsFocusable);
    }

    connect(this, SIGNAL(enabledChanged()), this, SLOT(onEnabledChanged()));
}

void Button::setFontName(const QString &name)
{
    this->font_name = name;
}

void Button::setFontSize(const int &size)
{
    this->font_size = size;
}

void Button::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->pos();
    bool inside = boundingRect().contains(pos);
    if (down && !inside) {
        down = false;
        reset();
    } else if(inside && boundingRect().contains(event->buttonDownPos(Qt::LeftButton))) {
        down = true;
        if (mouse_area != getMouseArea(pos))
            doTransform(pos);
    }
    mouse_area = getMouseArea(pos);
    QGraphicsObject::mouseMoveEvent(event);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    down = true;
    doTransform(event->pos());
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    reset();
    if (down) {
        down = false;
        emit clicked();
    }
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (compact) {
        setFocus(Qt::MouseFocusReason);
    } else {
        title->show();
        frame->setEnabled(true);
    }
}

void Button::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (compact) {
        clearFocus();
    } else {
        title->hide();
        frame->setEnabled(false);
    }
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
    if (compact) {
        boxColor = Qt::black;
        if (hasFocus()) boxColor = ReverseColor(boxColor);
        boxColor.setAlphaF(0.8);
        edgeWidth = 2;
    } else {
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
    } else {
        painter->drawPixmap(rect.toRect(), icon);
    }
}

void Button::doTransform(const QPointF &pos)
{
    if (compact) return;
    qreal width = boundingRect().width();
    qreal height = boundingRect().height();
    QVector3D axis(0, 0, 0);
    QVector3D origin(width / 2.0, height / 2.0, 0);
    qreal angle = 0;

    QList<QGraphicsTransform *> transforms;

    switch (getMouseArea(pos)) {
    case Right: {
        origin.setX(0);
        axis.setY(1);
        angle = 15;
        break;
    }
    case Left: {
        origin.setX(width);
        axis.setY(1);
        angle = -15;
        break;
    }
    case Top: {
        origin.setY(height);
        axis.setX(1);
        angle = 15;
        break;
    }
    case Bottom: {
        origin.setY(0);
        axis.setX(1);
        angle = -15;
        break;
    }
    default: {
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

        transforms << scale;

        setTransformations(transforms);
        xScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        yScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }
    }

    rotation = new QGraphicsRotation;
    QPropertyAnimation *rotation_animation = new QPropertyAnimation(rotation, "angle", this);
    rotation_animation->setDuration(100);
    rotation_animation->setStartValue(0);
    rotation_animation->setEndValue(angle);

    rotation->setAxis(axis);
    rotation->setOrigin(origin);

    transforms << rotation;

    setTransformations(transforms);
    rotation_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Button::reset()
{
    if (compact) return;

    QList<QGraphicsTransform *> transforms;

    if (scale) {
        QPropertyAnimation *xScale_animation = new QPropertyAnimation(scale, "xScale", this);
        xScale_animation->setDuration(100);
        xScale_animation->setEndValue(1);
        QPropertyAnimation *yScale_animation = new QPropertyAnimation(scale, "yScale", this);
        yScale_animation->setDuration(100);
        yScale_animation->setEndValue(1);

        transforms << scale;

        setTransformations(transforms);
        xScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
        yScale_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    if (rotation) {
        QPropertyAnimation *rotation_animation = new QPropertyAnimation(rotation, "angle", this);
        rotation_animation->setDuration(100);
        rotation_animation->setEndValue(0);

        transforms << rotation;

        setTransformations(transforms);
        rotation_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

Button::MouseArea Button::getMouseArea(const QPointF &pos) const
{
    QRectF rect = boundingRect();
    if (!boundingRect().contains(pos))
        return Outside;
    else if (pos.x() > rect.width() - 30)
        return Right;
    else if (pos.x() < 30)
        return Left;
    else if (pos.y() < 30)
        return Top;
    else if (pos.y() > rect.height() - 30)
        return Bottom;

    return Center;
}

void Button::onEnabledChanged()
{
    setOpacity(isEnabled() ? 1.0 : 0.2);
}
