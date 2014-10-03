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

#include "heroskincontainer.h"
#include "skinitem.h"
#include "qsanbutton.h"
#include "settings.h"
#include "genericcardcontainerui.h"
#include "engine.h"
#include "clientplayer.h"
#include "stylehelper.h"

#include <QPainter>
#include <QCursor>
#include <QScrollBar>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsProxyWidget>

static const int LEFT_MARGIN = 5;
static const int AVAILABLE_AREA_WIDTH = 400;
static const int Y_STEP = 12;
static const int Y_START_POS = 32;

static const int SKIN_ITEM_WIDTH = SKIN_ITEM_AREA.width();
static const int SKIN_ITEM_HEIGHT = SKIN_ITEM_AREA.height();

HeroSkinContainer *HeroSkinContainer::m_currentTopMostContainer = NULL;

HeroSkinContainer::HeroSkinContainer(const QString &generalName,
    const QString &kingdom, QGraphicsItem *parent/* = 0*/)
    : QGraphicsObject(parent), m_generalName(generalName),
    m_backgroundPixmap("image/system/heroskin/container.png"),
    m_vScrollBar(NULL), m_oldScrollValue(0)
{
    setFlag(ItemIsMovable);
    setCursor(Qt::ArrowCursor);

    QSanButton *closeButton = new QSanButton("heroskin", "close", this);
    closeButton->setPos(387, 5);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QGraphicsPixmapItem *nameBgItem = NULL;
    PlayerCardContainer::_paintPixmap(nameBgItem, QRect(11, 6, 87, 18),
        G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, kingdom), this);

    QGraphicsPixmapItem *positionIcon = NULL;
    QString key = (Self->getGeneralName() == m_generalName) ?
                QSanRoomSkin::S_SKIN_KEY_HEAD_ICON : QSanRoomSkin::S_SKIN_KEY_DEPUTY_ICON;
    PlayerCardContainer::_paintPixmap(positionIcon, QRect(9, 3, 29, 24),
                                      G_ROOM_SKIN.getPixmap(key), this);

    QString name = Sanguosha->translate("&" + m_generalName);
    if (name.startsWith("&")) {
        name = Sanguosha->translate(m_generalName);
    }

    QGraphicsPixmapItem *avatarNameItem = new QGraphicsPixmapItem(this);
    G_DASHBOARD_LAYOUT.m_avatarNameFont.paintText(avatarNameItem,
    G_COMMON_LAYOUT.generalButtonNameRegion,
    Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignJustify, name);
    avatarNameItem->setPos(35, 1);

    initSkins();
    fillSkins();
}

bool HeroSkinContainer::hasSkin(const QString &generalName)
{
    return Sanguosha->getGeneral(generalName)->skinCount() > 0;
}

int HeroSkinContainer::getNextSkinIndex(const QString &generalName, int skinIndex)
{
    int result = skinIndex + 1;

    if (result >= Sanguosha->getGeneral(generalName)->skinCount())
        result = 0;

    return result;
}

void HeroSkinContainer::initSkins()
{
    QGraphicsRectItem *dummyRectItem = new QGraphicsRectItem(QRectF(LEFT_MARGIN, 35,
        AVAILABLE_AREA_WIDTH, 174), this);
    dummyRectItem->setFlag(ItemHasNoContents);
    dummyRectItem->setFlag(ItemClipsChildrenToShape);

    int skinIndexUsed = getCurrentSkinId();
    createSkinItem(skinIndexUsed, dummyRectItem, true);

    const int skinCount = Sanguosha->getGeneral(m_generalName)->skinCount();
    for (int i = 0; i < skinCount;)
        createSkinItem(++i, dummyRectItem);

    //default skin
    if (0 != skinIndexUsed) {
        createSkinItem(0, dummyRectItem);
    }
}

void HeroSkinContainer::createSkinItem(int skinId, QGraphicsItem *parent, bool used/* = false*/)
{
    SkinItem *skinItem = new SkinItem(m_generalName, skinId, used, parent);
    connect(skinItem, SIGNAL(clicked(int)), this, SLOT(skinSelected(int)));
    m_skins << skinItem;
    m_skinIndexToItem[skinId] = skinItem;
}

