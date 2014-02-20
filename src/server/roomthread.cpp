#include "roomthread.h"
#include "room.h"
#include "engine.h"
#include "gamerule.h"
#include "scenerule.h"
#include "scenario.h"
#include "ai.h"
#include "jsonutils.h"
#include "settings.h"
#include "standard.h"

#include <QTime>
#include <json/json.h>

#ifdef QSAN_UI_LIBRARY_AVAILABLE
#pragma message WARN("UI elements detected in server side!!!")
#endif

using namespace QSanProtocol::Utils;

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

Json::Value LogMessage::toJsonValue() const{
    QStringList tos;
    foreach (ServerPlayer *player, to)
        if (player != NULL) tos << player->objectName();

    QStringList log;
    log << type << (from ? from->objectName() : "") << tos.join("+") << card_str << arg << arg2;
    Json::Value json_log = QSanProtocol::Utils::toJsonArray(log);
    return json_log;
}

DamageStruct::DamageStruct()
    : from(NULL), to(NULL), card(NULL), damage(1), nature(Normal), chain(false), transfer(false), by_user(true), reason(QString())
{
}

DamageStruct::DamageStruct(const Card *card, ServerPlayer *from, ServerPlayer *to, int damage, DamageStruct::Nature nature)
    : chain(false), transfer(false), by_user(true), reason(QString())
{
    this->card = card;
    this->from = from;
    this->to = to;
    this->damage = damage;
    this->nature = nature;
}

DamageStruct::DamageStruct(const QString &reason, ServerPlayer *from, ServerPlayer *to, int damage, DamageStruct::Nature nature)
    : card(NULL), chain(false), transfer(false), by_user(true)
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

