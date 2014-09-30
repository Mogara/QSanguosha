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

#include "roomthread.h"
#include "room.h"
#include "engine.h"
#include "gamerule.h"
#include "scenerule.h"
#include "scenario.h"
#include "ai.h"
#include "settings.h"
#include "standard.h"
#include "json.h"

#include <QTime>

#ifdef QSAN_UI_LIBRARY_AVAILABLE
#pragma message WARN("UI elements detected in server side!!!")
#endif

LogMessage::LogMessage()
    : from(NULL)
{
}

QString LogMessage::toString() const{
    QStringList tos;
    foreach(ServerPlayer *player, to)
        if (player != NULL) tos << player->objectName();

    return QString("%1:%2->%3:%4:%5:%6")
        .arg(type)
        .arg(from ? from->objectName() : "")
        .arg(tos.join("+"))
        .arg(card_str).arg(arg).arg(arg2);
}

QVariant LogMessage::toVariant() const{
    QStringList tos;
    foreach(ServerPlayer *player, to)
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
    : card(NULL), from(NULL), to(NULL)
{
}

SlashEffectStruct::SlashEffectStruct()
    : jink_num(1), slash(NULL), jink(NULL), from(NULL), to(NULL), drank(0), nature(DamageStruct::Normal)
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
    : card(NULL), from(NULL), m_isOwnerUse(true), m_addHistory(true)
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
        foreach(QString target_name, target_names)
            to << room->findChild<ServerPlayer *>(target_name);
    }
}

QString EventTriplet::toString() const{
    return QString("event[%1], room[%2], target = %3[%4]\n")
        .arg(_m_event)
        .arg(_m_room->getId())
        .arg(_m_target ? _m_target->objectName() : "NULL")
        .arg(_m_target ? _m_target->getGeneralName() : QString());
}

QString HegemonyMode::GetMappedRole(const QString &kingdom) {
    static QMap<QString, QString> roles;
    if (roles.isEmpty()) {
        roles["wei"] = "lord";
        roles["shu"] = "loyalist";
        roles["wu"] = "rebel";
        roles["qun"] = "renegade";
        roles["god"] = "careerist";
    }
    if (roles[kingdom].isEmpty())
        return kingdom;
    return roles[kingdom];
}

QString HegemonyMode::GetMappedKingdom(const QString &role) {
    static QMap<QString, QString> kingdoms;
    if (kingdoms.isEmpty()){
        kingdoms["lord"] = "wei";
        kingdoms["loyalist"] = "shu";
        kingdoms["rebel"] = "wu";
        kingdoms["renegade"] = "qun";
    }
    if (kingdoms[role].isEmpty())
        return role;
    return kingdoms[role];
}

RoomThread::RoomThread(Room *room)
    : room(room)
{
    //Create GameRule inside the thread where RoomThread exists
    game_rule = new GameRule(this);
}

void RoomThread::addPlayerSkills(ServerPlayer *player, bool invoke_game_start) {
    QVariant void_data;
    bool invoke_verify = false;

    foreach(const TriggerSkill *skill, player->getTriggerSkills()) {
        addTriggerSkill(skill);

        if (invoke_game_start && skill->getTriggerEvents().contains(GameStart))
            invoke_verify = true;
    }

    //We should make someone trigger a whole GameStart event instead of trigger a skill only.
    if (invoke_verify)
        trigger(GameStart, room, player, void_data);
}

void RoomThread::constructTriggerTable() {
    foreach(ServerPlayer *player, room->getPlayers())
        addPlayerSkills(player, true);
}

