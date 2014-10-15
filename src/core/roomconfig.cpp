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

#include "roomconfig.h"
#include "json.h"
#include "settings.h"

RoomConfig::RoomConfig(const Settings *config)
{
    ServerName = config->ServerName;
    OperationTimeout = config->OperationTimeout;
    OperationNoLimit = config->OperationNoLimit;
    RandomSeat = config->RandomSeat;
    EnableCheat = config->EnableCheat;
    FreeChoose = config->FreeChoose;
    DisableChat = config->DisableChat;
    CountDownSeconds = config->CountDownSeconds;
    NullificationCountDown = config->NullificationCountDown;
    EnableMinimizeDialog = config->EnableMinimizeDialog;
    RewardTheFirstShowingPlayer = config->RewardTheFirstShowingPlayer;
    ForbidAddingRobot = config->ForbidAddingRobot;
    OriginAIDelay = config->OriginAIDelay;
    AIDelay = config->AIDelay;
    AIDelayAD = config->AIDelayAD;
    AlterAIDelayAD = config->AlterAIDelayAD;
    DisableLua = config->DisableLua;
    SurrenderAtDeath = config->SurrenderAtDeath;
    LuckCardLimitation = config->LuckCardLimitation;
    GameMode = config->GameMode;
    PileSwappingLimitation = config->value("PileSwappingLimitation", 5).toInt();
    HegemonyMaxChoice = config->value("HegemonyMaxChoice", 7).toInt();
    AIChat = config->value("AIChat", true).toBool();
    BanPackages = config->BanPackages;
    EnableLordConvertion = config->value("EnableLordConvertion", true).toBool();
    CardConversions = config->value("CardConversions").toStringList();
}

bool RoomConfig::parse(const QVariant &data)
{
    JsonArray config = data.value<JsonArray>();
    if (config.size() != 14)
        return false;

    GameMode = config.at(0).toString();
    ServerName = config.at(1).toString();

    OperationTimeout = config.at(2).toInt();
    CountDownSeconds = config.at(3).toInt();
    NullificationCountDown = config.at(4).toInt();
    OriginAIDelay = config.at(5).toInt();
    AIDelay = config.at(6).toInt();
    AIDelayAD = config.at(7).toInt();
    LuckCardLimitation = config.at(8).toInt();
    PileSwappingLimitation = config.at(9).toInt();
    HegemonyMaxChoice = config.at(10).toInt();

    int flag = config.at(11).toInt();
#define getFlag(var) var = static_cast<bool>(flag & 0x1);flag >>= 1
    getFlag(EnableLordConvertion);
    getFlag(SurrenderAtDeath);
    getFlag(DisableLua);
    getFlag(AlterAIDelayAD);
    getFlag(ForbidAddingRobot);
    getFlag(RewardTheFirstShowingPlayer);
    getFlag(EnableMinimizeDialog);
    getFlag(DisableChat);
    getFlag(FreeChoose);
    getFlag(EnableCheat);
    getFlag(RandomSeat);
    getFlag(OperationNoLimit);
    getFlag(AIChat);
#undef getFlag

    BanPackages = config.at(12).toStringList();
    CardConversions = config.at(13).toStringList();
    return true;
}

QVariant RoomConfig::toVariant() const
{
    JsonArray data;
    data << GameMode;
    data << ServerName;

    data << OperationTimeout;
    data << CountDownSeconds;
    data << NullificationCountDown;
    data << OriginAIDelay;
    data << AIDelay;
    data << AIDelayAD;
    data << LuckCardLimitation;
    data << PileSwappingLimitation;
    data << HegemonyMaxChoice;

    int flag = AIChat;
#define setFlag(var) flag = (flag << 1) | static_cast<int>(var)
    setFlag(OperationNoLimit);
    setFlag(RandomSeat);
    setFlag(EnableCheat);
    setFlag(FreeChoose);
    setFlag(DisableChat);
    setFlag(EnableMinimizeDialog);
    setFlag(RewardTheFirstShowingPlayer);
    setFlag(ForbidAddingRobot);
    setFlag(AlterAIDelayAD);
    setFlag(DisableLua);
    setFlag(SurrenderAtDeath);
    setFlag(EnableLordConvertion);
#undef setFlag

    data << BanPackages;
    data << CardConversions;

    return data;
}
