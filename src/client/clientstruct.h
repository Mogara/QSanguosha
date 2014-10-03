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

#ifndef _CLIENT_STRUCT_H
#define _CLIENT_STRUCT_H

#include "player.h"
#include "qsanselectableitem.h"
#include "protocol.h"
#include <QMap>
#include <QWidget>

struct ServerInfoStruct {
    bool parse(const QString &str);
    //Get the timeout allowance for a command. Server countdown is more lenient than the client.
    //@param command: type of command
    //@return countdown for command in milliseconds.
    time_t getCommandTimeout(QSanProtocol::CommandType command, QSanProtocol::ProcessInstanceType instance);

    QString Name;
    QString GameMode;
    int OperationTimeout;
    int NullificationCountDown;
    QStringList Extensions;
    bool RandomSeat;
    bool EnableCheat;
    bool FreeChoose;
    bool ForbidAddingRobot;
    bool DisableChat;
    bool FirstShowingReward;

    bool DuringGame;
};

extern ServerInfoStruct ServerInfo;

class QLabel;
class QListWidget;

class ServerInfoWidget : public QWidget {
    Q_OBJECT

public:
    ServerInfoWidget(bool show_lack = false);
    void fill(const ServerInfoStruct &info, const QString &address);
    void updateLack(int count);
    void clear();

private:
    QLabel *name_label;
    QLabel *address_label;
    QLabel *port_label;
    QLabel *game_mode_label;
    QLabel *player_count_label;
    QLabel *random_seat_label;
    QLabel *enable_cheat_label;
    QLabel *free_choose_label;
    QLabel *forbid_adding_robot_label;
    QLabel *fisrt_showing_reward_label;
    QLabel *time_limit_label;
    QLabel *lack_label;
    QListWidget *list_widget;
};
#endif

