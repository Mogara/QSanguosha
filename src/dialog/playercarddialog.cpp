#include "playercarddialog.h"
#include "standard.h"
#include "engine.h"
#include "cardbutton.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QTextDocument>

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
    const General *general = player->getAvatarGeneral();
    QGroupBox *box = new QGroupBox(tr("Player info"));

    InfoRows infoRows;
    infoRows.add(tr("Avatar"), CreateImgString(general->getPixmapPath("big")))
            .add(tr("Name"), player->screenName())
            .add(tr("Gender"), player->getGenderString(), true)
            .add(tr("General"), player->getDisplayName())
            .add(tr("Kingdom"), player->getKingdom(), true)
            .add(tr("HP"), QString("%1/%2").arg(player->getHp()).arg(player->getMaxHP()));

    if(!player->getMarkDoc()->isEmpty())
        infoRows.add(tr("Marks"), player->getMarkDoc()->toHtml());

    QLabel *info = new QLabel(infoRows.toTableString());
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(info);

    box->setLayout(layout);

    return box;
}

QWidget *PlayerCardDialog::createHandcardButton(){
    QGroupBox *area = new QGroupBox(tr("Handcard area"));
    QVBoxLayout *layout =  new QVBoxLayout;
    area->setLayout(layout);

    if(player->isKongcheng()){
        CommandLinkButton *button = new CommandLinkButton(tr("This guy does not have any hand cards"));
        button->setEnabled(false);
        layout->addWidget(button);
    }else if((Self->hasSkill("dongcha") && player->hasFlag("dongchaee")) || Self == player){
        QList<const Card *> cards = player->getCards();
        foreach(const Card *card, cards){
            CardButton *button = new CardButton(card);
            layout->addWidget(button);
            connect(button, SIGNAL(idSelected(int)), this, SIGNAL(idSelected(int)));
        }
    }else{
        CardButton *button = new CardButton(NULL);
        button->setText(tr("Handcard (%1)").arg(player->getHandcardNum()));
        connect(button, SIGNAL(idSelected(int)), this, SIGNAL(idSelected(int)));
        layout->addWidget(button);
    }

    return area;
}

static QGroupBox *CreateButtonArea(PlayerCardDialog *dialog, const CardList &list, const QString &title, const QString &noCardText){
    QGroupBox *area = new QGroupBox(title);
    QVBoxLayout *layout = new QVBoxLayout;
    area->setLayout(layout);

    if(list.isEmpty()){
        CommandLinkButton *button = new CommandLinkButton;
        button->setText(noCardText);
        button->setEnabled(false);
        layout->addWidget(button);
    }else{
        foreach(const Card *card, list){
            CardButton *button = new CardButton(card);
            layout->addWidget(button);

            QObject::connect(button, SIGNAL(idSelected(int)), dialog, SIGNAL(idSelected(int)));
        }
    }

    return area;
}

QWidget *PlayerCardDialog::createEquipArea(){
    return CreateButtonArea(this, player->getEquips(), tr("Equip area"), tr("No equip"));
}

QWidget *PlayerCardDialog::createJudgingArea(){
    return CreateButtonArea(this, player->getJudgingArea(), tr("Judging area"), tr("No judging cards"));
}
