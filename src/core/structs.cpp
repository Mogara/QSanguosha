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

#include "structs.h"
#include "jsonutils.h"
#include "protocol.h"

using namespace QSanProtocol::Utils;

bool CardsMoveStruct::tryParse(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 8) return false;

    if ((!JsonUtils::isNumber(args[0]) && !args[0].canConvert<JsonArray>()) ||
        !JsonUtils::isIntArray(args, 1, 2) || !JsonUtils::isStringArray(args, 3, 6)) return false;

    if (JsonUtils::isNumber(args[0])) {
        int size = args[0].toInt();
        for (int i = 0; i < size; i++)
            card_ids.append(Card::S_UNKNOWN_CARD_ID);
    } else if (!JsonUtils::tryParse(args[0].value<JsonArray>(), card_ids)) {
        return false;
    }

    from_place = (Player::Place)args[1].toInt();
    to_place = (Player::Place)args[2].toInt();
    from_player_name = args[3].toString();
    to_player_name = args[4].toString();
    from_pile_name = args[5].toString();
    to_pile_name = args[6].toString();
    reason.tryParse(args[7]);
    return true;
}

Json::Value CardsMoveStruct::toJsonValue() const{
    Json::Value arg(Json::arrayValue);
    if (open) arg[0] = toJsonArray(card_ids);
    else arg[0] = card_ids.size();
    arg[1] = (int)from_place;
    arg[2] = (int)to_place;
    arg[3] = toJsonString(from_player_name);
    arg[4] = toJsonString(to_player_name);
    arg[5] = toJsonString(from_pile_name);
    arg[6] = toJsonString(to_pile_name);
    arg[7] = reason.toJsonValue();
    return arg;
}

bool CardMoveReason::tryParse(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 5 || !args[0].canConvert<int>() || !JsonUtils::isStringArray(args, 1, 4))
        return false;

    m_reason = args[0].toInt();
    m_playerId = args[1].toString();
    m_skillName = args[2].toString();
    m_eventName = args[3].toString();
    m_targetId = args[4].toString();

    return true;
}

Json::Value CardMoveReason::toJsonValue() const{
    Json::Value result;
    result[0] = m_reason;
    result[1] = toJsonString(m_playerId);
    result[2] = toJsonString(m_skillName);
    result[3] = toJsonString(m_eventName);
    result[4] = toJsonString(m_targetId);
    return result;
}

