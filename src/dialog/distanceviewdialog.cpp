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

#include "distanceviewdialog.h"

#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "roomscene.h"

#include <QFormLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>

class DistanceViewDialogUI {
public:
    DistanceViewDialogUI() {
        from = new QComboBox;
        to = new QComboBox;

        left = new QLineEdit;
        right = new QLineEdit;

        min = new QLineEdit;
        in_attack = new QLineEdit;

        left->setReadOnly(true);
        right->setReadOnly(true);
        min->setReadOnly(true);
        in_attack->setReadOnly(true);

        QList<const DistanceSkill *> skills = Sanguosha->getDistanceSkills();
        QLineEdit *horse_edit = new QLineEdit;
        horse_edit->setObjectName("HorseCorrect");
        horse_edit->setReadOnly(true);
        distance_edits << horse_edit;
        foreach(const DistanceSkill *skill, skills) {
            bool show_skill = false;
            foreach(const ClientPlayer *p, ClientInstance->getPlayers()) {
                if (p->hasShownSkill(skill)) {
                    show_skill = true;
                    break;
                }
            }
            if (!show_skill) continue;

            QLineEdit *distance_edit = new QLineEdit;
            distance_edit->setObjectName(skill->objectName());
            distance_edit->setReadOnly(true);

            distance_edits << distance_edit;
        }

        final = new QLineEdit;
        final->setReadOnly(true);
    }

    QComboBox *from, *to;
    QLineEdit *left, *right;
    QLineEdit *min;
    QList<QLineEdit *> distance_edits;
    QLineEdit *in_attack;
    QLineEdit *final;
};

DistanceViewDialog::DistanceViewDialog(QWidget *parent)
    : FlatDialog(parent)
{
    setWindowTitle(tr("Distance view"));

    QFormLayout *fLayout = new QFormLayout;

    ui = new DistanceViewDialogUI;

    RoomScene::FillPlayerNames(ui->from, false);
    RoomScene::FillPlayerNames(ui->to, false);

    connect(ui->from, (void (QComboBox::*)(int))(&QComboBox::currentIndexChanged), this, &DistanceViewDialog::showDistance);
    connect(ui->to, (void (QComboBox::*)(int))(&QComboBox::currentIndexChanged), this, &DistanceViewDialog::showDistance);

    fLayout->addRow(tr("From"), ui->from);
    fLayout->addRow(tr("To"), ui->to);
    fLayout->addRow(tr("Left"), ui->left);
    fLayout->addRow(tr("Right"), ui->right);
    fLayout->addRow(tr("Minimum"), ui->min);

    QGroupBox *box = new QGroupBox;
    fLayout->addRow(tr("Distance correct"), box);

    QFormLayout *box_layout = new QFormLayout;
    foreach(QLineEdit *edit, ui->distance_edits)
        box_layout->addRow(Sanguosha->translate(edit->objectName()), edit);

    box->setLayout(box_layout);

    fLayout->addRow(tr("In attack range"), ui->in_attack);
    fLayout->addRow(tr("Final"), ui->final);

    layout->addLayout(fLayout);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &DistanceViewDialog::reject);
    layout->addWidget(closeButton);

    showDistance();
}

DistanceViewDialog::~DistanceViewDialog() {
    delete ui;
}

void DistanceViewDialog::showDistance() {
    QString from_name = ui->from->itemData(ui->from->currentIndex()).toString();
    QString to_name = ui->to->itemData(ui->to->currentIndex()).toString();

    const ClientPlayer *from = ClientInstance->getPlayer(from_name);
    const ClientPlayer *to = ClientInstance->getPlayer(to_name);

    if (from->isRemoved() || to->isRemoved()) {
        ui->right->setText(tr("Not exist"));
        ui->left->setText(tr("Not exist"));
        ui->min->setText(tr("Not exist"));
    } else {
        int right_distance = from->originalRightDistanceTo(to);
        ui->right->setText(QString::number(right_distance));

        int left_distance = from->aliveCount(false) - right_distance;
        ui->left->setText(QString::number(left_distance));

        int min = qMin(left_distance, right_distance);
        ui->min->setText(QString("min(%1, %2)=%3")
            .arg(left_distance)
            .arg(right_distance)
            .arg(min));
    }

    foreach(QLineEdit *edit, ui->distance_edits) {
        QString skill_name = edit->objectName();
        if (skill_name == "HorseCorrect")
            skill_name = "Horse";
        const Skill *skill = Sanguosha->getSkill(skill_name);
        const DistanceSkill *distance_skill = qobject_cast<const DistanceSkill *>(skill);
        int correct = distance_skill->getCorrect(from, to);

        if (correct > 0)
            edit->setText(QString("+%1").arg(correct));
        else if (correct < 0)
            edit->setText(QString::number(correct));
        else
            edit->setText(QString());
    }

    ui->in_attack->setText(from->inMyAttackRange(to) ? tr("Yes") : tr("No"));

    if (from->isRemoved() || to->isRemoved())
        ui->final->setText(tr("Not exist"));
    else
        ui->final->setText(QString::number(from->distanceTo(to)));
}

