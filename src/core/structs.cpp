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

#include "structs.h"
#include "json.h"

bool CardsMoveStruct::tryParse(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 8) return false;

    if ((!JsonUtils::isNumber(args[0]) && !args[0].canConvert<JsonArray>()) ||
        !JsonUtils::isNumberArray(args, 1, 2) || !JsonUtils::isStringArray(args, 3, 6)) return false;

    if (JsonUtils::isNumber(args[0])) {
        int size = args[0].toInt();
        for (int i = 0; i < size; i++)
            card_ids.append(Card::S_UNKNOWN_CARD_ID);
    } else if (!JsonUtils::tryParse(args[0], card_ids)) {
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

QVariant CardsMoveStruct::toVariant() const{
    JsonArray arg;
    if (open) {
        arg << JsonUtils::toJsonArray(card_ids);
    } else {
        arg << card_ids.size();
    }

    arg << (int)from_place;
    arg << (int)to_place;
    arg << from_player_name;
    arg << to_player_name;
    arg << from_pile_name;
    arg << to_pile_name;
    arg << reason.toVariant();
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

QVariant CardMoveReason::toVariant() const{
    JsonArray result;
    result << m_reason;
    result << m_playerId;
    result << m_skillName;
    result << m_eventName;
    result << m_targetId;
    return result;
}

