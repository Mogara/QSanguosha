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
    : label(label), size(ButtonRect.size() * scale)
{
    init();
    initTextItems();
    prepareIcons();
}

Button::Button(const QPixmap &pixmap, qreal scale)
    : size(ButtonRect.size() * scale),
      m_icon(new QGraphicsPixmapItem(pixmap, this)), m_colorReversedIcon(NULL)
{
    init();
    prepareIcons();
}

Button::Button(const QString &label, const QSizeF &size)
    : label(label), size(size)
{
    init();
    initTextItems();
    prepareIcons();
}

Button::Button(const QPixmap &pixmap, const QSizeF &size)
    : size(size),
      m_icon(new QGraphicsPixmapItem(pixmap, this)), m_colorReversedIcon(NULL)
{
    init();
    prepareIcons();
}

void Button::init()
{
    setAcceptHoverEvents(true);

    font_name = "wqy-microhei";
    font_size = Config.TinyFont.pixelSize();

    setAcceptedMouseButtons(Qt::LeftButton);
    setFlags(ItemIsFocusable);
    connect(this, &Button::enabledChanged, this, &Button::onEnabledChanged);
}

void Button::initTextItems()
{
    //@todo:modify the properties of QSanSimpleTextFont directly instead
    IQSanComponentSkin::QSanSimpleTextFont ft;
    JsonArray val;
    val << font_name;
    val << font_size;
    val << 2;

    JsonArray val3;
    val3 << 255 << 255 << 255; //white
    val << QVariant(val3);

    ft.tryParse(val);

    QRect rect = boundingRect().toRect();

    m_icon = new QGraphicsPixmapItem(this);
    ft.paintText(m_icon, rect, Qt::AlignCenter, label);

    val3.clear();
    val3 << 0 << 0 << 0; //black
    val[3] = val3;
    ft.tryParse(val);

    m_colorReversedIcon = new QGraphicsPixmapItem(this);
    ft.paintText(m_colorReversedIcon, rect, Qt::AlignCenter, label);
    m_colorReversedIcon->hide();
}

void Button::prepareIcons()
{
    m_icon->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    if (m_colorReversedIcon != NULL)
        m_colorReversedIcon->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

    updateIconsPosition();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit clicked();
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    setFocus(Qt::MouseFocusReason);
    setTextColorReversed(true);
}

void Button::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    clearFocus();
    setTextColorReversed(false);
}

void Button::setTextColorReversed(bool reversed)
{
    if (m_colorReversedIcon != NULL) {
        m_icon->setVisible(!reversed);
        m_colorReversedIcon->setVisible(reversed);
    }
}

void Button::updateIconsPosition()
{
    const qreal width = m_icon->boundingRect().width();
    const qreal height = m_icon->boundingRect().height();

    const QRectF rect = boundingRect();
    m_icon->setPos((rect.width() - width) / 2, (rect.height() - height) / 2);

    if (m_colorReversedIcon != NULL)
        m_colorReversedIcon->setPos(m_icon->pos());
}

QColor Button::backgroundColor() const
{
    QColor color;

    if (hasFocus())
        color = Qt::white;
    else
        color = Qt::black;

    color.setAlphaF(0.8);

    return color;
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

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QRectF rect = boundingRect();

    QPen pen(edgeColor());
    pen.setWidth(edgeWidth());
    painter->setPen(pen);
    painter->setBrush(backgroundColor());
    painter->drawRect(rect);
}

void Button::onEnabledChanged()
{
    setOpacity(isEnabled() ? 1.0 : 0.2);
}
