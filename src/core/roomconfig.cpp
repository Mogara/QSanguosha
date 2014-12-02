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

#include <QCryptographicHash>
#include <QSqlQuery>

#include "roomconfig.h"
#include "json.h"
#include "settings.h"

RoomConfig::RoomConfig(const Settings *config)
{
    ServerName = config->ServerName;
    if (!config->RoomPassword.isEmpty())
        Password = QCryptographicHash::hash(config->RoomPassword.toLatin1(), QCryptographicHash::Md5).toHex();
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
    if (config.size() != 17)
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
        QString generals = pair.toString();
        int split_pos = generals.indexOf('+');
        if (split_pos <= 0 || split_pos >= generals.size() - 1)
            continue;

        QString general1 = generals.left(split_pos);
        QString general2 = generals.mid(split_pos + 1);
        if (!general1.isEmpty() && !general2.isEmpty())
            BannedGeneralPairs << BanPair(general1, general2);
    }

    Password = config.at(16).toString();

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
        QString pair_str(pair.first);
        pair_str.append('+');
        pair_str.append(pair.second);
        banned_general_pairs << pair_str;
    }
    data << QVariant(banned_general_pairs);

    data << Password;

    return data;
}

bool RoomConfig::isBanned(const QString &first, const QString &second) {
    if (BannedGenerals.contains(first) || BannedGenerals.contains(second))
        return true;

    BanPair pair(first, second);
    return BannedGeneralPairs.contains(pair);
}

RoomInfoStruct::RoomInfoStruct()
{
    RoomId = 0;
    PlayerNum = -1;
}

RoomInfoStruct::RoomInfoStruct(const Settings *config)
{
    RoomId = 0;
    PlayerNum = -1;
    Name = config->ServerName;
    GameMode = config->GameMode;
    BanPackages = config->BanPackages.toSet();
    OperationTimeout = config->OperationTimeout;
    NullificationCountDown = config->NullificationCountDown;
    RandomSeat = config->RandomSeat;
    EnableCheat = config->EnableCheat;
    FreeChoose = config->FreeChoose;
    ForbidAddingRobot = config->ForbidAddingRobot;
    DisableChat = config->DisableChat;
    FirstShowingReward = config->RewardTheFirstShowingPlayer;
    RequirePassword = !config->RoomPassword.isEmpty();
}

RoomInfoStruct::RoomInfoStruct(const RoomConfig &config)
{
    RoomId = 0;
    PlayerNum = -1;
    Name = config.ServerName;
    GameMode = config.GameMode;
    BanPackages = config.BanPackages;
    OperationTimeout = config.OperationTimeout;
    NullificationCountDown = config.NullificationCountDown;
    RandomSeat = config.RandomSeat;
    EnableCheat = config.EnableCheat;
    FreeChoose = config.FreeChoose;
    ForbidAddingRobot = config.ForbidAddingRobot;
    DisableChat = config.DisableChat;
    FirstShowingReward = config.RewardTheFirstShowingPlayer;
    RequirePassword = !config.Password.isEmpty();
}

RoomInfoStruct::RoomInfoStruct(const QSqlRecord &record)
{
    RoomId = record.value("id").toLongLong();
    PlayerNum = record.value("playernum").toInt();
    HostAddress = record.value("hostaddress").toString();
    Name = record.value("name").toString();
    GameMode = record.value("gamemode").toString();
    QStringList ban_packages = record.value("banpackages").toString().split('+');
    foreach(const QString &package, ban_packages)
        BanPackages.insert(package);
    OperationTimeout = record.value("operationtimeout").toInt();
    NullificationCountDown = record.value("nullificationcountdown").toInt();
    RandomSeat = record.value("randomseat").toBool();
    EnableCheat = record.value("enablecheat").toBool();
    FreeChoose = record.value("freechoose").toBool();
    ForbidAddingRobot = record.value("forbidaddingrobot").toBool();
    DisableChat = record.value("disablechat").toBool();
    FirstShowingReward = record.value("firstshowingreward").toBool();
    RequirePassword = record.value("requirepassword").toBool();
}

