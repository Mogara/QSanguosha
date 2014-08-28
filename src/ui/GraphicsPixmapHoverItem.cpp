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

#include "GraphicsPixmapHoverItem.h"
#include "GenericCardContainerUI.h"
#include "pixmapanimation.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

static const char *CHANGE_SKIN_EMOTION_NAME = "skin_changing";
QList<QPixmap> GraphicsPixmapHoverItem::m_skinChangingFrames;
int GraphicsPixmapHoverItem::m_skinChangingFrameCount = 0;

GraphicsPixmapHoverItem::GraphicsPixmapHoverItem(PlayerCardContainer *playerCardContainer,
                                                 QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    m_playerCardContainer(playerCardContainer), m_timer(0), m_val(0),
    m_currentSkinChangingFrameIndex(-1)
{
    setAcceptHoverEvents(true);

    if (m_skinChangingFrames.isEmpty()) {
        initSkinChangingFrames();
    }
}

void GraphicsPixmapHoverItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    emit hover_enter();
}

void GraphicsPixmapHoverItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    emit hover_leave();
}

/*!
    \internal

    Highlights \a item as selected.

    NOTE: This function is a duplicate of qt_graphicsItem_highlightSelected() in
          qgraphicssvgitem.cpp!
*/
static void qt_graphicsItem_highlightSelected(
    QGraphicsItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
    if (qFuzzyIsNull(qMax(murect.width(), murect.height())))
        return;

    const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
    if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
        return;

    qreal itemPenWidth;
    switch (item->type()) {
        case QGraphicsEllipseItem::Type:
            itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
            break;
        case QGraphicsPathItem::Type:
            itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
            break;
        case QGraphicsPolygonItem::Type:
            itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
            break;
        case QGraphicsRectItem::Type:
            itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
            break;
        case QGraphicsSimpleTextItem::Type:
            itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
            break;
        case QGraphicsLineItem::Type:
            itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
            break;
        default:
            itemPenWidth = 1.0;
    }
    const qreal pad = itemPenWidth / 2;

    const qreal penWidth = 0; // cosmetic pen

    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( // ensure good contrast against fgcolor
        fgcolor.red()   > 127 ? 0 : 255,
        fgcolor.green() > 127 ? 0 : 255,
        fgcolor.blue()  > 127 ? 0 : 255);

    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}

void GraphicsPixmapHoverItem::paint(QPainter *painter,
    const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (pixmap().isNull()) {
        return;
    }

    QPixmap tempPix(pixmap().size());
    tempPix.fill(Qt::transparent);

    QPainter tempPainter(&tempPix);
    tempPainter.setRenderHint(QPainter::SmoothPixmapTransform,
        (transformationMode() == Qt::SmoothTransformation));

    if (m_val > 0) {
        tempPainter.drawPixmap(offset(), m_heroSkinPixmap);

        double percent = 1 - (double)m_val / (double)m_max;
        QRectF rect = QRectF(offset().x(), offset().y(),
            boundingRect().width(), percent * boundingRect().height());

        tempPainter.setClipRect(rect);
        tempPainter.drawPixmap(offset(), pixmap());

        tempPainter.setClipRect(boundingRect());
        tempPainter.drawPixmap(rect.left() - 9, rect.bottom() - 25,
            m_skinChangingFrames[m_currentSkinChangingFrameIndex]);

        // For tempPix may be processed outside, we should call tempPainter.end() to
        // release its control of tempPix, or an exception will occur
        tempPainter.end();
        if (!isAvatarOfDashboard() && isSecondaryAvartarItem()) {
            tempPix = m_playerCardContainer->paintByMask(tempPix);
        }
    } else {
        tempPainter.drawPixmap(offset(), pixmap());
    }

    if (option->state & QStyle::State_Selected) {
        qt_graphicsItem_highlightSelected(this, &tempPainter, option);
    }

    painter->drawPixmap(0, 0, tempPix);
}

void GraphicsPixmapHoverItem::timerEvent(QTimerEvent *)
{
    ++m_currentSkinChangingFrameIndex;
    if (m_currentSkinChangingFrameIndex >= m_skinChangingFrameCount) {
        m_currentSkinChangingFrameIndex = 0;
    }

    m_val += m_step;
    if (m_val >= m_max) {
        stopChangeHeroSkinAnimation();
        setPixmap(m_heroSkinPixmap);
        return;
    }

    update();
}

void GraphicsPixmapHoverItem::startChangeHeroSkinAnimation(const QString &generalName)
{
    if (m_timer != 0) {
        return;
    }

    emit skin_changing_start();

    if (NULL != m_playerCardContainer) {
        if (isPrimaryAvartarItem()) {
            m_heroSkinPixmap = m_playerCardContainer->getHeadAvatarIcon(generalName);
        } else {
            m_heroSkinPixmap = m_playerCardContainer->getDeputyAvatarIcon(generalName);
        }

        QSize itemSize = boundingRect().size().toSize();
        if (m_heroSkinPixmap.size() != itemSize) {
            m_heroSkinPixmap = m_heroSkinPixmap.scaled(itemSize, Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation);
        }

        m_timer = startTimer(m_interval);
    }
}

void GraphicsPixmapHoverItem::stopChangeHeroSkinAnimation()
{
    if (m_timer != 0) {
        killTimer(m_timer);
        m_timer = 0;
    }
    m_val = 0;
    m_currentSkinChangingFrameIndex = -1;

    emit skin_changing_finished();
}

void GraphicsPixmapHoverItem::initSkinChangingFrames()
{
    m_skinChangingFrameCount = PixmapAnimation::GetFrameCount(CHANGE_SKIN_EMOTION_NAME);
    for (int i = 0; i < m_skinChangingFrameCount; ++i) {
        QString fileName = QString("image/system/emotion/%1/%2.png")
            .arg(CHANGE_SKIN_EMOTION_NAME).arg(QString::number(i));

        QPixmap framePixmap = G_ROOM_SKIN.getPixmapFromFileName(fileName);
        m_skinChangingFrames << framePixmap.scaled(framePixmap.width() + 15,
            framePixmap.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

bool GraphicsPixmapHoverItem::isPrimaryAvartarItem() const
{
    return (this == m_playerCardContainer->getHeadAvartarItem());
}

bool GraphicsPixmapHoverItem::isSecondaryAvartarItem() const
{
    return (this == m_playerCardContainer->getDeputyAvartarItem());
}

bool GraphicsPixmapHoverItem::isAvatarOfDashboard() const
{
    return m_playerCardContainer->inherits("Dashboard");
}
