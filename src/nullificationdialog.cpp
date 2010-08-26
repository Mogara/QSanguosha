#include "nullificationdialog.h"
#include "client.h"
#include "engine.h"

#include <QToolButton>
#include <QCommandLinkButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimerEvent>

NullificationDialog::NullificationDialog(const QString &trick_name, ClientPlayer *source, ClientPlayer *target, const QList<int> &card_ids){
    QLabel *label = new QLabel(tr("Do you want to use [nullification] ?"));

    progress_bar = new QProgressBar;    

    const Card *trick_card = Sanguosha->findChild<const Card *>(trick_name);
    QToolButton *trick_button = new QToolButton;
    trick_button->setIcon(QIcon(trick_card->getPixmapPath()));
    trick_button->setIconSize(QSize(100, 140));
    trick_button->setText(Sanguosha->translate(trick_name));
    trick_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    static QSize avatar_size(80, 80);

    QToolButton *source_button = new QToolButton;
    const General *source_general = source->getGeneral();
    QString source_general_name = Sanguosha->translate(source_general->objectName());
    source_button->setIcon(QIcon(source_general->getPixmapPath("big")));
    source_button->setIconSize(avatar_size);
    source_button->setText(tr("User\n%1[%2]").arg(source->objectName()).arg(source_general_name));
    source_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton *target_button = new QToolButton;
    const General *target_general = target->getGeneral();
    QString target_general_name = Sanguosha->translate(target_general->objectName());
    target_button->setIcon(QIcon(target_general->getPixmapPath("big")));
    target_button->setIconSize(avatar_size);
    target_button->setText(tr("Target\n%1[%2]").arg(target->objectName()).arg(target_general_name));
    target_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(trick_button);
    hlayout->addWidget(source_button);
    hlayout->addWidget(target_button);

    QVBoxLayout *vlayout = new QVBoxLayout;
    button_group = new QButtonGroup;
    foreach(int card_id, card_ids){
        QCommandLinkButton *button = new QCommandLinkButton;
        const Card *card = Sanguosha->getCard(card_id);
        button->setIcon(card->getSuitIcon());
        button->setText(tr("[%1]").arg(Sanguosha->translate(card->objectName())));
        QString suit_name = Sanguosha->translate(card->getSuitString());
        button->setDescription(tr("Suit: %1, Number: %2").arg(suit_name).arg(card->getNumberString()));

        button_group->addButton(button, card_id);
        vlayout->addWidget(button);

        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
    }

    QCommandLinkButton *cancel_button = new QCommandLinkButton;
    cancel_button->setText(tr("Cancel"));
    cancel_button->setDescription(tr("The trick card won't be nullified"));
    button_group->addButton(cancel_button, -1);
    vlayout->addWidget(cancel_button);
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(this, SIGNAL(rejected()), this, SLOT(onReject()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(progress_bar);
    layout->addLayout(hlayout);
    layout->addLayout(vlayout);

    setLayout(layout);

    startTimer(200); // 1/5 seconds
}

void NullificationDialog::timerEvent(QTimerEvent *event){
    int new_value = progress_bar->value() + 4;
    if(new_value >= progress_bar->maximum()){
        killTimer(event->timerId());
        if(isVisible())
            reject();
    }else
        progress_bar->setValue(new_value);
}

void NullificationDialog::reply(){
    accept();

    int card_id = button_group->checkedId();
    ClientInstance->replyNullification(card_id);
}

void NullificationDialog::onReject(){
    ClientInstance->replyNullification(-1);
}
