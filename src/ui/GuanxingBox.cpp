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

#include "GuanxingBox.h"
#include "roomscene.h"
#include "engine.h"
#include "client.h"
#include "GraphicsBox.h"

GuanxingBox::GuanxingBox()
    : CardContainer()
{
}

void GuanxingBox::doGuanxing(const QList<int> &card_ids, bool up_only) {
    if (card_ids.isEmpty()) {
        clear();
        return;
    }

    this->up_only = up_only;
    up_items.clear();
    scene_width = RoomSceneInstance->sceneRect().width();

    foreach(int card_id, card_ids) {
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setAutoBack(false);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(card_item, SIGNAL(released()), this, SLOT(adjust()));

        up_items << card_item;
        card_item->setParentItem(this);
    }

    item_count = up_items.length();
    update();
    setPos(RoomSceneInstance->tableCenterPos() - QPointF(boundingRect().width() / 2, boundingRect().height() / 2));
    show();

    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * up_items.length() - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * ((up_items.length() + 1) / 2) - blank + 50;
        one_row = false;
    }
    int first_row = one_row ? up_items.length() : (up_items.length() + 1) / 2;

    for (int i = 0; i < up_items.length(); i++) {
        CardItem *card_item = up_items.at(i);

        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45);
        }
        else {
            if (up_items.length() % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2
                + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height + blank);
        }

        card_item->resetTransform();
        card_item->setPos(25, 45);
        card_item->setHomePos(pos);
        card_item->goBack(true);
    }
}

void GuanxingBox::adjust() {
    CardItem *item = qobject_cast<CardItem *>(sender());
    if (item == NULL) return;

    const int count = up_items.length() + down_items.length();

    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int blank = 3;
    int width = (card_width + blank) * count - blank + 50;
    bool one_row = true;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * (count + 1) / 2 - blank + 50;
        one_row = false;
    }
    const int first_row = one_row ? count : (count + 1) / 2;

    up_items.removeOne(item);
    down_items.removeOne(item);

    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    const int middle_y = 45 + (one_row ? card_height : (card_height * 2 + blank));

    QList<CardItem *> *items = (up_only || item->y() + card_height / 2 <= middle_y) ? &up_items : &down_items;
    bool odd_row = true;
    if (!one_row && count % 2) {
        const qreal y = item->y() + card_height / 2;
        if ((y >= 45 + card_height && y <= 45 + card_height * 2 + blank)
            || y >= 45 + card_height * 3 + blank * 3) odd_row = false;
    }
    const int start_x = 25 + (odd_row ? 0 : (card_width / 2 + blank / 2));
    int c = (item->x() + item->boundingRect().width() / 2 - start_x) / card_width;
    c = qBound(0, c, items->length());
    items->insert(c, item);

    for (int i = 0; i < up_items.length(); i++) {
        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45);
        }
        else {
            if (count % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2
                + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height + blank);
        }
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack(true);
    }

    for (int i = 0; i < down_items.length(); i++) {
        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45 + (card_height + blank) * (one_row ? 1 : 2));
        }
        else {
            if (count % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2
                + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height * 3 + blank * 3);
        }
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack(true);
    }
}

void GuanxingBox::clear() {
    foreach(CardItem *card_item, up_items)
        delete card_item;
    foreach(CardItem *card_item, down_items)
        delete card_item;

    up_items.clear();
    down_items.clear();

    hide();
}

void GuanxingBox::reply() {
    QList<int> up_cards, down_cards;
    foreach(CardItem *card_item, up_items)
        up_cards << card_item->getCard()->getId();

    foreach(CardItem *card_item, down_items)
        down_cards << card_item->getCard()->getId();

    ClientInstance->onPlayerReplyGuanxing(up_cards, down_cards);
    clear();
}

QRectF GuanxingBox::boundingRect() const {
    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * item_count - blank + 50;
    if (width * 1.5 > (scene_width ? scene_width : 800)) {
        width = (card_width + blank) * ((item_count + 1) / 2) - blank + 50;
        one_row = false;
    }
    int height = (one_row ? 1 : 2) * card_height + (one_row ? 0 : blank);
    if (!up_only) height = height * 2 + blank;
    height += 90;

    return QRectF(0, 0, width, height);
}

void GuanxingBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    GraphicsBox::paintGraphicsBoxStyle(painter, tr("Please arrange the cards"), boundingRect());

    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * item_count - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * ((item_count + 1) / 2) - blank + 50;
        one_row = false;
    }
    int first_row = one_row ? item_count : (item_count + 1) / 2;

    for (int i = 0; i < item_count; ++i) {
        int x, y = 0;
        if (i < first_row) {
            x = 25 + (card_width + blank) * i;
            y = 45;
        }
        else {
            if (item_count % 2 == 1)
                x = 25 + card_width / 2 + blank / 2
                + (card_width + blank) * (i - first_row);
            else
                x = 25 + (card_width + blank) * (i - first_row);
            y = 45 + card_height + blank;
        }
        QRect top_rect(x, y, card_width, card_height);
        painter->drawPixmap(top_rect, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CHOOSE_GENERAL_BOX_DEST_SEAT));
        if (!up_only) {
            IQSanComponentSkin::QSanSimpleTextFont font = G_COMMON_LAYOUT.m_chooseGeneralBoxDestSeatFont;
            font.paintText(painter, top_rect, Qt::AlignCenter, tr("cards on the top of the pile"));
            QRect bottom_rect(x, y + (card_height + blank) * (one_row ? 1 : 2), card_width, card_height);
            painter->drawPixmap(bottom_rect, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CHOOSE_GENERAL_BOX_DEST_SEAT));
            font.paintText(painter, bottom_rect, Qt::AlignCenter, tr("cards at the bottom of the pile"));
        }
    }
}