bool CardUseStruct::tryParse(const Json::Value &usage, Room *room) {
    if (usage.size() < 2 || !usage[0].isString() || !usage[1].isArray())
        return false;

    card = Card::Parse(toQString(usage[0]));
    const Json::Value &targets = usage[1];

    for (unsigned int i = 0; i < targets.size(); i++) {
        if (!targets[i].isString()) return false;
        this->to << room->findChild<ServerPlayer *>(toQString(targets[i]));
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
        foreach (QString target_name, target_names)
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

RoomThread::RoomThread(Room *room)
    : room(room)
{
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
        forever {
            trigger(TurnStart, room, room->getCurrent());
            if (room->isFinished()) break;
            room->setCurrent(qobject_cast<ServerPlayer *>(room->getCurrent()->getNextAlive()));
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
        ServerPlayer *next = qobject_cast<ServerPlayer *>(player->getNextAlive());
        if (player->getPhase() != Player::NotActive) {
            QVariant _variant;
            game_rule->effect(EventPhaseEnd, room, player, _variant);
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
    GameRule *game_rule = new GameRule(this);

    addTriggerSkill(game_rule);
    foreach (const TriggerSkill *triggerSkill, Sanguosha->getGlobalTriggerSkills())
        addTriggerSkill(triggerSkill);

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
        trigger(GameStart, (Room *)room, NULL);
        constructTriggerTable();
        actionNormal(game_rule);
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

static bool CompareByPriority(const TriggerSkill *a, const TriggerSkill *b) {
    if (a->getDynamicPriority() == b->getDynamicPriority())
        return b->inherits("WeaponSkill") || b->inherits("ArmorSkill") || b->inherits("GameRule");
    return a->getDynamicPriority() > b->getDynamicPriority();
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
        foreach (const TriggerSkill *skill, skills) {
            double priority = skill->getPriority();
            int len = room->getPlayers().length();
            foreach (ServerPlayer *p, room->getAllPlayers(true)) {
                if (p->hasSkill(skill->objectName())) {
                    priority += (double)len / 100;
                    break;
                }
                len--;
            }
            TriggerSkill *mutable_skill = const_cast<TriggerSkill *>(skill);
            mutable_skill->setDynamicPriority(priority);
        }
        qStableSort(skills.begin(), skills.end(), CompareByPriority);

        do {
            trigger_who.clear();
            foreach (const TriggerSkill *skill, skills) {
                ServerPlayer *ask_who = target;
                if (!triggered.contains(skill)) {
                    if (skill->objectName() == "game_rule") {
                        while (room->isPaused()) {}
                        if (will_trigger.isEmpty()
                                || skill->getDynamicPriority() == will_trigger.last()->getDynamicPriority()) {
                            will_trigger.append(skill);
                            trigger_who[ask_who].append(skill);
                        } else if(skill->getDynamicPriority() != will_trigger.last()->getDynamicPriority())
                            break;
                        triggered.prepend(skill);
                    } else {
                        while (room->isPaused()) {}
                        if (will_trigger.isEmpty()
                                || skill->getDynamicPriority() == will_trigger.last()->getDynamicPriority()) {
                            QStringList triggerSkillList = skill->triggerable(triggerEvent, room, target, data, ask_who);
                            if (!triggerSkillList.isEmpty()) {
                                foreach (QString skill_name, triggerSkillList) {
                                    const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill_name);
                                    if (trskill) {
                                        will_trigger.append(trskill);
                                        trigger_who[ask_who].append(trskill);
                                    }
                                }
                            }
                        } else if(skill->getDynamicPriority() != will_trigger.last()->getDynamicPriority())
                            break;

                        triggered.prepend(skill);
                    }
                }
                triggerable_tested << skill;
            }

            if (!will_trigger.isEmpty()) {
                will_trigger.clear();
                foreach(ServerPlayer *p, trigger_who.keys()) {
                    QList<const TriggerSkill *> who_skills = trigger_who.value(p);
                    QList<const TriggerSkill *> already_triggered;
                    if (who_skills.isEmpty()) continue;
                    if (p && !p->hasShownAllGenerals())
                        room->setPlayerFlag(p, "Global_askForSkillCost");           // TriggerOrder need protect
                    bool has_compulsory = false;
                    foreach (const TriggerSkill *skill, who_skills)
                        if (skill->getFrequency() == Skill::Compulsory) {
                            has_compulsory = true;
                            break;
                        }

                    forever {
                        will_trigger.clear();
                        QStringList names, back_up;
                        QStringList _names;
                        foreach (const TriggerSkill *skill, who_skills) {
                            QString name = skill->objectName();
                            _names.append(name);
                            if (names.contains(name))
                                back_up << name;
                            else
                                names << name;
                        }

                        if (names.isEmpty()) break;

                        if ((names.length() > 1 || !back_up.isEmpty()) && !has_compulsory)
                            names << "trigger_none";

                        QString name;
                        if (p != NULL)
                            name = room->askForChoice(p, "TriggerOrder", names.join("+"), data);
                        else
                            name = names.first();
                        if (name == "trigger_none") break;
                        const TriggerSkill *skill = who_skills[_names.indexOf(name)];

                        //----------------------------------------------- TriggerSkill::cost
                        if (skill->isGlobal() || (p && p->ownSkill(name) && p->hasShownSkill(Sanguosha->getSkill(name)))) // if hasShown, then needn't protect
                            if (p && p->hasFlag("Global_askForSkillCost"))
                                room->setPlayerFlag(p, "-Global_askForSkillCost");
                        bool do_effect = false;
                        if (skill->cost(triggerEvent, room, target, data)) {
                            do_effect = true;
                            if (p && p->ownSkill(name) && !p->hasShownSkill(Sanguosha->getSkill(name)))
                                p->showGeneral(p->inHeadSkills(name));
                        }
                        if (p && !p->hasFlag("Global_askForSkillCost") && !p->hasShownAllGenerals())          // for next time
                            room->setPlayerFlag(p, "Global_askForSkillCost");
                        already_triggered.append(skill);
                        //-----------------------------------------------

                        //----------------------------------------------- TriggerSkill::effect
                        if (do_effect) {
                            broken = skill->effect(triggerEvent, room, target, data);
                                if (broken) break;
                        }
                        //-----------------------------------------------

                        who_skills.clear();
                        foreach (const TriggerSkill *skill, triggered) {
                            ServerPlayer *ask_who = target;
                            if (skill->objectName() == "game_rule") {
                                while (room->isPaused()) {}
                                if (skill->getDynamicPriority() == triggered.first()->getDynamicPriority()) {
                                    if (ask_who == p)
                                        who_skills.append(skill);
                                } else
                                    break;
                            } else {
                                while (room->isPaused()) {}
                                if (skill->getDynamicPriority() == triggered.first()->getDynamicPriority()) {
                                    QStringList triggerSkillList = skill->triggerable(triggerEvent, room, target, data, ask_who);
                                    if (!triggerSkillList.isEmpty()) {
                                        foreach (QString skill_name, triggerSkillList) {
                                            const TriggerSkill *trskill = Sanguosha->getTriggerSkill(skill_name);
                                            if (trskill) {
                                                will_trigger.append(skill);
                                                if (ask_who == p)
                                                    who_skills.append(trskill);
                                            }
                                        }
                                    }
                                } else
                                    break;
                            }
                        }

                        foreach (const TriggerSkill* s, already_triggered)
                            if (who_skills.contains(s))
                                who_skills.removeOne(s);

                        if (has_compulsory){
                            has_compulsory = false;
                            foreach (const TriggerSkill *s, who_skills){
                                if (s->getFrequency() == Skill::Compulsory){
                                    has_compulsory = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (p && p->hasFlag("Global_askForSkillCost"))
                        room->setPlayerFlag(p, "-Global_askForSkillCost"); // remove Flag

                    if (broken) break;
                }
            }

            if (broken) break;

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

    while (room->isPaused()) {}
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
    foreach (TriggerEvent triggerEvent, events) {
        QList<const TriggerSkill *> &table = skill_table[triggerEvent];
        table << skill;
        foreach (const TriggerSkill *askill, table) {
            double priority = askill->getPriority();
            int len = room->getPlayers().length();
            foreach (ServerPlayer *p, room->getAllPlayers(true)) {
                if (p->hasSkill(askill->objectName())) {
                    priority += (double)len / 100;
                    break;
                }
                len--;
            }
            TriggerSkill *mutable_skill = const_cast<TriggerSkill *>(askill);
            mutable_skill->setDynamicPriority(priority);
        }
        qStableSort(table.begin(), table.end(), CompareByPriority);
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
    if (secs == -1) secs = Config.AIDelay;
    Q_ASSERT(secs >= 0);
    if (room->property("to_test").toString().isEmpty() && Config.AIDelay > 0)
        msleep(secs);
}

