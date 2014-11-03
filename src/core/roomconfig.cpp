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
    BanPackages = config->BanPackages.toSet();
    EnableLordConvertion = config->value("EnableLordConvertion", true).toBool();
    CardConversions = config->value("CardConversions").toStringList().toSet();
    BannedGenerals = config->value("Banlist/Generals").toStringList().toSet();
    QStringList banned_pairs = config->value("Banlist/Pairs").toStringList();
    foreach (const QString &pair, banned_pairs) {
        int split_pos = pair.indexOf('+');
        if (split_pos > 0 && split_pos < pair.length() - 1) {
            QStringList generals = pair.split('+');
            BannedGeneralPairs.insert(BanPair(generals.first(), generals.last()));
        }
    }
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
#define getFlag(var) var = ((flag & 1) == 1);flag >>= 1
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

    JsonArray ban_packages = config.at(12).value<JsonArray>();
    foreach (const QVariant &package, ban_packages) {
        BanPackages << package.toString();
    }

    JsonArray card_conversions = config.at(13).value<JsonArray>();
    foreach (const QVariant &conversion, card_conversions) {
        CardConversions << conversion.toString();
    }

    JsonArray banned_generals = config.at(14).value<JsonArray>();
    foreach (const QVariant &general, banned_generals) {
        BannedGenerals << general.toString();
    }

    JsonArray banned_general_pairs = config.at(15).value<JsonArray>();
    foreach (const QVariant &pair, banned_general_pairs) {
        JsonArray generals = pair.value<JsonArray>();
        if (generals.size() != 2)
            continue;

        QString general1 = generals.first().toString();
        QString general2 = generals.last().toString();
        if (!general1.isEmpty() && !general2.isEmpty())
            BannedGeneralPairs << BanPair(general1, general2);
    }

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
#define setFlag(var) flag = (flag << 1) | (var ? 1 : 0)
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
    data << flag;

    JsonArray ban_packages;
    foreach (const QString &package, BanPackages) {
        ban_packages << package;
    }
    data << QVariant(ban_packages);

    JsonArray card_conversions;
    foreach (const QString conversions, CardConversions) {
        card_conversions << conversions;
    }
    data << QVariant(card_conversions);

    JsonArray banned_generals;
    foreach (const QString &general, BannedGenerals) {
        banned_generals << general;
    }
    data << QVariant(banned_generals);

    JsonArray banned_general_pairs;
    foreach (const BanPair &pair, BannedGeneralPairs) {
        JsonArray array;
        array << pair.first << pair.second;
        banned_general_pairs << QVariant(array);
    }
    data << QVariant(banned_general_pairs);

    return data;
}

bool RoomConfig::isBanned(const QString &first, const QString &second) {
    if (BannedGenerals.contains(first) || BannedGenerals.contains(second))
        return true;

    BanPair pair(first, second);
    return BannedGeneralPairs.contains(pair);
}
