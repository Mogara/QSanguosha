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

#include "standard.h"
#include "room.h"
#include "skill.h"
#include "engine.h"

QString BasicCard::getType() const{
    return "basic";
}

Card::CardType BasicCard::getTypeId() const{
    return TypeBasic;
}

TrickCard::TrickCard(Suit suit, int number)
    : Card(suit, number), cancelable(true)
{
    handling_method = Card::MethodUse;
}

void TrickCard::setCancelable(bool cancelable) {
    this->cancelable = cancelable;
}

QString TrickCard::getType() const{
    return "trick";
}

Card::CardType TrickCard::getTypeId() const{
    return TypeTrick;
}

bool TrickCard::isCancelable(const CardEffectStruct &effect) const{
    Q_UNUSED(effect);
    return cancelable;
}

QString EquipCard::getType() const{
    return "equip";
}

Card::CardType EquipCard::getTypeId() const{
    return TypeEquip;
}

bool EquipCard::isAvailable(const Player *player) const{
    return !player->isProhibited(player, this) && Card::isAvailable(player);
}

void EquipCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;

    ServerPlayer *player = use.from;
    if (use.to.isEmpty())
        use.to << player;

    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, player, data);
    thread->trigger(CardUsed, room, player, data);
    thread->trigger(CardFinished, room, player, data);
}

void EquipCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if (targets.isEmpty()) {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName(), QString(), this->getSkillName(), QString());
        room->moveCardTo(this, room->getCardOwner(getEffectiveId()), NULL, Player::DiscardPile, reason, true);
    }
    int equipped_id = Card::S_UNKNOWN_CARD_ID;
    ServerPlayer *target = targets.first();
    if (target->getEquip(location()))
        equipped_id = target->getEquip(location())->getEffectiveId();

    QList<CardsMoveStruct> exchangeMove;
    CardsMoveStruct move1(getEffectiveId(), target, Player::PlaceEquip,
        CardMoveReason(CardMoveReason::S_REASON_USE, target->objectName()));
    exchangeMove.push_back(move1);
    if (equipped_id != Card::S_UNKNOWN_CARD_ID) {
        CardsMoveStruct move2(equipped_id, target, Player::PlaceTable,
            CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, target->objectName()));
        exchangeMove.push_back(move2);
    }
    room->moveCardsAtomic(exchangeMove, true);

    LogMessage log;
    log.from = target;
    log.type = "$Install";
    log.card_str = QString::number(getEffectiveId());
    room->sendLog(log);

    if (equipped_id != Card::S_UNKNOWN_CARD_ID) {
        if (room->getCardPlace(equipped_id) == Player::PlaceTable) {
            CardsMoveStruct move3(equipped_id, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, target->objectName()));
            room->moveCardsAtomic(move3, true);
        }
    }
}

void EquipCard::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    const Skill *skill = Sanguosha->getSkill(this);
    if (skill) {
        if (skill->inherits("ViewAsSkill")) {
            room->attachSkillToPlayer(player, this->objectName());
        } else if (skill->inherits("TriggerSkill")) {
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            room->getThread()->addTriggerSkill(trigger_skill);
        }
    }
}

void EquipCard::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    if (Sanguosha->getSkill(this) && Sanguosha->getSkill(this)->inherits("ViewAsSkill"))
        room->detachSkillFromPlayer(player, this->objectName(), true);
}

QString GlobalEffect::getSubtype() const{
    return "global_effect";
}

void GlobalEffect::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *source = card_use.from;
    QList<ServerPlayer *> targets;
    if (card_use.to.isEmpty()) {
        QList<ServerPlayer *> all_players = room->getAllPlayers();
        foreach (ServerPlayer *player, all_players) {
            const Skill *skill = room->isProhibited(source, player, this);
            if (skill) {
                if (!skill->isVisible())
                    skill = Sanguosha->getMainSkill(skill->objectName());
                if (skill->isVisible()) {
                    LogMessage log;
                    log.type = "#SkillAvoid";
                    log.from = player;
                    log.arg = skill->objectName();
                    log.arg2 = objectName();
                    room->sendLog(log);

                    room->broadcastSkillInvoke(skill->objectName(), player);
                }
            } else {
                targets << player;
            }
        }
    } else {
        targets = card_use.to;
    }

    CardUseStruct use = card_use;
    use.to = targets;
    TrickCard::onUse(room, use);
}