void HeroSkinContainer::fillSkins()
{
    int skinCount = m_skins.count();
    if (0 == skinCount) {
        return;
    }

    int columns = (skinCount > 3) ? 3 : skinCount;
    int rows = skinCount / columns;
    if (skinCount % columns != 0) {
        ++rows;
    }

    if (skinCount > 3) {
        m_vScrollBar = new QScrollBar(Qt::Vertical);
        m_vScrollBar->setStyleSheet(StyleHelper::styleSheetOfScrollBar());
        m_vScrollBar->setFocusPolicy(Qt::StrongFocus);
        connect(m_vScrollBar, SIGNAL(valueChanged(int)), this,
                SLOT(scrollBarValueChanged(int)));

        m_vScrollBar->setMaximum((rows - 1) * (SKIN_ITEM_HEIGHT + Y_STEP));
        m_vScrollBar->setPageStep(12 + (rows - 1) * 3);
        m_vScrollBar->setSingleStep(15 + (rows - 1) * 3);

        QGraphicsProxyWidget *scrollBarWidget = new QGraphicsProxyWidget(this);
        scrollBarWidget->setWidget(m_vScrollBar);
        scrollBarWidget->setGeometry(QRectF(391, 35, 10, 174));
    }

    int xStep = (AVAILABLE_AREA_WIDTH - columns * SKIN_ITEM_WIDTH) / (columns + 1);
    int xStartPos = LEFT_MARGIN + xStep;

    int x = xStartPos;
    int y = Y_START_POS;
    int skinItemIndex = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            m_skins[skinItemIndex]->setPos(x, y);

            ++skinItemIndex;
            if (skinItemIndex >= skinCount) {
                return;
            }

            x += (SKIN_ITEM_WIDTH + xStep);
        }

        x = xStartPos;
        y += (SKIN_ITEM_HEIGHT + Y_STEP);
    }
}

QRectF HeroSkinContainer::boundingRect() const
{
    return QRectF(QPoint(0, 0), m_backgroundPixmap.size());
}

void HeroSkinContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0, 0, m_backgroundPixmap);
}

void HeroSkinContainer::close()
{
    hide();
}

void HeroSkinContainer::skinSelected(const int skinId)
{
    setCurrentSkinId(skinId);

    close();

    swapWithSkinItemUsed(skinId);

    if (NULL != m_vScrollBar) {
        m_vScrollBar->setValue(0);
    }
}

void HeroSkinContainer::swapWithSkinItemUsed(int skinIndex)
{
    SkinItem *oldSkinItemUsed = m_skins.first();
    SkinItem *newSkinItemUsed = m_skinIndexToItem[skinIndex];
    oldSkinItemUsed->setUsed(false);
    newSkinItemUsed->setUsed(true);

    QPointF oldSkinItemUsedPos = oldSkinItemUsed->pos();
    QPointF newSkinItemUsedPos = newSkinItemUsed->pos();
    oldSkinItemUsed->setPos(newSkinItemUsedPos);
    newSkinItemUsed->setPos(oldSkinItemUsedPos);

    m_skins.swap(0, m_skins.indexOf(newSkinItemUsed));
}

int HeroSkinContainer::getCurrentSkinId() const
{
    if (Self->getActualGeneral1Name() == m_generalName)
        return Self->getHeadSkinId();
    else
        return Self->getDeputySkinId();
}

void HeroSkinContainer::setCurrentSkinId(const int skinId)
{
    if (Self->getActualGeneral1Name() == m_generalName)
        Self->setHeadSkinId(skinId);
    else
        Self->setDeputySkinId(skinId);
}

void HeroSkinContainer::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    bringToTopMost();
}

void HeroSkinContainer::bringToTopMost()
{
    if (NULL != m_currentTopMostContainer) {
        if (this == m_currentTopMostContainer) {
            return;
        }

        m_currentTopMostContainer->setZValue(m_currentTopMostContainer->m_originalZValue);
    }

    m_originalZValue = zValue();
    m_currentTopMostContainer = this;
    m_currentTopMostContainer->setZValue(UINT_MAX);
}

void HeroSkinContainer::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (NULL != m_vScrollBar) {
        int deltaValue = event->delta();
        int scrollBarValue = m_vScrollBar->value();
        scrollBarValue += (-deltaValue / 120) * m_vScrollBar->pageStep();
        m_vScrollBar->setValue(scrollBarValue);
    }
}

void HeroSkinContainer::scrollBarValueChanged(int newValue)
{
    int diff = newValue - m_oldScrollValue;
    foreach (SkinItem *skinItem, m_skins) {
        skinItem->moveBy(0, -diff);
    }

    m_oldScrollValue = newValue;
}
