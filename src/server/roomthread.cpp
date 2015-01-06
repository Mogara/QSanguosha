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
#include "structs.h"

#include <QTime>

#ifdef QSAN_UI_LIBRARY_AVAILABLE
#pragma message WARN("UI elements detected in server side!!!")
#endif

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

    foreach (const TriggerSkill *skill, player->getTriggerSkills()) {
        addTriggerSkill(skill);

        if (invoke_game_start && skill->getTriggerEvents().contains(GameStart))
            invoke_verify = true;
    }

    //We should make someone trigger a whole GameStart event instead of trigger a skill only.
    if (invoke_verify)
        trigger(GameStart, room, player, void_data);
}

void RoomThread::constructTriggerTable() {
    foreach (ServerPlayer *player, room->getPlayers())
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
    foreach (const TriggerSkill *triggerSkill, Sanguosha->getGlobalTriggerSkills())
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
        // delay(3000);
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
    QList<const TriggerSkill *> rules; // we can't get a GameRule with Engine::getTriggerSkill() :(
    TriggerList trigger_who;

    try {
        QList<const TriggerSkill *> triggered;
        QList<const TriggerSkill *> &skills = skill_table[triggerEvent];
        qStableSort(skills.begin(), skills.end(), compareByPriority);

        do {
            trigger_who.clear();
            foreach (const TriggerSkill *skill, skills) {
                if (!triggered.contains(skill)) {
                    if (skill->objectName() == "game_rule" || (room->getScenario()
                                                              && room->getScenario()->objectName() == skill->objectName())) {
                        room->tryPause();
                        if (will_trigger.isEmpty()
                            || skill->getPriority() == will_trigger.last()->getPriority()) {
                            will_trigger.append(skill);
                            trigger_who[NULL].append(skill->objectName());// Don't assign game rule to some player.
                            rules.append(skill);
                        } else if (skill->getPriority() != will_trigger.last()->getPriority())
                            break;
                        triggered.prepend(skill);
                    } else {
                        room->tryPause();
                        if (will_trigger.isEmpty()
                            || skill->getPriority() == will_trigger.last()->getPriority()) {
                            TriggerList triggerSkillList = skill->triggerable(triggerEvent, room, target, data);
                            foreach (ServerPlayer *p, room->getPlayers()) {
                                if (triggerSkillList.contains(p) && !triggerSkillList.value(p).isEmpty()) {
                                    foreach (const QString &skill_name, triggerSkillList.value(p)) {
                                        const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill_name);
                                        if (trskill) {
                                            will_trigger.append(trskill);
                                            trigger_who[p].append(skill_name);
                                        } else {
                                            trskill = Sanguosha->getTriggerSkill(skill_name.split("!").first());
                                            if (trskill) {
                                                will_trigger.append(trskill);
                                                trigger_who[p].append(skill_name);
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (skill->getPriority() != will_trigger.last()->getPriority())
                            break;

                        triggered.prepend(skill);
                    }
                }
                triggerable_tested << skill;
            }

            if (!will_trigger.isEmpty()) {
                will_trigger.clear();
                foreach (ServerPlayer *p, room->getPlayers()) {
                    if (!trigger_who.contains(p)) continue;
                    QStringList already_triggered;
                    forever {
                        QStringList who_skills = trigger_who.value(p);
                        if (who_skills.isEmpty()) break;
                        bool has_compulsory = false;
                        foreach (const QString &skill, who_skills){
                            const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill);
                            if (!trskill)
                                trskill = Sanguosha->getTriggerSkill(skill.split("!").first());
                            if (trskill && p->hasShownSkill(skill)
                                && (trskill->getFrequency() == Skill::Compulsory
                                    //|| trskill->getFrequency() == Skill::NotCompulsory //for Paoxia, Anjian, etc.
                                    || trskill->getFrequency() == Skill::Wake)) {
                                has_compulsory = true;
                                break;
                            }
                        }
                        will_trigger.clear();
                        QStringList names, back_up;
                        QStringList _names;
                        foreach (const QString &skill_name, who_skills) {
                            if (skill_name.contains("!") && skill_name.endsWith("&")) {
                                QString realSkillName = skill_name.split("!").first();
                                QStringList targets = skill_name.split("!").last().remove("&").split("+");
                                int index = 1; // this index is for UI only
                                bool cannotSkip = false;
                                const TriggerSkill *trskill = Sanguosha->getTriggerSkill(realSkillName);
                                if (trskill && p->hasShownSkill(trskill)
                                    && (trskill->getFrequency() == Skill::Compulsory
                                        //|| trskill->getFrequency() == Skill::NotCompulsory //for Paoxia, Anjian, etc.
                                        || trskill->getFrequency() == Skill::Wake))
                                    cannotSkip = true;
                                foreach (const QString &target, targets) {
                                    QString name = QString("%1!%2&%3").arg(realSkillName).arg(target).arg(index);
                                    _names.append(name);
                                    if (names.contains(name))
                                        back_up << name;
                                    else
                                        names << name;
                                    if (cannotSkip)
                                        break;
                                    ++index;
                                }
                            } else {
                                _names.append(skill_name);
                                if (names.contains(skill_name))
                                    back_up << skill_name;
                                else
                                    names << skill_name;
                            }
                        }

                        if (names.isEmpty()) break;

                        QString name;
                        foreach (const QString &skillName, names) {
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
                            if (names.length() == 1 && back_up.isEmpty()) {
                                // users should be able to cancel if it triggers someskill 100 times.
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
                                foreach (const QString &skillName, names) {
                                    if (skillName.contains("!") && !skillName.contains("&")) {
                                        ServerPlayer *owner = room->findPlayer(skillName.split("!").last());
                                        map[owner] << skillName;
                                    } else
                                        map[p] << skillName;
                                }
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
                        
                        ServerPlayer *skill_target = target;
                        //QString result_skill_name = who_skills[_names.indexOf(name)];
                        QString result_skill_name = name;
                        const TriggerSkill *result_skill = Sanguosha->getTriggerSkill(result_skill_name);
                        if (!result_skill) {
                            QStringList list = result_skill_name.split("!");
                            result_skill = Sanguosha->getTriggerSkill(list.first());
                            skill_target = room->findPlayer(list.last().split("&").first());
                        }

                        //----------------------------------------------- TriggerSkill::cost
                        if (p && !p->hasShownSkill(result_skill))
                            p->setFlags("Global_askForSkillCost");           // SkillCost need protect
                        already_triggered.append(result_skill_name);
                        bool do_effect = false;
                        if (result_skill->cost(triggerEvent, room, skill_target, data, p)) {
                            do_effect = true;
                            if (p && p->ownSkill(result_skill) && !p->hasShownSkill(result_skill))
                                p->showGeneral(p->inHeadSkills(result_skill));
                        }
                        if (p && p->hasFlag("Global_askForSkillCost"))          // for next time
                            p->setFlags("-Global_askForSkillCost");
                        //-----------------------------------------------

                        //----------------------------------------------- TriggerSkill::effect
                        if (do_effect) {
                            broken = result_skill->effect(triggerEvent, room, skill_target, data, p);
                            if (broken)
                                break;
                        }
                        //-----------------------------------------------

                        trigger_who.clear();
                        foreach (const TriggerSkill *skill, triggered) {
                            if (skill->objectName() == "game_rule" || (room->getScenario()
                                                                       && room->getScenario()->objectName() == skill->objectName())) {
                                room->tryPause();
                                continue; // dont assign them to some person.
                            } else {
                                room->tryPause();
                                if (skill->getPriority() == triggered.first()->getPriority()) {
                                    TriggerList triggerSkillList = skill->triggerable(triggerEvent, room, target, data);
                                    foreach (ServerPlayer *player, room->getAllPlayers(true)){
                                        if (triggerSkillList.contains(player) && !triggerSkillList.value(player).isEmpty()){
                                            foreach (const QString &skill_name, triggerSkillList.value(player)) {
                                                const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill_name);
                                                if (trskill) {
                                                    trigger_who[player].append(skill_name);
                                                } else {
                                                    trskill = Sanguosha->getTriggerSkill(skill_name.split("!").first());
                                                    if (trskill)
                                                        trigger_who[player].append(skill_name);
                                                }
                                            }
                                        }
                                    }
                                } else
                                    break;
                            }
                        }

                        foreach (const QString &s, already_triggered) {
                            if (s.contains("&")) {
                                // s is "skillName!targetObjectName&number"
                                QString skillName_copy = s.split("!").first();
                                QString triggered_target = s.split("!").last().split("&").first();
                                foreach (const QString &skill, trigger_who[p]) {
                                    if (skill.contains("!") && skill.endsWith("&")) {
                                        if (skill.split("!").first() == skillName_copy) { // check skill
                                            QStringList targets = skill.split("!").last().remove("&").split("+");
                                            if (!targets.contains(triggered_target)) { // this skill is triggered.
                                                continue;
                                            } else {
                                                int n = targets.indexOf(triggered_target);
                                                if (n == targets.length() - 1)
                                                    trigger_who[p].removeOne(skill);
                                                else {
                                                    QStringList targets_after = targets.mid(n + 1);
                                                    QString skill_after = QString("%1!%2&").arg(skillName_copy).arg(targets_after.join("+"));
                                                    trigger_who[p].replaceInStrings(skill, skill_after);
                                                }
                                            }
                                        } else {
                                            continue;
                                        }
                                    }
                                }
                            } else {
                                if (trigger_who[p].contains(s))
                                    trigger_who[p].removeOne(s);
                            }
                        }

                        if (has_compulsory) {
                            has_compulsory = false;
                            foreach (const QString &skillName, trigger_who[p]) {
                                const TriggerSkill *s = Sanguosha->getTriggerSkill(skillName);
                                if (!s)
                                    s = Sanguosha->getTriggerSkill(skillName.split("!").first());
                                if (s && p->hasShownSkill(s)
                                    && (s->getFrequency() == Skill::Compulsory
                                        //|| s->getFrequency() == Skill::NotCompulsory // for Paoxiao, Anjian, etc.
                                        || s->getFrequency() == Skill::Wake)) {
                                    has_compulsory = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (broken) break;
                }
                // @todo_Slob: for drawing cards when game starts -- stupid design of triggering no player!
                if (!broken) {
                    if (!trigger_who[NULL].isEmpty()) {
                        foreach (QString skill_name, trigger_who[NULL]) {
                            const TriggerSkill *skill;
                            foreach (const TriggerSkill *rule, rules) { // because we cannot get a GameRule with Engine::getTriggerSkill()
                                if (rule->objectName() == skill_name) {
                                    skill = rule;
                                    break;
                                }
                            }
                            Q_ASSERT(skill);
                            if (skill->cost(triggerEvent, room, target, data, NULL)) {
                                broken = skill->effect(triggerEvent, room, target, data, NULL);
                                if (broken)
                                    break;
                            }
                        }
                    }
                }
            }

            if (broken)
                break;
        } while (skills.length() != triggerable_tested.size());

        if (target) {
            foreach (AI *ai, room->ais)
                ai->filterEvent(triggerEvent, target, data);
        }

        // pop event stack
        event_stack.pop_back();
    }
    catch (TriggerEvent throwed_event) {
        if (target) {
            foreach (AI *ai, room->ais)
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
    foreach (const TriggerEvent &triggerEvent, events) {
        QList<const TriggerSkill *> &table = skill_table[triggerEvent];
        table << skill;
        qStableSort(table.begin(), table.end(), compareByPriority);
    }

    if (skill->isVisible()) {
        foreach (const Skill *skill, Sanguosha->getRelatedSkills(skill->objectName())) {
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            if (trigger_skill)
                addTriggerSkill(trigger_skill);
        }
    }
}

void RoomThread::delay(long secs) {
    if (secs == -1) secs = room->config.AIDelay;
    Q_ASSERT(secs >= 0);
    if (room->property("to_test").toString().isEmpty() && room->config.AIDelay > 0)
        msleep(secs);
}

