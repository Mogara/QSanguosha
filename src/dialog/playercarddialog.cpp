#include "playercarddialog.h"
#include "carditem.h"
#include "standard.h"
#include "engine.h"
#include "client.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>

PlayerCardButton::PlayerCardButton(const QString &name)
    : QCommandLinkButton(name), scale(1.0)
{
}

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

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(avatar);

    box->setLayout(layout);

    return box;
}

QWidget *PlayerCardDialog::createHandcardButton() {
    if (!player->isKongcheng() && (Self == player || handcard_visible)) {
        QGroupBox *area = new QGroupBox(tr("Handcard area"));
        QVBoxLayout *layout =  new QVBoxLayout;
        QList<const Card *> cards = player->getHandcards();
        for (int i = 0; i < cards.length(); i += 2) {
            const Card *card = Sanguosha->getEngineCard(cards.at(i)->getId());
            PlayerCardButton *button1 = new PlayerCardButton(card->getFullName());
            button1->setIcon(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));

            mapper.insert(button1, card->getId());
            connect(button1, SIGNAL(clicked()), this, SLOT(emitId()));

            PlayerCardButton *button2 = NULL;
            if (i < cards.length() - 1) {
                card = Sanguosha->getEngineCard(cards.at(i + 1)->getId());;
                button2 = new PlayerCardButton(card->getFullName());
                button2->setIcon(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));

                mapper.insert(button2, card->getId());
                connect(button2, SIGNAL(clicked()), this, SLOT(emitId()));
            }
            if (button1 && button2) {
                QHBoxLayout *hlayout = new QHBoxLayout;
                button1->setScale(0.65);
                button2->setScale(0.65);
                hlayout->addWidget(button1);
                hlayout->addWidget(button2);
                layout->addLayout(hlayout);
            } else {
                Q_ASSERT(button1 != NULL);
                layout->addWidget(button1);
            }
        }

        area->setLayout(layout);
        return area;
    }

    PlayerCardButton *button = new PlayerCardButton(tr("Handcard"));
    button->setObjectName("handcard_button");
    int num = player->getHandcardNum();
    if (num == 0) {
        button->setDescription(tr("This guy has no any hand cards"));
        button->setEnabled(false);
    } else {
        button->setDescription(tr("This guy has %1 hand card(s)").arg(num));
        button->setEnabled(method != Card::MethodDiscard || Self->canDiscard(player, "h"));
        mapper.insert(button, -1);
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
    }

    return button;
}

QWidget *PlayerCardDialog::createEquipArea() {
    QGroupBox *area = new QGroupBox(tr("Equip area"));
    QVBoxLayout *layout = new QVBoxLayout;

    WrappedCard *weapon = player->getWeapon();
    if (weapon) {
        PlayerCardButton *button = new PlayerCardButton(weapon->getFullName());
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(Sanguosha->getEngineCard(weapon->getId())->getSuit()));
        button->setEnabled(!disabled_ids.contains(weapon->getEffectiveId())
                            && (method != Card::MethodDiscard || Self->canDiscard(player, weapon->getEffectiveId())));
        mapper.insert(button, weapon->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    WrappedCard *armor = player->getArmor();
    if (armor) {
        PlayerCardButton *button = new PlayerCardButton(armor->getFullName());
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(Sanguosha->getEngineCard(armor->getId())->getSuit()));
        button->setEnabled(!disabled_ids.contains(armor->getEffectiveId())
                            && (method != Card::MethodDiscard || Self->canDiscard(player, armor->getEffectiveId())));
        mapper.insert(button, armor->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    WrappedCard *horse = player->getDefensiveHorse();
    if (horse) {
        PlayerCardButton *button = new PlayerCardButton(horse->getFullName() + tr("(+1 horse)"));
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(Sanguosha->getEngineCard(horse->getId())->getSuit()));
        button->setEnabled(!disabled_ids.contains(horse->getEffectiveId())
                            && (method != Card::MethodDiscard || Self->canDiscard(player, horse->getEffectiveId())));
        mapper.insert(button, horse->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    horse = player->getOffensiveHorse();
    if (horse) {
        PlayerCardButton *button = new PlayerCardButton(horse->getFullName() + tr("(-1 horse)"));
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(Sanguosha->getEngineCard(horse->getId())->getSuit()));
        button->setEnabled(!disabled_ids.contains(horse->getEffectiveId())
                            && (method != Card::MethodDiscard || Self->canDiscard(player, horse->getEffectiveId())));
        mapper.insert(button, horse->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    if (layout->count() == 0) {
        PlayerCardButton *no_equip = new PlayerCardButton(tr("No equip"));
        no_equip->setEnabled(false);
        no_equip->setObjectName("noequip_button");
        return no_equip;
    } else {
        area->setLayout(layout);
        return area;
    }
}

QWidget *PlayerCardDialog::createJudgingArea() {
    QGroupBox *area = new QGroupBox(tr("Judging Area"));
    QVBoxLayout *layout = new QVBoxLayout;
    QList<const Card *> cards = player->getJudgingArea();
    foreach (const Card *card, cards) {
        const Card *real = Sanguosha->getEngineCard(card->getId());
        PlayerCardButton *button = new PlayerCardButton(real->getFullName());
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(real->getSuit()));
        layout->addWidget(button);
        button->setEnabled(!disabled_ids.contains(card->getEffectiveId())
                            && (method != Card::MethodDiscard || Self->canDiscard(player, card->getEffectiveId())));
        mapper.insert(button, card->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
    }

    if (layout->count() == 0) {
        PlayerCardButton *button = new PlayerCardButton(tr("No judging cards"));
        button->setEnabled(false);
        button->setObjectName("nojuding_button");
        return button;
    } else {
        area->setLayout(layout);
        return area;
    }
}

void PlayerCardDialog::emitId() {
    int id = mapper.value(sender(), -2);
    if (id != -2)
        emit card_id_chosen(id);
}

