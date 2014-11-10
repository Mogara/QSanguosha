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

#include "banipdialog.h"
#include "server.h"
#include "room.h"
#include "serverplayer.h"
#include "settings.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>

BanIpDialog::BanIpDialog(QWidget *parent, Server *server)
    : FlatDialog(parent), server(server) {
    /*
        if (Sanguosha->currentRoom() == NULL){
        QMessageBox::warning(this, tr("Warining!"), tr("Game is not started!"));
        return;
        }
        */
    QVBoxLayout *left_layout = new QVBoxLayout;
    QVBoxLayout *right_layout = new QVBoxLayout;

    left = new QListWidget;
    left->setSortingEnabled(false);
    right = new QListWidget;

    QPushButton *insert = new QPushButton(tr("Insert to banned IP list"));
    QPushButton *kick = new QPushButton(tr("Kick from server"));

    QPushButton *remove = new QPushButton(tr("Remove from banned IP list"));

    left_layout->addWidget(left);

    QHBoxLayout *left_button_layout = new QHBoxLayout;
    left_button_layout->addWidget(insert);
    left_button_layout->addWidget(kick);

    left_layout->addLayout(left_button_layout);

    right_layout->addWidget(right);
    right_layout->addWidget(remove);

    QPushButton *ok = new QPushButton(tr("OK"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));

    QHBoxLayout *up_layout = new QHBoxLayout;
    up_layout->addLayout(left_layout);
    up_layout->addLayout(right_layout);

    QHBoxLayout *down_layout = new QHBoxLayout;
    down_layout->addWidget(ok);
    down_layout->addWidget(cancel);

    layout->addLayout(up_layout);
    layout->addLayout(down_layout);

    connect(ok, &QPushButton::clicked, this, &BanIpDialog::accept);
    connect(this, &BanIpDialog::accepted, this, &BanIpDialog::save);
    connect(cancel, &QPushButton::clicked, this, &BanIpDialog::reject);
    connect(insert, &QPushButton::clicked, this, &BanIpDialog::insertClicked);
    connect(remove, &QPushButton::clicked, this, &BanIpDialog::removeClicked);
    connect(kick, &QPushButton::clicked, this, &BanIpDialog::kickClicked);

    if (server)
        loadIPList();
    else
        QMessageBox::warning(this, tr("Warning!"), tr("There is no server running!"));

    loadBannedList();

    setWindowTitle(tr("Manage connected players"));
}

void BanIpDialog::loadIPList(){
    foreach(Room *room, server->rooms){
        foreach(ServerPlayer *p, room->getPlayers()){
            if (p->getState() != "offline" && p->getState() != "robot") {
                sp_list << p;
            }
        }
    }

    left->clear();

    foreach(ServerPlayer *p, sp_list){
        QString parsed_string = QString("%1::%2").arg(p->screenName(), p->getIp());
        left->addItem(parsed_string);
    }
}

void BanIpDialog::loadBannedList() {
    QStringList banned = Config.value("BannedIP", QStringList()).toStringList();

    right->clear();
    right->addItems(banned);
}

void BanIpDialog::insertClicked() {
    int row = left->currentRow();
    if (row != -1){
        QString ip = left->currentItem()->text().split("::").last();

        if (ip.startsWith("127."))
            QMessageBox::warning(this, tr("Warning!"), tr("This is your local Loopback Address and can't be banned!"));

        if (right->findItems(ip, Qt::MatchFlags(Qt::MatchExactly)).isEmpty())
            right->addItem(ip);
    }
}

void BanIpDialog::removeClicked(){
    int row = right->currentRow();
    if (row != -1)
        delete right->takeItem(row);
}

void BanIpDialog::kickClicked(){
    int row = left->currentRow();
    if (row != -1){
        ServerPlayer *p = sp_list[row];
        QStringList split_data = left->currentItem()->text().split("::");
        QString ip = split_data.takeLast();
        QString screenName = split_data.join("::");
        if (p->screenName() == screenName && p->getIp() == ip)
            p->kick(); // procedure kick
    }
}

void BanIpDialog::save(){
    QSet<QString> ip_set;

    for (int i = 0; i < right->count(); i++)
        ip_set << right->item(i)->text();

    QStringList ips = ip_set.toList();
    Config.setValue("BannedIP", ips);
}

void BanIpDialog::addPlayer(ServerPlayer *player)
{
    if (player->getState() != "offline" && player->getState() != "robot") {
        sp_list << player;
    }

    QString parsed_string = QString("%1::%2").arg(player->screenName(), player->getIp());
    left->addItem(parsed_string);
    connect(player, &ServerPlayer::disconnected, this, &BanIpDialog::removePlayer);
}

void BanIpDialog::removePlayer()
{
    ServerPlayer *player = qobject_cast<ServerPlayer *>(sender());
    if (player) {
        int row = sp_list.indexOf(player);
        if (row != -1) {
            delete left->takeItem(row);
            sp_list.removeAt(row);
        }
    }
}
