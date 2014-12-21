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

#include "clientstruct.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "json.h"

RoomInfoStruct ServerInfo;

#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>

ServerInfoWidget::ServerInfoWidget(bool show_lack) {
    name_label = new QLabel;
    address_label = new QLabel;
    port_label = new QLabel;
    game_mode_label = new QLabel;
    player_count_label = new QLabel;
    random_seat_label = new QLabel;
    enable_cheat_label = new QLabel;
    free_choose_label = new QLabel;
    forbid_adding_robot_label = new QLabel;
    fisrt_showing_reward_label = new QLabel;
    time_limit_label = new QLabel;

    list_widget = new QListWidget;
    list_widget->setViewMode(QListView::IconMode);
    list_widget->setMovement(QListView::Static);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Server name"), name_label);
    layout->addRow(tr("Address"), address_label);
    layout->addRow(tr("Port"), port_label);
    layout->addRow(tr("Game mode"), game_mode_label);
    layout->addRow(tr("Player count"), player_count_label);
    layout->addRow(tr("Random seat"), random_seat_label);
    layout->addRow(tr("Enable cheat"), enable_cheat_label);
    layout->addRow(tr("Free choose"), free_choose_label);
    layout->addRow(tr("Forbid adding robot"), forbid_adding_robot_label);
    layout->addRow(tr("Enable First Showing Reward"), fisrt_showing_reward_label);
    layout->addRow(tr("Operation time"), time_limit_label);
    layout->addRow(tr("Extension packages"), list_widget);

    if (show_lack) {
        lack_label = new QLabel;
        layout->addRow(tr("Lack"), lack_label);
    }
    else
        lack_label = NULL;

    setLayout(layout);
}

void ServerInfoWidget::fill(const RoomInfoStruct &info, const QString &address) {
    name_label->setText(info.Name);
    address_label->setText(address);
    game_mode_label->setText(Sanguosha->getModeName(info.GameMode));
    int player_count = Sanguosha->getPlayerCount(info.GameMode);
    player_count_label->setText(QString::number(player_count));
    port_label->setText(QString::number(Config.ServerPort));

    random_seat_label->setText(info.RandomSeat ? tr("Enabled") : tr("Disabled"));
    enable_cheat_label->setText(info.EnableCheat ? tr("Enabled") : tr("Disabled"));
    free_choose_label->setText(info.FreeChoose ? tr("Enabled") : tr("Disabled"));
    forbid_adding_robot_label->setText(info.ForbidAddingRobot ? tr("Enabled") : tr("Disabled"));
    fisrt_showing_reward_label->setText(info.FirstShowingReward ? tr("Enabled") : tr("Disabled"));

    if (info.OperationTimeout == 0)
        time_limit_label->setText(tr("No limit"));
    else
        time_limit_label->setText(tr("%1 seconds").arg(info.OperationTimeout));

    list_widget->clear();

    static QIcon enabled_icon("image/system/enabled.png");
    static QIcon disabled_icon("image/system/disabled.png");

    QStringList extentions = Sanguosha->getExtensions();
    foreach (const QString &extension, extentions) {
        bool checked = !info.BanPackages.contains(extension);
        QString package_name = Sanguosha->translate(extension);
        QCheckBox *checkbox = new QCheckBox(package_name);
        checkbox->setChecked(checked);

        new QListWidgetItem(checked ? enabled_icon : disabled_icon, package_name, list_widget);
    }
}

void ServerInfoWidget::updateLack(int count) {
    if (lack_label) {
        QString path = QString("image/system/number/%1.png").arg(count);
        lack_label->setPixmap(QPixmap(path));
    }
}

void ServerInfoWidget::clear() {
    name_label->clear();
    address_label->clear();
    port_label->clear();
    game_mode_label->clear();
    player_count_label->clear();
    random_seat_label->clear();
    enable_cheat_label->clear();
    free_choose_label->clear();
    forbid_adding_robot_label->clear();
    fisrt_showing_reward_label->clear();
    time_limit_label->clear();
    list_widget->clear();
}
