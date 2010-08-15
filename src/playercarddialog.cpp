#include "playercarddialog.h"
#include "standard.h"
#include "engine.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QGroupBox>

PlayerCardDialog::PlayerCardDialog(const ClientPlayer *player, const QString &flags)
    :player(player), mapper(new QSignalMapper(this))
{
    QVBoxLayout *layout = new QVBoxLayout;

    static QChar handcard_flag('h');
    static QChar equip_flag('e');
    static QChar judging_flag('j');

    if(flags.contains(handcard_flag))
        layout->addWidget(createHandcardButton());

    if(flags.contains(equip_flag))
        layout->addWidget(createEquipArea());

    if(flags.contains(judging_flag))
        layout->addWidget(createJudgingArea());

    connect(mapper, SIGNAL(mapped(int)), this, SIGNAL(card_id_chosen(int)));

    setLayout(layout);
}

QWidget *PlayerCardDialog::createHandcardButton(){
    QCommandLinkButton *button = new QCommandLinkButton(tr("Handcard"));
    int num = player->getHandcardNum();
    if(num == 0){
        button->setDescription(tr("This guy has no any hand cards"));
        button->setEnabled(false);
    }else{
        button->setDescription(tr("This guy has %1 hand card(s)").arg(num));

        mapper->setMapping(button, -1);
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
    }

    return button;
}

QWidget *PlayerCardDialog::createEquipArea(){
    QGroupBox *area = new QGroupBox(tr("Equip area"));
    QVBoxLayout *layout = new QVBoxLayout();

    const Weapon *weapon = player->getWeapon();
    if(weapon){
        QCommandLinkButton *button = new QCommandLinkButton(weapon->getFullName());
        button->setIcon(weapon->getSuitIcon());

        mapper->setMapping(button, weapon->getID());
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        layout->addWidget(button);
    }

    const Armor *armor = player->getArmor();
    if(armor){
        QCommandLinkButton *button = new QCommandLinkButton(armor->getFullName());
        button->setIcon(armor->getSuitIcon());

        mapper->setMapping(button, armor->getID());
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        layout->addWidget(button);
    }

    const Horse *horse = player->getDefensiveHorse();
    if(horse){
        QCommandLinkButton *button = new QCommandLinkButton(horse->getFullName());
        button->setIcon(horse->getSuitIcon());

        mapper->setMapping(button, horse->getID());
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        layout->addWidget(button);
    }

    horse = player->getOffensiveHorse();
    if(horse){
        QCommandLinkButton *button = new QCommandLinkButton(horse->getFullName());
        button->setIcon(horse->getSuitIcon());

        mapper->setMapping(button, horse->getID());
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        layout->addWidget(button);
    }

    if(layout->count() == 0){
        QCommandLinkButton *no_equip = new QCommandLinkButton(tr("No equip"));
        no_equip->setEnabled(false);
        return no_equip;
    }else{
        area->setLayout(layout);
        return area;
    }
}

QWidget *PlayerCardDialog::createJudgingArea(){
    QGroupBox *area = new QGroupBox(tr("Judging Area"));
    QVBoxLayout *layout = new QVBoxLayout;
    QStack<const Card *> cards = player->getJudgingArea();
    QVectorIterator<const Card *> itor(cards);
    while(itor.hasNext()){
        const Card *card = itor.next();
        QCommandLinkButton *button = new QCommandLinkButton(card->getFullName());
        button->setIcon(card->getSuitIcon());

        mapper->setMapping(button, card->getID());
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
    }

    if(layout->count() == 0){
        QCommandLinkButton *button = new QCommandLinkButton(tr("No judging cards"));
        button->setEnabled(false);
        return button;
    }else{
        area->setLayout(layout);
        return area;
    }
}
