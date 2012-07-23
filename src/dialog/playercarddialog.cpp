#include "playercarddialog.h"
#include "carditem.h"
#include "standard.h"
#include "engine.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>

PlayerCardDialog::PlayerCardDialog(const ClientPlayer *player, const QString &flags)
    :player(player)
{
    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *layout = new QHBoxLayout;

    static QChar handcard_flag('h');
    static QChar equip_flag('e');
    static QChar judging_flag('j');

    layout->addWidget(createAvatar());

    if(flags.contains(handcard_flag))
        vlayout->addWidget(createHandcardButton());

    if(flags.contains(equip_flag))
        vlayout->addWidget(createEquipArea());

    if(flags.contains(judging_flag))
        vlayout->addWidget(createJudgingArea());

    layout->addLayout(vlayout);

    setLayout(layout);
}

QWidget *PlayerCardDialog::createAvatar(){
    QGroupBox *box = new QGroupBox(player->screenName());

    QLabel *avatar = new QLabel(box);
    avatar->setPixmap(QPixmap(G_ROOM_SKIN.getGeneralPixmap(player->getGeneralName(), QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(avatar);
    // @todo: add magatamas box here!
    // layout->addWidget(new QGraphicsProxyWidget(magatamas));

    box->setLayout(layout);

    return box;
}

QWidget *PlayerCardDialog::createHandcardButton(){
    if(!player->isKongcheng() && ((Self->hasSkill("dongcha") && player->hasFlag("dongchaee")) || Self == player)){
        QGroupBox *area = new QGroupBox(tr("Handcard area"));
        QVBoxLayout *layout =  new QVBoxLayout;
        QList<const Card *> cards = player->getCards();
        foreach(const Card *card, cards){
            QCommandLinkButton *button = new QCommandLinkButton(card->getFullName());
            button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));

            mapper.insert(button, card->getId());
            connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
            layout->addWidget(button);
        }

        area->setLayout(layout);
        return area;
    }

    QCommandLinkButton *button = new QCommandLinkButton(tr("Handcard"));
    button->setObjectName("handcard_button");
    int num = player->getHandcardNum();
    if(num == 0){
        button->setDescription(tr("This guy has no any hand cards"));
        button->setEnabled(false);
    }else{
        button->setDescription(tr("This guy has %1 hand card(s)").arg(num));

        mapper.insert(button, -1);
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
    }

    return button;
}

QWidget *PlayerCardDialog::createEquipArea(){
    QGroupBox *area = new QGroupBox(tr("Equip area"));
    QVBoxLayout *layout = new QVBoxLayout;

    WrappedCard *weapon = player->getWeapon();
    if(weapon){
        QCommandLinkButton *button = new QCommandLinkButton(weapon->getFullName());
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(weapon->getSuit()));

        mapper.insert(button, weapon->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    WrappedCard *armor = player->getArmor();
    if(armor){
        QCommandLinkButton *button = new QCommandLinkButton(armor->getFullName());
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(armor->getSuit()));

        mapper.insert(button, armor->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    WrappedCard *horse = player->getDefensiveHorse();
    if(horse){
        QCommandLinkButton *button = new QCommandLinkButton(horse->getFullName() + tr("(+1 horse)"));
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(horse->getSuit()));

        mapper.insert(button, horse->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    horse = player->getOffensiveHorse();
    if(horse){
        QCommandLinkButton *button = new QCommandLinkButton(horse->getFullName() + tr("(-1 horse)"));
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(horse->getSuit()));

        mapper.insert(button, horse->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
        layout->addWidget(button);
    }

    if(layout->count() == 0){
        QCommandLinkButton *no_equip = new QCommandLinkButton(tr("No equip"));
        no_equip->setEnabled(false);
        no_equip->setObjectName("noequip_button");
        return no_equip;
    }else{
        area->setLayout(layout);
        return area;
    }
}

QWidget *PlayerCardDialog::createJudgingArea(){
    QGroupBox *area = new QGroupBox(tr("Judging Area"));
    QVBoxLayout *layout = new QVBoxLayout;
    QList<const Card *> cards = player->getJudgingArea();
    foreach(const Card *card, cards){
        const Card* real = Sanguosha->getEngineCard(card->getId());
        QCommandLinkButton *button = new QCommandLinkButton(real->getFullName());
        button->setIcon(G_ROOM_SKIN.getCardSuitPixmap(real->getSuit()));
        layout->addWidget(button);

        mapper.insert(button, card->getId());
        connect(button, SIGNAL(clicked()), this, SLOT(emitId()));
    }

    if(layout->count() == 0){
        QCommandLinkButton *button = new QCommandLinkButton(tr("No judging cards"));
        button->setEnabled(false);
        button->setObjectName("nojuding_button");
        return button;
    }else{
        area->setLayout(layout);
        return area;
    }
}

void PlayerCardDialog::emitId(){
    int id = mapper.value(sender(), -2);
    if(id != -2)
        emit card_id_chosen(id);
}
