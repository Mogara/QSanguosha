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
    ui->attackRangeLineEdit->setText(QString::number(from->getAttackRange()));


    if(from->hasSkill("mashu") || (from->hasSkill("yicong") && from->getHp()>2))
        ui->fromSkillSource->setText("-1");

    if(from->getOffensiveHorse())
        ui->fromEquipSource->setText("-1");

    if(from->hasSkill("feiying") || (from->hasSkill("feiying") && from->getHp()<=2))
        ui->fromSkillDest->setText("+1");

    if(from->getDefensiveHorse())
        ui->fromEquipDest->setText("+1");

    if(to->hasSkill("mashu") || (to->hasSkill("yicong") && to->getHp()>2))
        ui->toSkillSource->setText("-1");

    if(to->getOffensiveHorse())
        ui->toEquipSource->setText("-1");

    if(to->hasSkill("feiying") || (to->hasSkill("yicong") && to->getHp()<=2))
        ui->toSkillDest->setText("+1");

    if(to->getDefensiveHorse())
        ui->toEquipDest->setText("+1");

    ui->finalResultLineEdit->setText(QString::number(from->distanceTo(to)));
    QString result = from->inMyAttackRange(to) ? tr("Yes") : tr("No");
    ui->inAttackRangeLineEdit->setText(result);
}
