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
#include "exppattern.h"
#include "room.h"

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

LogMessage::LogMessage()
    : from(NULL)
{
}

QString LogMessage::toString() const{
    QStringList tos;
    foreach (ServerPlayer *player, to)
        if (player != NULL) tos << player->objectName();

    return QString("%1:%2->%3:%4:%5:%6")
        .arg(type)
        .arg(from ? from->objectName() : "")
        .arg(tos.join("+"))
        .arg(card_str).arg(arg).arg(arg2);
}

QVariant LogMessage::toVariant() const{
    QStringList tos;
    foreach (ServerPlayer *player, to)
        if (player != NULL) tos << player->objectName();

    QStringList log;
    log << type << (from ? from->objectName() : "") << tos.join("+") << card_str << arg << arg2;
    return JsonUtils::toJsonArray(log);
}

DamageStruct::DamageStruct()
    : from(NULL), to(NULL), card(NULL), damage(1), nature(Normal), chain(false), transfer(false), by_user(true), reason(QString()), transfer_reason(QString()), prevented(false)
{
}

DamageStruct::DamageStruct(const Card *card, ServerPlayer *from, ServerPlayer *to, int damage, DamageStruct::Nature nature)
    : chain(false), transfer(false), by_user(true), reason(QString()), transfer_reason(QString()), prevented(false)
{
    this->card = card;
    this->from = from;
    this->to = to;
    this->damage = damage;
    this->nature = nature;
}

DamageStruct::DamageStruct(const QString &reason, ServerPlayer *from, ServerPlayer *to, int damage, DamageStruct::Nature nature)
    : card(NULL), chain(false), transfer(false), by_user(true), transfer_reason(QString()), prevented(false)
{
    this->from = from;
    this->to = to;
    this->damage = damage;
    this->nature = nature;
    this->reason = reason;
}

QString DamageStruct::getReason() const{
    if (reason != QString())
        return reason;
    else if (card)
        return card->objectName();
    return QString();
}

CardEffectStruct::CardEffectStruct()
    : card(NULL), from(NULL), to(NULL), multiple(false), nullified(false)
{
}

SlashEffectStruct::SlashEffectStruct()
    : jink_num(1), slash(NULL), jink(NULL), from(NULL), to(NULL), drank(0), nature(DamageStruct::Normal), nullified(false)
{
}

DyingStruct::DyingStruct()
    : who(NULL), damage(NULL)
{
}

DeathStruct::DeathStruct()
    : who(NULL), damage(NULL)
{
}

RecoverStruct::RecoverStruct()
    : recover(1), who(NULL), card(NULL)
{
}

PindianStruct::PindianStruct()
    : from(NULL), to(NULL), from_card(NULL), to_card(NULL), success(false)
{
}

bool PindianStruct::isSuccess() const{
    return success;
}

JudgeStruct::JudgeStruct()
    : who(NULL), card(NULL), pattern("."), good(true), time_consuming(false),
    negative(false), play_animation(true), _m_result(TRIAL_RESULT_UNKNOWN)
{
}

bool JudgeStruct::isEffected() const{
    return negative ? isBad() : isGood();
}

void JudgeStruct::updateResult() {
    bool effected = (good == ExpPattern(pattern).match(who, card));
    if (effected)
        _m_result = TRIAL_RESULT_GOOD;
    else
        _m_result = TRIAL_RESULT_BAD;
}

bool JudgeStruct::isGood() const{
    Q_ASSERT(_m_result != TRIAL_RESULT_UNKNOWN);
    return _m_result == TRIAL_RESULT_GOOD;
}

bool JudgeStruct::isBad() const{
    return !isGood();
}

bool JudgeStruct::isGood(const Card *card) const{
    Q_ASSERT(card);
    return (good == ExpPattern(pattern).match(who, card));
}

PhaseChangeStruct::PhaseChangeStruct()
    : from(Player::NotActive), to(Player::NotActive)
{
}

CardUseStruct::CardUseStruct()
    : card(NULL), from(NULL), m_isOwnerUse(true), m_addHistory(true), nullified_list(QStringList())
{
}

CardUseStruct::CardUseStruct(const Card *card, ServerPlayer *from, QList<ServerPlayer *> to, bool isOwnerUse) {
    this->card = card;
    this->from = from;
    this->to = to;
    this->m_isOwnerUse = isOwnerUse;
    this->m_addHistory = true;
}

CardUseStruct::CardUseStruct(const Card *card, ServerPlayer *from, ServerPlayer *target, bool isOwnerUse) {
    this->card = card;
    this->from = from;
    this->to << target;
    this->m_isOwnerUse = isOwnerUse;
    this->m_addHistory = true;
}

bool CardUseStruct::isValid(const QString &pattern) const{
    Q_UNUSED(pattern)
        return card != NULL;
    /*if (card == NULL) return false;
    if (!card->getSkillName().isEmpty()) {
    bool validSkill = false;
    QString skillName = card->getSkillName();
    QSet<const Skill *> skills = from->getVisibleSkills();
    for (int i = 0; i < 4; i++) {
    const EquipCard *equip = from->getEquip(i);
    if (equip == NULL) continue;
    const Skill *skill = Sanguosha->getSkill(equip);
    if (skill)
    skills.insert(skill);
    }
    foreach (const Skill *skill, skills) {
    if (skill->objectName() != skillName) continue;
    const ViewAsSkill *vsSkill = ViewAsSkill::parseViewAsSkill(skill);
    if (vsSkill) {
    if (!vsSkill->isAvailable(from, m_reason, pattern))
    return false;
    else {
    validSkill = true;
    break;
    }
    } else if (skill->getFrequency() == Skill::Wake) {
    bool valid = (from->getMark(skill->objectName()) > 0);
    if (!valid)
    return false;
    else
    validSkill = true;
    } else
    return false;
    }
    if (!validSkill) return false;
    }
    if (card->targetFixed())
    return true;
    else {
    QList<const Player *> targets;
    foreach (const ServerPlayer *player, to)
    targets.push_back(player);
    return card->targetsFeasible(targets, from);
    }*/
}

bool CardUseStruct::tryParse(const QVariant &usage, Room *room) {
    JsonArray use = usage.value<JsonArray>();
    if (use.size() < 2 || !JsonUtils::isString(use[0]) || !use[1].canConvert<JsonArray>())
        return false;

    card = Card::Parse(use[0].toString());
    JsonArray targets = use[1].value<JsonArray>();

    foreach (const QVariant &target, targets) {
        if (!JsonUtils::isString(target)) return false;
        this->to << room->findChild<ServerPlayer *>(target.toString());
    }
    return true;
}

void CardUseStruct::parse(const QString &str, Room *room) {
    QStringList words = str.split("->", QString::KeepEmptyParts);
    Q_ASSERT(words.length() == 1 || words.length() == 2);

    QString card_str = words.at(0);
    QString target_str = ".";

    if (words.length() == 2 && !words.at(1).isEmpty())
        target_str = words.at(1);

    card = Card::Parse(card_str);

    if (target_str != ".") {
        QStringList target_names = target_str.split("+");
        foreach (const QString &target_name, target_names)
            to << room->findChild<ServerPlayer *>(target_name);
    }
}