void RoomThread::actionNormal(GameRule *game_rule) {
    try {
        forever{
            trigger(TurnStart, room, room->getCurrent());
            if (room->isFinished()) break;
            ServerPlayer *regular_next = qobject_cast<ServerPlayer *>(room->getCurrent()->getNextAlive(1, false));
            while (!room->getTag("ExtraTurnList").isNull()) {
                QStringList extraTurnList = room->getTag("ExtraTurnList").toStringList();
                if (!extraTurnList.isEmpty()) {
                    QString extraTurnPlayer = extraTurnList.takeFirst();
                    room->setTag("ExtraTurnList", QVariant::fromValue(extraTurnList));
                    ServerPlayer *next = room->findPlayer(extraTurnPlayer);
                    room->setCurrent(next);
                    trigger(TurnStart, room, next);
                    if (room->isFinished()) break;
                } else
                    room->removeTag("ExtraTurnList");
            }
            if (room->isFinished()) break;
            room->setCurrent(regular_next);
        }
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken)
            _handleTurnBrokenNormal(game_rule);
        else
            throw triggerEvent;
    }
}

void RoomThread::_handleTurnBrokenNormal(GameRule *game_rule) {
    try {
        ServerPlayer *player = room->getCurrent();
        trigger(TurnBroken, room, player);
        ServerPlayer *next = qobject_cast<ServerPlayer *>(player->getNextAlive(1, false));
        if (player->getPhase() != Player::NotActive) {
            QVariant _variant;
            game_rule->effect(EventPhaseEnd, room, player, _variant, player);
            player->changePhase(player->getPhase(), Player::NotActive);
        }

        room->setCurrent(next);
        actionNormal(game_rule);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken)
            _handleTurnBrokenNormal(game_rule);
        else
            throw triggerEvent;
    }
}

void RoomThread::run() {
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    Sanguosha->registerRoom(room);

    addTriggerSkill(game_rule);
    foreach(const TriggerSkill *triggerSkill, Sanguosha->getGlobalTriggerSkills())
        addTriggerSkill(triggerSkill);

    if (room->getScenario() != NULL) {
        const ScenarioRule *rule = room->getScenario()->getRule();
        if (rule) addTriggerSkill(rule);
    }

    QString winner = game_rule->getWinner(room->getPlayers().first());
    if (!winner.isNull()) {
        try {
            room->gameOver(winner);
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == GameFinished) {
                terminate();
                Sanguosha->unregisterRoom();
                return;
            } else
                Q_ASSERT(false);
        }
    }

    // start game
    try {
        trigger(GameStart, room, NULL);
        constructTriggerTable();
        actionNormal(game_rule);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == GameFinished) {
            Sanguosha->unregisterRoom();
            return;
        }
        else
            Q_ASSERT(false);
    }
}

static bool compareByPriority(const TriggerSkill *a, const TriggerSkill *b) {
    return a->getPriority() > b->getPriority();
}

