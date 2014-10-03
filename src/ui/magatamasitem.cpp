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

#include "magatamasitem.h"
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "skinbank.h"
#include "sprite.h"

MagatamasBoxItem::MagatamasBoxItem()
    : QGraphicsObject(NULL)
{
    m_hp = 0;
    m_maxHp = 0;
}

MagatamasBoxItem::MagatamasBoxItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    m_hp = 0;
    m_maxHp = 0;
}

void MagatamasBoxItem::setOrientation(Qt::Orientation orientation) {
    m_orientation = orientation;
    _updateLayout();
}

void MagatamasBoxItem::_updateLayout() {
    for (int i = 0; i < 4; i++) {
        _icons[i] = G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS).arg(QString::number(i)))
            .scaled(m_iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

void MagatamasBoxItem::setIconSize(QSize size) {
    m_iconSize = size;
    _updateLayout();
}

QRectF MagatamasBoxItem::boundingRect() const{
    int buckets = qMin(m_maxHp, 4) + G_COMMON_LAYOUT.m_hpExtraSpaceHolder;
    if (m_orientation == Qt::Horizontal)
        return QRectF(0, 0, buckets * m_iconSize.width(), m_iconSize.height());
    else
        return QRectF(0, 0, m_iconSize.width(), buckets * m_iconSize.height());
}

void MagatamasBoxItem::setHp(int hp) {
    _doHpChangeAnimation(hp);
    m_hp = hp;
    update();
}

void MagatamasBoxItem::setAnchor(QPoint anchor, Qt::Alignment align) {
    m_anchor = anchor;
    m_align = align;
}

void MagatamasBoxItem::setMaxHp(int maxHp) {
    m_maxHp = maxHp;
    _autoAdjustPos();
}

void MagatamasBoxItem::_autoAdjustPos() {
    if (!anchorEnabled) return;
    QRectF rect = boundingRect();
    Qt::Alignment hAlign = m_align & Qt::AlignHorizontal_Mask;
    if (hAlign == Qt::AlignRight)
        setX(m_anchor.x() - rect.width());
    else if (hAlign == Qt::AlignHCenter)
        setX(m_anchor.x() - rect.width() / 2);
    else
        setX(m_anchor.x());
    Qt::Alignment vAlign = m_align & Qt::AlignVertical_Mask;
    if (vAlign == Qt::AlignBottom)
        setY(m_anchor.y() - rect.height());
    else if (vAlign == Qt::AlignVCenter)
        setY(m_anchor.y() - rect.height() / 2);
    else
        setY(m_anchor.y());
}

void MagatamasBoxItem::update() {
    _updateLayout();
    _autoAdjustPos();
    QGraphicsItem::update();
}

void MagatamasBoxItem::_doHpChangeAnimation(int newHp) {
    if (newHp >= m_hp) return;

    int width = m_imageArea.width();
    int height = m_imageArea.height();
    int xStep, yStep;
    if (this->m_orientation == Qt::Horizontal) {
        xStep = width;
        yStep = 0;
    }
    else {
        xStep = 0;
        yStep = height;
    }

    int mHp = m_hp;
    if (m_hp < 0) {
        newHp -= m_hp;
        mHp = 0;
    }
    for (int i = qMax(newHp, mHp - 10), j = 0; i < mHp; ++i, ++j) {
        Sprite *aniMaga = new Sprite;
        aniMaga->setPixmap(_icons[qBound(0, i, 3)]);
        aniMaga->setParentItem(this);
        aniMaga->setOffset(QPoint(-(width - m_imageArea.left()) / 2, -(height - m_imageArea.top()) / 2));

        int lostHp = m_maxHp - mHp;
        if (lostHp >= m_maxHp)
            lostHp = m_maxHp - 1;

        lostHp += j;
        if (lostHp >= m_maxHp)
            lostHp = m_maxHp - 1;

        int pos = m_maxHp > 4 ? 0 : lostHp;
        aniMaga->setPos(QPoint(xStep * pos - aniMaga->offset().x(), yStep * pos - aniMaga->offset().y()));

        QPropertyAnimation *fade = new QPropertyAnimation(aniMaga, "opacity");
        fade->setEndValue(0);
        fade->setDuration(500);
        QPropertyAnimation *grow = new QPropertyAnimation(aniMaga, "scale");
        grow->setEndValue(4);
        grow->setDuration(500);

        connect(fade, SIGNAL(finished()), aniMaga, SLOT(deleteLater()));

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(fade);
        group->addAnimation(grow);

        group->start(QAbstractAnimation::DeleteWhenStopped);

        aniMaga->show();
    }
}

void MagatamasBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_maxHp <= 0) return;

    int imageIndex = qBound(0, m_hp, 3);
    if (m_hp == m_maxHp) imageIndex = 3;

    int xStep, yStep;
    double blank;
    int bgWidth, bgHeight;
    if (this->m_orientation == Qt::Horizontal) {
        xStep = m_iconSize.width();
        yStep = 0;
        blank = (xStep - m_imageArea.width()) / 2;
        bgWidth = xStep * qMin(m_maxHp, 4) + 5;
        bgHeight = m_iconSize.height() + 2;
    } else {
        xStep = 0;
        yStep = m_iconSize.height();
        blank = (yStep - m_imageArea.height()) / 2;
        bgWidth = m_iconSize.width() + 2;
        bgHeight = yStep * qMin(m_maxHp, 4) + 5;
    }

    if (m_showBackground) {
        QRect rect(-2, -2, bgWidth, bgHeight);
        painter->drawPixmap(rect, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS_BG));
    }

    if (m_maxHp <= 4) {
        int i;
        for (i = 0; i < m_maxHp - qMax(m_hp, 0); i++) {
            QRect rect(xStep * i + blank, yStep * i + blank, m_imageArea.width(), m_imageArea.height());
            //rect.translate(m_imageArea.topLeft());
            painter->drawPixmap(rect, _icons[0]);
        }
        for (; i < m_maxHp; i++) {
            QRect rect(xStep * i + blank, yStep * i + blank, m_imageArea.width(), m_imageArea.height());
            //rect.translate(m_imageArea.topLeft());
            painter->drawPixmap(rect, _icons[imageIndex]);
        }
    } else {
        painter->drawPixmap(m_imageArea, _icons[imageIndex]);
        QRect rect(xStep, yStep, m_imageArea.width(), m_imageArea.height());
        rect.translate(m_imageArea.topLeft());
        if (this->m_orientation == Qt::Horizontal)
            rect.translate(xStep * 0.5, yStep * 0.5);
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(painter, rect, Qt::AlignCenter, QString::number(m_hp));
        rect.translate(xStep, yStep);
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(painter, rect, Qt::AlignCenter, "/");
        rect.translate(xStep, yStep);
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(painter, rect, Qt::AlignCenter, QString::number(m_maxHp));
    }
}

