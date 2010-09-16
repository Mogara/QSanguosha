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

    Player::CorrectStruct from_correct = from->getCorrectStruct();
    ui->fromSkillSource->setText(QString::number(from_correct.skill_src));
    ui->fromEquipSource->setText(QString::number(from_correct.equip_src));
    ui->fromSkillDest->setText(QString::number(from_correct.skill_dest));
    ui->fromEquipDest->setText(QString::number(from_correct.equip_dest));

    Player::CorrectStruct to_correct = to->getCorrectStruct();
    ui->toSkillSource->setText(QString::number(to_correct.skill_src));
    ui->toEquipSource->setText(QString::number(to_correct.equip_src));
    ui->toSkillDest->setText(QString::number(to_correct.skill_dest));
    ui->toEquipDest->setText(QString::number(to_correct.equip_dest));

    ui->finalResultLineEdit->setText(QString::number(from->distanceTo(to)));
    QString result = from->inMyAttackRange(to) ? tr("Yes") : tr("No");
    ui->inAttackRangeLineEdit->setText(result);
}
