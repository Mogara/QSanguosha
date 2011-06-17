#include "distanceviewdialog.h"
#include "ui_distanceviewdialog.h"

#include "client.h"
#include "clientplayer.h"
#include "engine.h"

DistanceViewDialog::DistanceViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DistanceViewDialog)
{
    ui->setupUi(this);

    players = ClientInstance->findChildren<const ClientPlayer *>();
    foreach(const ClientPlayer *player, players){
        QString name = player->getGeneralName();
        name = Sanguosha->translate(name);
        ui->fromCombobox->addItem(name);
        ui->toCombobox->addItem(name);
    }

    ui->toCombobox->setCurrentIndex(players.length()-1);

    connect(ui->fromCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(showDistance()));
    connect(ui->toCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(showDistance()));

    showDistance();
}

DistanceViewDialog::~DistanceViewDialog()
{
    delete ui;
}

void DistanceViewDialog::showDistance()
{
    const ClientPlayer *from = players.at(ui->fromCombobox->currentIndex());
    const ClientPlayer *to = players.at(ui->toCombobox->currentIndex());

    ui->fromSeatLineEdit->setText(QString::number(from->getSeat()));
    ui->toSeatLineEdit->setText(QString::number(to->getSeat()));

    int right_distance = qAbs(from->getSeat() - to->getSeat());
    int left_distance = from->aliveCount() - right_distance;

    ui->leftDistanceLineEdit->setText(QString::number(left_distance));
    ui->rightDistanceLineEdit->setText(QString::number(right_distance));
    ui->minorValueLineEdit->setText(QString::number(qMin(left_distance, right_distance)));
    ui->attackRangeLineEdit->setText(QString::number(from->getAttackRange()));

    if(from->getOffensiveHorse())
        ui->ohorseLineEdit->setText("-1");
    else
        ui->ohorseLineEdit->clear();

    if(from->hasSkill("mashu"))
        ui->mashuLineEdit->setText("-1");
    else
        ui->mashuLineEdit->clear();

    if(from->hasSkill("yicong") && from->getHp() >2)
        ui->ohorseYicongLineEdit->setText("-1");
    else
        ui->ohorseYicongLineEdit->clear();

    if(to->getDefensiveHorse())
        ui->dhorseLineEdit->setText("+1");
    else
        ui->dhorseLineEdit->clear();

    if(to->hasSkill("feiying"))
        ui->feiyingLineEdit->setText("+1");
    else
        ui->feiyingLineEdit->clear();

    if(to->hasSkill("yicong") && to->getHp()<=2)
        ui->dhorseYicongLineEdit->setText("+1");
    else
        ui->dhorseYicongLineEdit->clear();

    ui->finalResultLineEdit->setText(QString::number(from->distanceTo(to)));
    QString result = from->inMyAttackRange(to) ? tr("Yes") : tr("No");
    ui->inAttackRangeLineEdit->setText(result);
}
