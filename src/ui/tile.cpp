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

#include "tile.h"

#include <QFile>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>
#include <QTimer>

static QRectF ButtonRect(0, 0, 154, 154);

Tile::Tile(const QString &label, const QSizeF &size)
    : Button(QPixmap(), size),
      down(false), auto_hide_title(true), mouse_area(Outside),
      rotation(NULL), scale(NULL), title(NULL), scroll_timer(NULL)
{
    this->label = label;
    init();
}

Tile::Tile(const QString &label, qreal scale)
    : Button(QPixmap(), scale),
      down(false), auto_hide_title(true), mouse_area(Outside),
      rotation(NULL), scale(NULL), title(NULL), scroll_timer(NULL)
{
    this->label = label;
    size = ButtonRect.size() * scale;
    init();
}

void Tile::init()
{
    title = new Title(this, label, font_name, font_size);
    title->setPos(8, boundingRect().height() - title->boundingRect().height() - 8);
    title->hide();

    setFlag(ItemIsFocusable, false);
}

void Tile::setIcon(QString path)
{
    QRegExp fileName("[\\w-.]+");
    if (fileName.exactMatch(path)) {
        path = QString("image/system/button/icon/%1.png").arg(path);
    }

    if (QFile::exists(path))
        m_icon->setPixmap(QPixmap(path));

    updateIconsPosition();
}

void Tile::addScrollTexts(const QStringList &texts)
{
    if (scroll_timer == NULL) {
        scroll_timer = new QTimer(this);
        scroll_timer->setSingleShot(true);
    } else {
        scroll_timer->stop();
        disconnect(scroll_timer, &QTimer::timeout, this, &Tile::scrollToNextContent);
    }

    if (!texts.isEmpty()) {
        foreach (const QString &text, texts) {
            Title *title = new Title(this, text, font_name, font_size - 2);
            title->setOpacity(0.0);
            title->setX(boundingRect().width() / 10);
            scroll_contents << title;
        }

        current_text_id = 0;
        QGraphicsObject *first_content = scroll_contents.first();
        first_content->setOpacity(1.0);
        first_content->setY(first_content->x());

        connect(scroll_timer, &QTimer::timeout, this, &Tile::scrollToNextContent);
        scroll_timer->start(((qrand() % 3) + 3) * 1000);
    }
}

void Tile::setScrollText(int index, const QString &text)
{
    Title *textItem = qobject_cast<Title *>(scroll_contents.at(index));
    if (textItem == NULL)
        return;
    textItem->setText(text);
}

void Tile::scrollToNextContent()
{
    QGraphicsObject *current = scroll_contents.at(current_text_id);
    current_text_id = (current_text_id + 1) % scroll_contents.size();
    QGraphicsObject *next = scroll_contents.at(current_text_id);
    int y = next->x();
    QPropertyAnimation *opacity1 = new QPropertyAnimation(current, "opacity");
    opacity1->setStartValue(1.0);
    opacity1->setEndValue(0.0);

    QPropertyAnimation *y1 = new QPropertyAnimation(current, "y");
    y1->setStartValue(y);
    y1->setEndValue(0.0);

    QPropertyAnimation *opacity2 = new QPropertyAnimation(next, "opacity");
    opacity2->setStartValue(0.0);
    opacity2->setEndValue(1.0);

    QPropertyAnimation *y2 = new QPropertyAnimation(next, "y");
    y2->setStartValue(boundingRect().height() - next->boundingRect().height());
    y2->setEndValue(y);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(opacity1);
    group->addAnimation(y1);
    group->addAnimation(opacity2);
    group->addAnimation(y2);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    scroll_timer->start(((qrand() % 3) + 3) * 1000);
}

void Tile::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->pos();
    bool inside = boundingRect().contains(pos);
    if (down && !inside) {
        down = false;
        reset();
    } else if (inside && boundingRect().contains(event->buttonDownPos(Qt::LeftButton))) {
        down = true;
        if (mouse_area != getMouseArea(pos))
            doTransform(pos);
    }
    mouse_area = getMouseArea(pos);
    QGraphicsObject::mouseMoveEvent(event);
}

void Tile::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    down = true;
    doTransform(event->pos());
}

void Tile::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    reset();
    if (down) {
        down = false;
        emit clicked();
    }
}

void Tile::doTransform(const QPointF &pos)
{
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

void Tile::reset()
{
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

void Tile::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (auto_hide_title)
        title->show();
}

void Tile::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (auto_hide_title)
        title->hide();
}


Tile::MouseArea Tile::getMouseArea(const QPointF &pos) const
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
