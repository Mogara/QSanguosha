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

#include "freechoosedialog.h"
#include "general.h"
#include "engine.h"
#include "skinbank.h"

#include <QGridLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QButtonGroup>

FreeChooseDialog::FreeChooseDialog(QWidget *parent, ButtonGroupType type)
    : FlatDialog(parent), type(type)
{
    setWindowTitle(tr("Free choose generals"));

    QTabWidget *tab_widget = new QTabWidget;

    group = new QButtonGroup(this);
    group->setExclusive(type == Exclusive);

    QList<const General *> all_generals = Sanguosha->getGeneralList();
    QMap<QString, QList<const General *> > map;
    foreach(const General *general, all_generals) {
        if (general->isTotallyHidden())
            continue;

        if (general->isLord())
            continue;

        if (general->getPackage() == "jiange-defense")
            continue;

        map[general->getKingdom()] << general;
    }

    QStringList kingdoms = Sanguosha->getKingdoms();

    foreach(QString kingdom, kingdoms) {
        QList<const General *> generals = map[kingdom];

        if (!generals.isEmpty()) {
            QWidget *tab = createTab(generals);
            tab_widget->addTab(tab,
                QIcon(G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_ICON, kingdom)),
                Sanguosha->translate(kingdom));
        }
    }

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, &QPushButton::clicked, this, &FreeChooseDialog::chooseGeneral);

    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    connect(cancel_button, &QPushButton::clicked, this, &FreeChooseDialog::reject);

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    layout->addWidget(tab_widget);
    layout->addLayout(button_layout);

    if (type == Exclusive)
        group->buttons().first()->click();
}

void FreeChooseDialog::chooseGeneral()
{
    if (type == Pair) {
        QList<QAbstractButton *> buttons = group->buttons();
        QString first, second;
        foreach(QAbstractButton *button, buttons) {
            if (!button->isChecked())
                continue;

            if (first.isEmpty()) {
                first = button->objectName();
            } else {
                second = button->objectName();
                emit pair_chosen(first, second);
                break;
            }
        }
        if (second.isEmpty()){
            QMessageBox::information(this, tr("Information"), tr("You can only select 2 generals in Pairs mode."));
            return;
        }
    } else if (type == Multi) {
        QStringList general_names;
        foreach(QAbstractButton *button, group->buttons()) {
            if (button->isChecked())
                general_names << button->objectName();
        }
        if (!general_names.isEmpty())
            emit general_chosen(general_names.join("+"));
    } else {
        QAbstractButton *button = group->checkedButton();
        if (button)
            emit general_chosen(button->objectName());
    }

    accept();
}

QWidget *FreeChooseDialog::createTab(const QList<const General *> &generals)
{
    QWidget *tab = new QWidget;

    QGridLayout *layout = new QGridLayout;
    layout->setOriginCorner(Qt::TopLeftCorner);
    QIcon lord_icon("image/system/roles/lord.png");

    const int columns = 4;

    for (int i = 0; i < generals.length(); ++i) {
        const General *general = generals.at(i);
        QString general_name = general->objectName();
        QString text = QString("%1[%2]")
            .arg(Sanguosha->translate(general_name))
            .arg(Sanguosha->translate(general->getPackage()));

        QAbstractButton *button;
        if (type == Exclusive)
            button = new QRadioButton(text);
        else
            button = new QCheckBox(text);

        button->setObjectName(general_name);
        button->setToolTip(general->getSkillDescription(true));
        if (general->isLord())
            button->setIcon(lord_icon);

        group->addButton(button);

        int row = i / columns;
        int column = i % columns;
        layout->addWidget(button, row, column);
    }

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->addStretch();

    QVBoxLayout *tablayout = new QVBoxLayout;
    tablayout->addLayout(layout);
    tablayout->addLayout(layout2);

    tab->setLayout(tablayout);

    if (type == Pair) {
        connect(group, (void (QButtonGroup::*)(QAbstractButton *))(&QButtonGroup::buttonClicked), this, &FreeChooseDialog::disableButtons);
    }

    return tab;
}

void FreeChooseDialog::disableButtons(QAbstractButton *)
{
    QList<QAbstractButton *> buttons = group->buttons();
    QList<QAbstractButton *> checked;
    foreach(QAbstractButton *btn, buttons){
        if (btn->isChecked())
            checked << btn;
    }
    if (checked.length() == 2){
        foreach(QAbstractButton *btn, buttons){
            if (!btn->isChecked())
                btn->setEnabled(false);
            else
                btn->setEnabled(true);
        }
    } else if (checked.length() == 1){
        QString checked_kingdom = Sanguosha->getGeneral(checked.first()->objectName())->getKingdom();
        foreach(QAbstractButton *btn, buttons){
            QString btn_kingdom = Sanguosha->getGeneral(btn->objectName())->getKingdom();
            btn->setEnabled(checked_kingdom == btn_kingdom);
        }
    } else if (checked.length() == 0){
        foreach(QAbstractButton *btn, buttons)
            btn->setEnabled(true);
    } else {
        Q_ASSERT(false);
    }
}

