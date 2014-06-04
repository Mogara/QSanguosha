/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#include "protocol.h"
#include <json/json.h>

using namespace std;
using namespace QSanProtocol;

unsigned int QSanProtocol::Packet::_m_globalSerial = 0;
const unsigned int QSanProtocol::Packet::S_MAX_PACKET_SIZE = 65535;
const string QSanProtocol::Countdown::S_COUNTDOWN_MAGIC = "MG_COUNTDOWN";
const char *QSanProtocol::S_PLAYER_SELF_REFERENCE_ID = "MG_SELF";

bool QSanProtocol::Countdown::tryParse(Json::Value val) {
    if (!val.isArray() || (val.size() != 2 && val.size() != 3) ||
        !val[0].isString() || val[0].asString() != S_COUNTDOWN_MAGIC)
        return false;
    if (val.size() == 3) {
        if (!Utils::isIntArray(val, 1, 2)) return false;
        m_current = (time_t)val[1].asInt();
        m_max = (time_t)val[2].asInt();
        m_type = S_COUNTDOWN_USE_SPECIFIED;
        return true;
    }
    else if (val.size() == 2) {
        CountdownType type = (CountdownType)val[1].asInt();
        if (type != S_COUNTDOWN_NO_LIMIT && type != S_COUNTDOWN_USE_DEFAULT)
            return false;
        else m_type = type;
        return true;
    }
    else
        return false;
}

bool QSanProtocol::Utils::isStringArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray() || jsonObject.size() <= endIndex)
        return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!jsonObject[i].isString())
            return false;
    }
    return true;
}

bool QSanProtocol::Utils::isIntArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex) {
    if (!jsonObject.isArray() || jsonObject.size() <= endIndex)
        return false;
    for (unsigned int i = startIndex; i <= endIndex; i++) {
        if (!jsonObject[i].isInt())
            return false;
    }
    return true;
}

bool QSanProtocol::Packet::parse(const string &s) {
    if (s.length() > S_MAX_PACKET_SIZE) {
        return false;
    }

    Json::Value result;

    Json::Reader reader;
    bool success = reader.parse(s, result);
    if (!success || !Utils::isIntArray(result, 0, 3) || result.size() > 5)
        return false;

    m_globalSerial = (unsigned int)result[0].asInt();
    m_localSerial = (unsigned int)result[1].asInt();
    m_packetDescription = static_cast<PacketDescription>(result[2].asInt());
    m_command = (CommandType)result[3].asInt();

    if (result.size() == 5)
        setBody(result[4]);
    return true;
}


//characters in JSON string representations are unicode-escaped. So we don't need Base64 here.
QByteArray QSanProtocol::Packet::toUtf8() const{
    Json::Value result(Json::arrayValue);
    result[0] = m_globalSerial;
    result[1] = m_localSerial;
    result[2] = m_packetDescription;
    result[3] = m_command;
    const Json::Value &body = getBody();
    if (body != Json::nullValue)
        result[4] = body;

    Json::FastWriter writer;
    string msg = writer.write(result);

    //truncate too long messages
    if (msg.length() > S_MAX_PACKET_SIZE)
        msg = msg.substr(0, S_MAX_PACKET_SIZE);

    return msg.c_str();
}

QString QSanProtocol::Packet::toString() const{
    return QString::fromUtf8(toUtf8());
}

