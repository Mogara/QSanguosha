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

#include "sprite.h"

#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPainter>

EffectAnimation::EffectAnimation()
    : QObject()
{
    effects.clear();
    registered.clear();
}

void EffectAnimation::fade(QGraphicsItem *map) {
    AnimatedEffect *effect = qobject_cast<AnimatedEffect *>(map->graphicsEffect());
    if (effect) {
        effectOut(map);
        effect = registered.value(map);
        if (effect) effect->deleteLater();
        registered.insert(map, new FadeEffect(true, this));
        return;
    }

    map->show();
    FadeEffect *fade = new FadeEffect(true, this);
    map->setGraphicsEffect(fade);
    effects.insert(map, fade);
}

void EffectAnimation::emphasize(QGraphicsItem *map, bool stay) {
    AnimatedEffect *effect = qobject_cast<AnimatedEffect *>(map->graphicsEffect());
    if (effect) {
        effectOut(map);
        effect = registered.value(map);
        if (effect) effect->deleteLater();
        registered.insert(map, new EmphasizeEffect(stay, this));
        return;
    }

    EmphasizeEffect *emphasize = new EmphasizeEffect(stay, this);
    map->setGraphicsEffect(emphasize);
    effects.insert(map, emphasize);
}

void EffectAnimation::sendBack(QGraphicsItem *map) {
    AnimatedEffect *effect = qobject_cast<AnimatedEffect *>(map->graphicsEffect());
    if (effect) {
        effectOut(map);
        effect = registered.value(map);
        if (effect) effect->deleteLater();
        registered.insert(map, new SentbackEffect(true, this));
        return;
    }

    SentbackEffect *sendBack = new SentbackEffect(true, this);
    map->setGraphicsEffect(sendBack);
    effects.insert(map, sendBack);
}

void EffectAnimation::effectOut(QGraphicsItem *map) {
    AnimatedEffect *effect = qobject_cast<AnimatedEffect *>(map->graphicsEffect());
    if (effect) {
        connect(effect, &AnimatedEffect::loop_finished, this, (void (EffectAnimation::*)()) (&EffectAnimation::deleteEffect));
        effect->setStay(false);
    }

    effect = registered.value(map);
    if (effect)
        effect->deleteLater();

    registered.insert(map, NULL);
}

void EffectAnimation::deleteEffect() {
    AnimatedEffect *effect = qobject_cast<AnimatedEffect *>(sender());
    deleteEffect(effect);
}

void EffectAnimation::deleteEffect(AnimatedEffect *effect) {
    if (!effect) return;
    effect->deleteLater();
    QGraphicsItem *pix = effects.key(effect);
    if (pix) {
        AnimatedEffect *effect = registered.value(pix);
        if (effect) effect->reset();
        pix->setGraphicsEffect(registered.value(pix));
        effects.insert(pix, registered.value(pix));
        registered.insert(pix, NULL);
    }
}

EmphasizeEffect::EmphasizeEffect(bool stay, QObject *parent) {
    this->setObjectName("emphasizer");
    this->setParent(parent);
    index = 0;
    this->stay = stay;
    QPropertyAnimation *anim = new QPropertyAnimation(this, "index");
    connect(anim, &QPropertyAnimation::valueChanged, this, &EmphasizeEffect::update);
    anim->setEndValue(40);
    anim->setDuration((40 - index) * 5);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void EmphasizeEffect::draw(QPainter *painter) {
    QSizeF s = this->sourceBoundingRect().size();
    qreal scale = (-qAbs(index - 50) + 50) / 1000.0;
    scale = 0.1 - scale;

    QPoint offset;
    QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
    const QRectF target = boundingRect().adjusted(s.width() * scale - 1,
        s.height() * scale,
        -s.width() * scale,
        -s.height() * scale);
    const QRectF source(s.width() * 0.1, s.height() * 0.1, s.width(), s.height());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(target, pixmap, source);
}

QRectF EmphasizeEffect::boundingRectFor(const QRectF &sourceRect) const{
    qreal scale = 0.1;
    QRectF rect(sourceRect);
    rect.adjust(-sourceRect.width() * scale,
        -sourceRect.height() * scale,
        sourceRect.width() * scale,
        sourceRect.height() * scale);
    return rect;
}

void AnimatedEffect::setStay(bool stay) {
    this->stay = stay;
    if (!stay) {
        QPropertyAnimation *anim = new QPropertyAnimation(this, "index");
        anim->setEndValue(0);
        anim->setDuration(index * 5);

        connect(anim, &QPropertyAnimation::finished, this, &AnimatedEffect::deleteLater);
        connect(anim, &QPropertyAnimation::finished, this, &AnimatedEffect::loop_finished);
        connect(anim, &QPropertyAnimation::valueChanged, this, &AnimatedEffect::update);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

SentbackEffect::SentbackEffect(bool stay, QObject *parent) {
    grayed = NULL;
    this->setObjectName("backsender");
    this->setParent(parent);
    index = 0;
    this->stay = stay;

    QPropertyAnimation *anim = new QPropertyAnimation(this, "index");
    connect(anim, &QPropertyAnimation::valueChanged, this, &SentbackEffect::update);
    anim->setEndValue(40);
    anim->setDuration((40 - index) * 5);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF SentbackEffect::boundingRectFor(const QRectF &sourceRect) const{
    qreal scale = 0.05;
    QRectF rect(sourceRect);
    rect.adjust(-sourceRect.width() * scale,
        -sourceRect.height() * scale,
        sourceRect.width() * scale,
        sourceRect.height() * scale);
    return rect;
}

void SentbackEffect::draw(QPainter *painter) {
    QPoint offset;
    QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);

    if (!grayed) {
        grayed = new QImage(pixmap.size(), QImage::Format_ARGB32);

        QImage image = pixmap.toImage();
        int width = image.width();
        int height = image.height();
        int gray;

        QRgb col;

        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                col = image.pixel(i, j);
                gray = qGray(col) >> 1;
                grayed->setPixel(i, j, qRgba(gray, gray, gray, qAlpha(col)));
            }
        }
    }

    painter->drawPixmap(offset, pixmap);
    painter->setOpacity((40 - qAbs(index - 40)) / 80.0);
    painter->drawImage(offset, *grayed);

    return;
}

SentbackEffect::~SentbackEffect() {
    if (grayed){
        delete grayed;
        grayed = NULL;
    }
}

FadeEffect::FadeEffect(bool stay, QObject *parent) {
    this->setObjectName("fader");
    this->setParent(parent);
    index = 0;
    this->stay = stay;

    QPropertyAnimation *anim = new QPropertyAnimation(this, "index");
    connect(anim, &QPropertyAnimation::valueChanged, this, &FadeEffect::update);
    anim->setEndValue(40);
    anim->setDuration((40 - index) * 5);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FadeEffect::draw(QPainter *painter) {
    QPoint offset;
    QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
    painter->setOpacity(index / 40.0);
    painter->drawPixmap(offset, pixmap);
}

