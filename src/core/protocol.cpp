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

#include "protocol.h"
#include "json.h"

using namespace QSanProtocol;

unsigned int QSanProtocol::Packet::globalSerialSequence = 0;
const int QSanProtocol::Packet::S_MAX_PACKET_SIZE = 65535;
const char *QSanProtocol::S_PLAYER_SELF_REFERENCE_ID = "MG_SELF";

const int QSanProtocol::S_ALL_ALIVE_PLAYERS = 0;

bool QSanProtocol::Countdown::tryParse(const QVariant &var)
{
    if (!var.canConvert<JsonArray>())
        return false;

    JsonArray val = var.value<JsonArray>();

    //compatible with old JSON representation of Countdown
    if (JsonUtils::isString(val[0])) {
        if (val[0].toString() == "MG_COUNTDOWN")
            val.removeFirst();
        else
            return false;
    }

    if (val.size() == 2) {
        if (!JsonUtils::isNumberArray(val, 0, 1)) return false;
        current = (time_t)val[0].toInt();
        max = (time_t)val[1].toInt();
        type = S_COUNTDOWN_USE_SPECIFIED;
        return true;

    } else if (val.size() == 1 && val[0].canConvert<int>()) {
        CountdownType type = (CountdownType)val[0].toInt();
        if (type != S_COUNTDOWN_NO_LIMIT && type != S_COUNTDOWN_USE_DEFAULT)
            return false;
        else this->type = type;
        return true;

    } else
        return false;
}

QVariant QSanProtocol::Countdown::toVariant() const
{
    JsonArray val;
    if (type == S_COUNTDOWN_NO_LIMIT || type == S_COUNTDOWN_USE_DEFAULT) {
        val << (int)type;
    } else {
        val << (int)current;
        val << (int)max;
    }
    return val;
}

QSanProtocol::Packet::Packet(int packetDescription, CommandType command)
    : globalSerial(0), localSerial(0),
    command(command),
    packetDescription(static_cast<PacketDescription>(packetDescription))
{
}

unsigned int QSanProtocol::Packet::createGlobalSerial()
{
    globalSerial = ++globalSerialSequence;
    return globalSerial;
}

bool QSanProtocol::Packet::parse(const QByteArray &raw)
{
    if (raw.length() > S_MAX_PACKET_SIZE) {
        return false;
    }

    JsonDocument doc = JsonDocument::fromJson(raw);
    JsonArray result = doc.array();

    if (!JsonUtils::isNumberArray(result, 0, 3) || result.size() > 5)
        return false;

    globalSerial = result[0].toUInt();
    localSerial = result[1].toUInt();
    packetDescription = static_cast<PacketDescription>(result[2].toInt());
    command = (CommandType)result[3].toInt();

    if (result.size() == 5)
        messageBody = result[4];
    return true;
}

QByteArray QSanProtocol::Packet::toJson() const
{
    JsonArray result;
    result << globalSerial;
    result << localSerial;
    result << packetDescription;
    result << command;
    if (!messageBody.isNull())
        result << messageBody;

    JsonDocument doc(result);
    const QByteArray &msg = doc.toJson();

    //return an empty string here, for Packet::parse won't parse it (line 92)
    if (msg.length() > S_MAX_PACKET_SIZE)
        return QByteArray();

    return msg;
}

QString QSanProtocol::Packet::toString() const
{
    return QString::fromUtf8(toJson());
}