bool GlobalEffect::isAvailable(const Player *player) const{
    bool canUse = false;
    QList<const Player *> players = player->getAliveSiblings();
    players << player;
    foreach(const Player *p, players) {
        if (player->isProhibited(p, this))
            continue;

        canUse = true;
        break;
    }

    return canUse && TrickCard::isAvailable(player);
}

QString AOE::getSubtype() const{
    return "aoe";
}

bool AOE::isAvailable(const Player *player) const{
    bool canUse = false;
    QList<const Player *> players = player->getAliveSiblings();
    foreach(const Player *p, players) {
        if (player->isProhibited(p, this))
            continue;

        canUse = true;
        break;
    }

    return canUse && TrickCard::isAvailable(player);
}

void AOE::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *source = card_use.from;
    QList<ServerPlayer *> targets;
    if (card_use.to.isEmpty()) {
        QList<ServerPlayer *> other_players = room->getOtherPlayers(source);
        foreach (ServerPlayer *player, other_players) {
            const Skill *skill = room->isProhibited(source, player, this);
            if (skill) {
                if (!skill->isVisible())
                    skill = Sanguosha->getMainSkill(skill->objectName());
                if (skill->isVisible()) {
                    LogMessage log;
                    log.type = "#SkillAvoid";
                    log.from = player;
                    log.arg = skill->objectName();
                    log.arg2 = objectName();
                    room->sendLog(log);

                    room->broadcastSkillInvoke(skill->objectName(), player);
                }
            } else {
                targets << player;
            }
        }
    } else {
        targets = card_use.to;
    }

    CardUseStruct use = card_use;
    use.to = targets;
    TrickCard::onUse(room, use);
}

QString SingleTargetTrick::getSubtype() const{
    return "single_target_trick";
}

bool SingleTargetTrick::targetFilter(const QList<const Player *> &, const Player *, const Player *) const{
    return true;
}

DelayedTrick::DelayedTrick(Suit suit, int number, bool movable)
    : TrickCard(suit, number), movable(movable)
{
    judge.negative = true;
}

void DelayedTrick::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    WrappedCard *wrapped = Sanguosha->getWrappedCard(this->getEffectiveId());
    use.card = wrapped;

    LogMessage log;
    log.from = use.from;
    log.to = use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, use.from, data);

    CardMoveReason reason(CardMoveReason::S_REASON_USE, use.from->objectName(), use.to.first()->objectName(), getSkillName(), QString());
    room->moveCardTo(this, use.from, use.to.first(), Player::PlaceDelayedTrick, reason, true);

    QString skill_name = wrapped->showSkill();
    if (!skill_name.isNull() && use.from->ownSkill(skill_name) && !use.from->hasShownSkill(skill_name))
        use.from->showGeneral(use.from->inHeadSkills(skill_name));

    thread->trigger(CardUsed, room, use.from, data);
    thread->trigger(CardFinished, room, use.from, data);
}

void DelayedTrick::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if (targets.isEmpty()) {
        if (movable) {
            onNullified(source);
            if (room->getCardOwner(getEffectiveId()) != source) return;
        }
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName(), QString(), this->getSkillName(), QString());
        room->moveCardTo(this, NULL, Player::DiscardPile, reason, true);
    }
}

QString DelayedTrick::getSubtype() const{
    return "delayed_trick";
}

void DelayedTrick::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    CardMoveReason reason(CardMoveReason::S_REASON_USE, effect.to->objectName(), getSkillName(), QString());
    room->moveCardTo(this, NULL, Player::PlaceTable, reason, true);

    LogMessage log;
    log.from = effect.to;
    log.type = "#DelayedTrick";
    log.arg = effect.card->objectName();
    room->sendLog(log);

    JudgeStruct judge_struct = judge;
    judge_struct.who = effect.to;
    room->judge(judge_struct);

    if (judge_struct.isBad()) {
        takeEffect(effect.to);
        if (room->getCardOwner(getEffectiveId()) == NULL) {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString());
            room->throwCard(this, reason, NULL);
        }
    }
    else if (movable) {
        onNullified(effect.to);
    }
    else {
        if (room->getCardOwner(getEffectiveId()) == NULL) {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString());
            room->throwCard(this, reason, NULL);
        }
    }
}