time_t RoomInfoStruct::getCommandTimeout(QSanProtocol::CommandType command, QSanProtocol::ProcessInstanceType instance) {
    time_t timeOut;
    if (OperationTimeout == 0)
        return 0;
    else if (command == QSanProtocol::S_COMMAND_CHOOSE_GENERAL)
        timeOut = OperationTimeout * 1500;
    else if (command == QSanProtocol::S_COMMAND_SKILL_GUANXING
        || command == QSanProtocol::S_COMMAND_ARRANGE_GENERAL)
        timeOut = OperationTimeout * 2000;
    else if (command == QSanProtocol::S_COMMAND_NULLIFICATION)
        timeOut = NullificationCountDown * 1000;
    else
        timeOut = OperationTimeout * 1000;

    if (instance == QSanProtocol::S_SERVER_INSTANCE)
        timeOut += Config.S_SERVER_TIMEOUT_GRACIOUS_PERIOD;
    return timeOut;
}

bool RoomInfoStruct::parse(const QVariant &var)
{
    JsonArray data = var.value<JsonArray>();
    if (data.size() != 10)
        return false;

    RoomId = data.at(0).toLongLong();
    HostAddress = data.at(1).toString();
    PlayerNum = data.at(2).toInt();
    RoomState = static_cast<State>(data.at(3).toInt());

    Name = data.at(4).toString();
    GameMode = data.at(5).toString();
    BanPackages.clear();
    JsonArray banned_packages = data.at(6).value<JsonArray>();
    foreach (const QVariant &package, banned_packages)
        BanPackages.insert(package.toString());
    OperationTimeout = data.at(7).toInt();
    NullificationCountDown = data.at(8).toInt();

    int flags = data.at(9).toInt();
#define getFlag(flag) flag = ((flags & 0x1) == 0x1); flags >>= 1
    getFlag(RequirePassword);
    getFlag(FirstShowingReward);
    getFlag(DisableChat);
    getFlag(ForbidAddingRobot);
    getFlag(FreeChoose);
    getFlag(EnableCheat);
    getFlag(RandomSeat);
#undef getFlag

    return true;
}

QVariant RoomInfoStruct::toQVariant() const
{
    JsonArray data;
    data << RoomId;
    data << HostAddress;
    data << PlayerNum;
    data << RoomState;

    data << Name;
    data << GameMode;
    JsonArray banned_packages;
    foreach (const QString &package, BanPackages)
        banned_packages << package;
    data << QVariant(banned_packages);
    data << OperationTimeout;
    data << NullificationCountDown;

    int flags = 0;
#define setFlag(flag) flags = (flags << 1) | (flag ? 1 : 0)
    setFlag(RandomSeat);
    setFlag(EnableCheat);
    setFlag(FreeChoose);
    setFlag(ForbidAddingRobot);
    setFlag(DisableChat);
    setFlag(FirstShowingReward);
    setFlag(RequirePassword);
#undef setFlag
    data << flags;

    return data;
}

qlonglong RoomInfoStruct::save()
{
    if (RoomId <= 0) {
        QSqlQuery query;
        query.prepare("INSERT INTO `room` (`hostaddress`, `name`,`gamemode`,`banpackages`,`operationtimeout`,`nullificationcountdown`,`randomseat`,`enablecheat`,`freechoose`,`forbidaddingrobot`,`disablechat`,`firstshowingreward`,`requirepassword`) VALUES (:hostaddress, :name, :gamemode, :banpackages, :operationtimeout, :nullificationcountdown, :randomseat, :enablecheat, :freechoose, :forbidaddingrobot, :disablechat, :firstshowingreward, :requirepassword)");
        query.bindValue(":hostaddress", HostAddress);
        query.bindValue(":name", Name);
        query.bindValue(":gamemode", GameMode);
        query.bindValue(":banpackages", QStringList(BanPackages.toList()).join("+"));
        query.bindValue(":operationtimeout", OperationTimeout);
        query.bindValue(":nullificationcountdown", NullificationCountDown);
        query.bindValue(":randomseat", RandomSeat);
        query.bindValue(":enablecheat", EnableCheat);
        query.bindValue(":freechoose", FreeChoose);
        query.bindValue(":forbidaddingrobot", ForbidAddingRobot);
        query.bindValue(":disablechat", DisableChat);
        query.bindValue(":firstshowingreward", FirstShowingReward);
        query.bindValue(":requirepassword", RequirePassword);
        query.exec();

        RoomId = query.lastInsertId().toLongLong();
    }

    return RoomId;
}
