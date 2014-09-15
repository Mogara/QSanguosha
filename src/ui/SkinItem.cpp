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

#include "SkinItem.h"
#include "SkinBank.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

const char *HEROSKIN_USED_ICON = "image/system/heroskin/used.png";
const char *HEROSKIN_SELECT_FRAME_ICON = "image/system/heroskin/select.png";

SkinItem::SkinItem(const QString &generalName, int skinId, bool used,
                   QGraphicsItem *parent/* = 0*/)
    : QGraphicsObject(parent),
      m_skinPixmap(G_ROOM_SKIN.getGeneralPixmap(generalName,
                                                QSanRoomSkin::S_GENERAL_ICON_SIZE_HERO_SKIN,
                                                skinId)),
      m_skinId(skinId), m_used(used), m_hoverEnter(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    const General *general = Sanguosha->getGeneral(generalName);
    if (skinId != 0)
        general->tryLoadingSkinTranslation(skinId);
    QGraphicsPixmapItem *titleItem = new QGraphicsPixmapItem(this);
    G_COMMON_LAYOUT.skinItemTitleText.paintText(titleItem, SKIN_ITEM_AREA,
                                                Qt::AlignRight | Qt::AlignBottom,
                                                general->getTitle(skinId));
}

QRectF SkinItem::boundingRect() const
{
    return QRectF(SKIN_ITEM_RECT);
}

void SkinItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    static QPixmap tempPix(SKIN_ITEM_RECT.size());
    tempPix.fill(Qt::transparent);

    QPainter tempPainter(&tempPix);
    tempPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath roundRectPath;
    roundRectPath.addRoundedRect(SKIN_ITEM_AREA, 8, 8);
    tempPainter.setClipPath(roundRectPath);
    tempPainter.drawPixmap(SKIN_ITEM_AREA, m_skinPixmap);

    if (m_used)
        tempPainter.drawPixmap(20, 42, getUsedIcon());

    QPen pen(Qt::black);
    pen.setWidthF(1);
    tempPainter.setPen(pen);
    tempPainter.drawRoundedRect(SKIN_ITEM_AREA, 8, 8);

    if (m_hoverEnter) {
        tempPainter.setClipRect(SKIN_ITEM_RECT);
        tempPainter.drawPixmap(SKIN_ITEM_RECT, getSelectFrameIcon());
    }

    painter->drawPixmap(0, 0, tempPix);
}

void SkinItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (!m_used) {
        m_hoverEnter = true;
        update();
    }
}

void SkinItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (!m_used) {
        m_hoverEnter = false;
        update();
    }
}

void SkinItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void SkinItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    if (!m_used && isUnderMouse()) {
        m_hoverEnter = false;
        update();

        emit clicked(m_skinId);
    }
}

const QPixmap &SkinItem::getUsedIcon()
{
    static const QPixmap usedIcon(HEROSKIN_USED_ICON);
    return usedIcon;
}

const QPixmap &SkinItem::getSelectFrameIcon()
{
    static const QPixmap selectFrameIcon(HEROSKIN_SELECT_FRAME_ICON);
    return selectFrameIcon;
}