void DelayedTrick::onNullified(ServerPlayer *target) const{
    Room *room = target->getRoom();
    RoomThread *thread = room->getThread();
    if (movable) {
        QList<ServerPlayer *> players;
        QList<ServerPlayer *> count_players = room->getPlayers();
        ServerPlayer *starter = target;
        int index = count_players.indexOf(starter);
        for (int i = index + 1; i < count_players.length(); i++) {
            if (count_players[i]->isAlive())
                players << count_players[i];
        }

        for (int i = 0; i <= index; i++) {
            if (count_players[i]->isAlive())
                players << count_players[i];
        }

        ServerPlayer *p = NULL;

        foreach(ServerPlayer *player, players) {
            if (player->containsTrick(objectName()))
                continue;

            const ProhibitSkill *skill = room->isProhibited(target, player, this);
            if (skill) {
                LogMessage log;
                log.type = "#SkillAvoid";
                log.from = player;
                log.arg = skill->objectName();
                log.arg2 = objectName();
                room->sendLog(log);

                room->broadcastSkillInvoke(skill->objectName(), player);
                continue;
            }

            CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, target->objectName(), QString(), this->getSkillName(), QString());
            room->moveCardTo(this, target, player, Player::PlaceDelayedTrick, reason, true);

            if (target == player) break;

            CardUseStruct use(this, NULL, player);
            QVariant data = QVariant::fromValue(use);
            thread->trigger(TargetConfirming, room, player, data);
            CardUseStruct new_use = data.value<CardUseStruct>();
            if (new_use.to.isEmpty()) {
                p = player;
                break;
            }

            foreach(ServerPlayer *p, room->getAllPlayers())
                thread->trigger(TargetChosen, room, p, data);
            foreach(ServerPlayer *p, room->getAllPlayers())
                thread->trigger(TargetConfirmed, room, p, data);
            break;
        }
        if (p)
            onNullified(p);
    }
    else {
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, target->objectName());
        room->throwCard(this, reason, NULL);
    }
}

Weapon::Weapon(Suit suit, int number, int range)
    : EquipCard(suit, number), range(range)
{
}

int Weapon::getRange() const{
    return range;
}

QString Weapon::getSubtype() const{
    return "weapon";
}

EquipCard::Location Weapon::location() const{
    return WeaponLocation;
}

QString Weapon::getCommonEffectName() const{
    return "weapon";
}

QString Armor::getSubtype() const{
    return "armor";
}

EquipCard::Location Armor::location() const{
    return ArmorLocation;
}

QString Armor::getCommonEffectName() const{
    return "armor";
}

Horse::Horse(Suit suit, int number, int correct)
    : EquipCard(suit, number), correct(correct)
{
}

int Horse::getCorrect() const{
    return correct;
}

void Horse::onInstall(ServerPlayer *) const{
}

void Horse::onUninstall(ServerPlayer *) const{
}

QString Horse::getCommonEffectName() const{
    return "horse";
}

OffensiveHorse::OffensiveHorse(Card::Suit suit, int number, int correct)
    : Horse(suit, number, correct)
{
}

QString OffensiveHorse::getSubtype() const{
    return "offensive_horse";
}

DefensiveHorse::DefensiveHorse(Card::Suit suit, int number, int correct)
    : Horse(suit, number, correct)
{
}

QString DefensiveHorse::getSubtype() const{
    return "defensive_horse";
}

EquipCard::Location Horse::location() const{
    if (correct > 0)
        return DefensiveHorseLocation;
    else
        return OffensiveHorseLocation;
}

QString Treasure::getSubtype() const{
    return "treasure";
}

EquipCard::Location Treasure::location() const{
    return TreasureLocation;
}

QString Treasure::getCommonEffectName() const{
    return "treasure";
}
