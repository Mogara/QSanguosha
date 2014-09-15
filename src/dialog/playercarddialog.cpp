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

#include "playercarddialog.h"
#include "carditem.h"
#include "standard.h"
#include "engine.h"
#include "client.h"
#include "cardbutton.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>

PlayerCardDialog::PlayerCardDialog(const ClientPlayer *player, const QString &flags,
    bool handcard_visible, Card::HandlingMethod method, const QList<int> &disabled_ids)
    : player(player), handcard_visible(handcard_visible), method(method), disabled_ids(disabled_ids)
{
    QVBoxLayout *vlayout1 = new QVBoxLayout, *vlayout2 = new QVBoxLayout;
    QHBoxLayout *layout = new QHBoxLayout;

    static QChar handcard_flag('h');
    static QChar equip_flag('e');
    static QChar judging_flag('j');

    vlayout1->addWidget(createAvatar());
    vlayout1->addStretch();

    if (flags.contains(handcard_flag))
        vlayout2->addWidget(createHandcardButton());

    if (flags.contains(equip_flag))
        vlayout2->addWidget(createEquipArea());

    if (flags.contains(judging_flag))
        vlayout2->addWidget(createJudgingArea());

    layout->addLayout(vlayout1);
    layout->addLayout(vlayout2);
    setLayout(layout);
}

QWidget *PlayerCardDialog::createAvatar() {
    QGroupBox *box = new QGroupBox(ClientInstance->getPlayerName(player->objectName()));
    box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel *avatar = new QLabel(box);
    avatar->setPixmap(QPixmap(G_ROOM_SKIN.getGeneralPixmap(player->getGeneralName(), QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE)));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(avatar);

    if (player->getGeneral2() != NULL) {
        QLabel *avatar2 = new QLabel(box);
        avatar2->setPixmap(QPixmap(G_ROOM_SKIN.getGeneralPixmap(player->getGeneral2Name(), QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE)));
        layout->addWidget(avatar2);
    }

    box->setLayout(layout);

    return box;
}

QWidget *PlayerCardDialog::createHandcardButton() {
    if (!player->isKongcheng() && (Self == player || handcard_visible)) {
        QGroupBox *area = new QGroupBox(tr("Handcard area"));
        QVBoxLayout *layout = new QVBoxLayout;
        QList<const Card *> cards = player->getHandcards();
        for (int i = 0; i < cards.length(); i += 2) {
            const Card *card = Sanguosha->getEngineCard(cards.at(i)->getId());
            CardButton *button1 = new CardButton(card);
            button1->setIcon(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));
            connect(button1, SIGNAL(idSelected(int)), this, SLOT(idSelected(int)));

            CardButton *button2 = NULL;
            if (i < cards.length() - 1) {
                card = Sanguosha->getEngineCard(cards.at(i + 1)->getId());
                button2 = new CardButton(card);
                button2->setIcon(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));

                connect(button1, SIGNAL(idSelected(int)), this, SLOT(idSelected(int)));
            }
            if (button1 && button2) {
                QHBoxLayout *hlayout = new QHBoxLayout;
                button1->setScale(0.65);
                button2->setScale(0.65);
                hlayout->addWidget(button1);
                hlayout->addWidget(button2);
                layout->addLayout(hlayout);
            }
            else {
                Q_ASSERT(button1 != NULL);
                layout->addWidget(button1);
            }
        }

        area->setLayout(layout);
        return area;
    }

    CardButton *button = new CardButton(NULL);
    button->setText(tr("Handcard"));
    button->setObjectName("handcard_button");
    int num = player->getHandcardNum();
    if (num == 0) {
        button->setDescription(tr("This guy has no any hand cards"));
        button->setEnabled(false);
    }
    else {
        button->setDescription(tr("This guy has %1 hand card(s)").arg(num));
        button->setEnabled(method != Card::MethodDiscard || Self->canDiscard(player, "h"));
        connect(button, SIGNAL(idSelected(int)), this, SIGNAL(idSelected(int)));
    }

    return button;
}

QGroupBox *PlayerCardDialog::createButtonArea(const CardList &list, const QString &title, const QString &noCardText) {
    QGroupBox *area = new QGroupBox(title);
    QVBoxLayout *layout = new QVBoxLayout;
    area->setLayout(layout);

    if (list.isEmpty()) {
        CardButton *button = new CardButton(NULL);
        button->setText(noCardText);
        button->setEnabled(false);
        layout->addWidget(button);
    }
    else {
        foreach(const Card *card, list) {
            CardButton *button = new CardButton(card);
            layout->addWidget(button);
            button->setEnabled(!disabled_ids.contains(card->getEffectiveId())
                && (method != Card::MethodDiscard || Self->canDiscard(player, card->getEffectiveId())));
            QObject::connect(button, SIGNAL(idSelected(int)), this, SIGNAL(idSelected(int)));
        }
    }

    return area;
}

QWidget *PlayerCardDialog::createEquipArea() {
    return createButtonArea(player->getEquips(), tr("Equip area"), tr("No equip"));
}

QWidget *PlayerCardDialog::createJudgingArea() {
    return createButtonArea(player->getJudgingArea(), tr("Judging area"), tr("No judging cards"));
}