bool RoomThread::trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data) {
    // push it to event stack
    EventTriplet triplet(triggerEvent, room, target);
    event_stack.push_back(triplet);

    bool broken = false;
    QList<const TriggerSkill *> will_trigger;
    QSet<const TriggerSkill *> triggerable_tested;
    QMap<ServerPlayer *, QList<const TriggerSkill *> > trigger_who;

    try {
        QList<const TriggerSkill *> triggered;
        QList<const TriggerSkill *> &skills = skill_table[triggerEvent];
        qStableSort(skills.begin(), skills.end(), compareByPriority);

        do {
            trigger_who.clear();
            foreach(const TriggerSkill *skill, skills) {
                if (!triggered.contains(skill)) {
                    if (skill->objectName() == "game_rule" || (room->getScenario()
                                                              && room->getScenario()->objectName() == skill->objectName())) {
                        room->tryPause();
                        if (will_trigger.isEmpty()
                            || skill->getPriority() == will_trigger.last()->getPriority()) {
                            will_trigger.append(skill);
                            trigger_who[target].append(skill);
                        }
                        else if (skill->getPriority() != will_trigger.last()->getPriority())
                            break;
                        triggered.prepend(skill);
                    }
                    else {
                        room->tryPause();
                        if (will_trigger.isEmpty()
                            || skill->getPriority() == will_trigger.last()->getPriority()) {
                            QMap<ServerPlayer *, QStringList> triggerSkillList = skill->triggerable(triggerEvent, room, target, data);
                            foreach(ServerPlayer *p, room->getPlayers()){
                                if (triggerSkillList.contains(p) && !triggerSkillList.value(p).isEmpty())
                                    foreach(QString skill_name, triggerSkillList.value(p)) {
                                    const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill_name);
                                    if (trskill) {
                                        will_trigger.append(trskill);
                                        trigger_who[p].append(trskill);
                                    }
                                }
                            }
                        }
                        else if (skill->getPriority() != will_trigger.last()->getPriority())
                            break;

                        triggered.prepend(skill);
                    }
                }
                triggerable_tested << skill;
            }

            if (!will_trigger.isEmpty()) {
                will_trigger.clear();
                foreach(ServerPlayer *p, room->getAllPlayers(true)) {
                    if (!trigger_who.contains(p)) continue;
                    QList<const TriggerSkill *> already_triggered;
                    forever{
                        QList<const TriggerSkill *> who_skills = trigger_who.value(p);
                        if (who_skills.isEmpty()) break;
                        bool has_compulsory = false;
                        foreach(const TriggerSkill *skill, who_skills){
                            if (skill->getFrequency() == Skill::Compulsory && p->hasShownSkill(skill)) {
                                has_compulsory = true;
                                break;
                            }
                        }
                        will_trigger.clear();
                        QStringList names, back_up;
                        QStringList _names;
                        foreach(const TriggerSkill *skill, who_skills) {
                            QString skill_name = skill->objectName();
                            _names.append(skill_name);
                            if (names.contains(skill_name))
                                back_up << skill_name;
                            else
                                names << skill_name;
                        }

                        if (names.isEmpty()) break;

                        QString name;
                        foreach (QString skillName, names) {
                            const TriggerSkill *skill = Sanguosha->getTriggerSkill(skillName);
                            if (skill && skill->isGlobal() && skill->getFrequency() == Skill::Compulsory) {
                                name = skillName; // a new trick to deal with all "record-skill" or "compulsory-global",
                                                  // they should always be triggered first.
                                break;
                            }
                        }
                        if (name.isEmpty()) {
                            if (p && !p->hasShownAllGenerals())
                                p->setFlags("Global_askForSkillCost");           // TriggerOrder need protect
                            if (names.contains("game_rule"))
                                name = "game_rule";
                            else if (names.contains("custom_scenario"))
                                name = "custom_scenario";
                            else if (names.length() == 1) {
                                name = names.first();
                                if (name.contains("AskForGeneralShow") && p != NULL) {
                                    SPlayerDataMap map;
                                    map[p] = names;
                                    name = room->askForTriggerOrder(p, "GameRule:TurnStart", map, true, data);
                                }
                            } else if (p != NULL) {
                                QString reason = "GameRule:TriggerOrder";
                                if (names.length() == 2 && names.contains("GameRule_AskForGeneralShowHead"))
                                    reason = "GameRule:TurnStart";
                                SPlayerDataMap map;
                                map[p] = names;
                                name = room->askForTriggerOrder(p, reason, map, !has_compulsory, data);
                            } else {
                                name = names.last();
                            }
                            if (p && p->hasFlag("Global_askForSkillCost"))
                                p->setFlags("-Global_askForSkillCost");
                        }

                        if (name == "cancel") break;
                        if (name.contains(":"))
                            name = name.split(":").last();

                        const TriggerSkill *skill = who_skills[_names.indexOf(name)];

                        //----------------------------------------------- TriggerSkill::cost
                        if (p && !p->hasShownSkill(skill))
                            p->setFlags("Global_askForSkillCost");           // SkillCost need protect
                        already_triggered.append(skill);
                        bool do_effect = false;
                        if (skill->cost(triggerEvent, room, target, data, p)) {
                            do_effect = true;
                            if (p && p->ownSkill(name) && !p->hasShownSkill(name))
                                p->showGeneral(p->inHeadSkills(name));
                        }
                        if (p && p->hasFlag("Global_askForSkillCost"))          // for next time
                            p->setFlags("-Global_askForSkillCost");
                        //-----------------------------------------------

                        //----------------------------------------------- TriggerSkill::effect
                        if (do_effect) {
                            broken = skill->effect(triggerEvent, room, target, data, p);
                            if (broken) break;
                        }
                        //-----------------------------------------------

                        trigger_who.clear();
                        foreach(const TriggerSkill *skill, triggered) {
                            if (skill->objectName() == "game_rule" || (room->getScenario()
                                                                       && room->getScenario()->objectName() == skill->objectName())) {
                                room->tryPause();
                                continue;
                            }
                            else {
                                room->tryPause();
                                if (skill->getPriority() == triggered.first()->getPriority()) {
                                    QMap<ServerPlayer *, QStringList> triggerSkillList = skill->triggerable(triggerEvent, room, target, data);
                                    foreach(ServerPlayer *player, room->getAllPlayers(true)){
                                        if (triggerSkillList.contains(player) && !triggerSkillList.value(player).isEmpty()){
                                            foreach(QString skill_name, triggerSkillList.value(player)) {
                                                const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill_name);
                                                if (trskill) {
                                                    trigger_who[player].append(trskill);
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                    break;
                            }
                        }

                        foreach(const TriggerSkill* s, already_triggered)
                            if (trigger_who[p].contains(s))
                                trigger_who[p].removeOne(s);

                        if (has_compulsory){
                            has_compulsory = false;
                            foreach(const TriggerSkill *s, trigger_who[p]){
                                if (s->getFrequency() == Skill::Compulsory && p->hasShownSkill(skill)) {
                                    has_compulsory = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (broken) break;
                }
                // @todo_Slob: for drawing cards when game starts -- stupid design of triggering no player!
                // @todo_Slob: we needn't judge the priority of game_rule because of codes from Line. 444 to Line. 481
                if (!broken) {
                    if (!trigger_who[NULL].isEmpty()){
                        foreach(const TriggerSkill *skill, trigger_who[NULL]){
                            if (skill->cost(triggerEvent, room, target, data, NULL)) {
                                broken = skill->effect(triggerEvent, room, target, data, NULL);
                                if (broken) break;
                            }
                        }
                    }
                }
            }

            if (broken) break;

        } while (skills.length() != triggerable_tested.size());

        if (target) {
            foreach(AI *ai, room->ais)
                ai->filterEvent(triggerEvent, target, data);
        }

        // pop event stack
        event_stack.pop_back();
    }
    catch (TriggerEvent throwed_event) {
        if (target) {
            foreach(AI *ai, room->ais)
                ai->filterEvent(triggerEvent, target, data);
        }

        // pop event stack
        event_stack.pop_back();

        throw throwed_event;
    }

    room->tryPause();
    return broken;
}

const QList<EventTriplet> *RoomThread::getEventStack() const{
    return &event_stack;
}

bool RoomThread::trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target) {
    QVariant data;
    return trigger(triggerEvent, room, target, data);
}

void RoomThread::addTriggerSkill(const TriggerSkill *skill) {
    if (skill == NULL || skillSet.contains(skill->objectName()))
        return;

    skillSet << skill->objectName();

    QList<TriggerEvent> events = skill->getTriggerEvents();
    foreach(TriggerEvent triggerEvent, events) {
        QList<const TriggerSkill *> &table = skill_table[triggerEvent];
        table << skill;
        qStableSort(table.begin(), table.end(), compareByPriority);
    }

    if (skill->isVisible()) {
        foreach(const Skill *skill, Sanguosha->getRelatedSkills(skill->objectName())) {
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            if (trigger_skill)
                addTriggerSkill(trigger_skill);
        }
    }
}

void RoomThread::delay(long secs) {
    if (secs == -1) secs = Config.AIDelay;
    Q_ASSERT(secs >= 0);
    if (room->property("to_test").toString().isEmpty() && Config.AIDelay > 0)
        msleep(secs);
}

