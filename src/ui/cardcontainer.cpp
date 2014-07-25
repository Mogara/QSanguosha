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

#include "cardcontainer.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "client.h"
#include "roomscene.h"
#include "button.h"
#include "GraphicsBox.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

CardContainer::CardContainer()
    : confirm_button(new Button(tr("confirm"), 0.6, true)),
      scene_width(0), item_count(0)
{
    confirm_button->setParentItem(this);
    confirm_button->hide();
    connect(confirm_button, SIGNAL(clicked()), this, SLOT(clear()));

    GraphicsBox::stylize(this);
}

void CardContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    GraphicsBox::paintGraphicsBoxStyle(painter, tr("QSanguosha-Hegemony"), boundingRect());

    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    const int blank = 3;
    bool one_row = true;
    int width = (card_width + blank) * items.length() - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * ((items.length() + 1) / 2) - blank + 50;
        one_row = false;
    }

    int first_row = one_row ? items.length() : (items.length() + 1) / 2;

    for (int i = 0; i < items.length(); ++i) {
        int x, y = 0;
        if (i < first_row) {
            x = 25 + (card_width + blank) * i;
            y = 45;
        }
        else {
            if (items.length() % 2 == 1)
                x = 25 + card_width / 2 + blank / 2
                + (card_width + blank) * (i - first_row);
            else
                x = 25 + (card_width + blank) * (i - first_row);
            y = 45 + card_height + blank;
        }
        painter->drawPixmap(x, y, card_width, card_height, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CHOOSE_GENERAL_BOX_DEST_SEAT));
    }
}

QRectF CardContainer::boundingRect() const{
    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * item_count - blank + 50;
    if (width * 1.5 > (scene_width ? scene_width : 800)) {
        width = (card_width + blank) * ((item_count + 1) / 2) - blank + 50;
        one_row = false;
    }
    int height = (one_row ? 1 : 2) * card_height + 90 + (one_row ? 0 : blank);

    return QRectF(0, 0, width, height);
}

void CardContainer::fillCards(const QList<int> &card_ids, const QList<int> &disabled_ids) {
    QList<CardItem *> card_items;
    if (card_ids.isEmpty() && items.isEmpty())
        return;
    else if (card_ids.isEmpty() && !items.isEmpty()) {
        card_items = items;
        items.clear();
    }
    else if (!items.isEmpty()) {
        retained_stack.push(retained());
        items_stack.push(items);
        foreach(CardItem *item, items)
            item->hide();
        items.clear();
    }

    scene_width = RoomSceneInstance->sceneRect().width();

    confirm_button->hide();
    if (card_items.isEmpty())
        card_items = _createCards(card_ids);

    items.append(card_items);
    item_count = items.length();
    update();

    const int blank = 3;
    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    int width = (card_width + blank) * item_count - blank + 50;
    if (width * 1.5 > scene_width) {
        width = (card_width + blank) * ((item_count + 1) / 2) - blank + 50;
        one_row = false;
    }
    int first_row = one_row ? item_count : (item_count + 1) / 2;

    for (int i = 0; i < item_count; i++) {
        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45);
        }
        else {
            if (item_count % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2
                + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height + blank);
        }
        CardItem *item = items[i];
        item->resetTransform();
        item->setPos(pos);
        item->setHomePos(pos);
        item->setOpacity(1.0);
        item->setHomeOpacity(1.0);
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        if (disabled_ids.contains(item->getCard()->getEffectiveId()))
            item->setEnabled(false);
        item->setOuterGlowEffectEnabled(true);
        item->show();
    }
    confirm_button->setPos(boundingRect().center().x() - confirm_button->boundingRect().width() / 2, boundingRect().height() - 40);
}

bool CardContainer::_addCardItems(QList<CardItem *> &, const CardsMoveStruct &) {
    return true;
}

bool CardContainer::retained() {
    return confirm_button != NULL && confirm_button->isVisible();
}

void CardContainer::clear() {
    foreach(CardItem *item, items) {
        item->hide();
        delete item;
        item = NULL;
    }

    items.clear();
    if (!items_stack.isEmpty()) {
        items = items_stack.pop();
        bool retained = retained_stack.pop();
        fillCards();
        if (retained && confirm_button)
            confirm_button->show();
    }
    else {
        confirm_button->hide();
        hide();
    }
}

void CardContainer::freezeCards(bool is_frozen) {
    foreach(CardItem *item, items)
        item->setFrozen(is_frozen);
}

QList<CardItem *> CardContainer::removeCardItems(const QList<int> &card_ids, Player::Place) {
    QList<CardItem *> result;
    foreach(int card_id, card_ids) {
        CardItem *to_take = NULL;
        foreach(CardItem *item, items) {
            if (item->getCard()->getId() == card_id) {
                to_take = item;
                break;
            }
        }
        if (to_take == NULL) continue;

        to_take->setEnabled(false);

        CardItem *copy = new CardItem(to_take->getCard());
        copy->setPos(mapToScene(to_take->pos()));
        copy->setEnabled(false);
        result.append(copy);

        if (m_currentPlayer)
            to_take->showAvatar(m_currentPlayer->getGeneral());
    }
    return result;
}

int CardContainer::getFirstEnabled() const{
    foreach(CardItem *card, items) {
        if (card->isEnabled())
            return card->getCard()->getId();
    }
    return -1;
}

void CardContainer::startChoose() {
    confirm_button->hide();
    foreach(CardItem *item, items) {
        connect(item, SIGNAL(leave_hover()), this, SLOT(grabItem()));
        connect(item, SIGNAL(clicked()), this, SLOT(chooseItem()));
    }
}

void CardContainer::startGongxin(const QList<int> &enabled_ids) {
    if (enabled_ids.isEmpty()) return;
    foreach(CardItem *item, items) {
        const Card *card = item->getCard();
        if (card && enabled_ids.contains(card->getEffectiveId()))
            connect(item, SIGNAL(double_clicked()), this, SLOT(gongxinItem()));
        else
            item->setEnabled(false);
    }
}

void CardContainer::addConfirmButton() {
    foreach (CardItem *card, items)
        card->setFlag(ItemIsMovable, false);

    confirm_button->show();
}

void CardContainer::grabItem() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item && !collidesWithItem(card_item)) {
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::chooseItem() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::gongxinItem() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        emit item_gongxined(card_item->getCard()->getId());
        clear();
    }
}

void CardContainer::view(const ClientPlayer *player) {
    QList<int> card_ids;
    QList<const Card *> cards = player->getHandcards();
    foreach(const Card *card, cards)
        card_ids << card->getEffectiveId();

    fillCards(card_ids);
}
