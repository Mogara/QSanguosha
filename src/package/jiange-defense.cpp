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

#include "jiange-defense.h"
#include "skill.h"
#include "engine.h"
#include "standard-tricks.h"
#include "standard-basics.h"

class JGJizhen : public PhaseChangeSkill{
public:
    JGJizhen() : PhaseChangeSkill("jgjizhen") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish) {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->isFriendWith(player) && p->isWounded())
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = target;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(target, objectName());

        QList<ServerPlayer *> draw_list;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (p->isFriendWith(target) && p->isWounded()) {
                room->doAnimate(QSanProtocol::S_ALL_ALIVE_PLAYERS, target->objectName(), p->objectName());
                draw_list << p;
            }
        }
        room->sortByActionOrder(draw_list);
        room->drawCards(draw_list, 1);
        return false;
    }
};

class JGLingfeng : public PhaseChangeSkill{
public:
    JGLingfeng() : PhaseChangeSkill("jglingfeng") {
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player) && player->getPhase() == Player::Draw;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        QList<int> cardids = room->getNCards(2, false);
        CardsMoveStruct move(cardids, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_SHOW, target->objectName(), objectName(), QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        DummyCard dummy(cardids);
        room->obtainCard(target, &dummy);

        if (Sanguosha->getCard(cardids.first())->getColor() != Sanguosha->getCard(cardids.last())->getColor()) {
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *p, room->getOtherPlayers(target)) {
                if (!p->isFriendWith(target))
                    players << p;
            }
            if (players.isEmpty())
                return true;

            ServerPlayer *victim = room->askForPlayerChosen(target, players, objectName(), "@jglingfeng");
            if (victim == NULL)
                victim = players.at(qrand() % players.length());

            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, target->objectName(), victim->objectName());
            room->loseHp(victim, 1);
        }

        return true;
    }
};

class JGBiantian : public TriggerSkill{
public:
    JGBiantian() : TriggerSkill("jgbiantian") {
        frequency = Compulsory;
        events << EventPhaseStart << Death << EventLoseSkill << FinishJudge;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == EventPhaseStart && TriggerSkill::triggerable(player)) {
            if (player->getPhase() == Player::RoundStart) {
                foreach(ServerPlayer *p, room->getAlivePlayers()) {
                    p->loseAllMarks("@gale");
                    p->loseAllMarks("@fog");
                }
            }
            else if (player->getPhase() == Player::Start)
                return QStringList(objectName());
        }
        else if ((player != NULL && (triggerEvent == Death && player->hasSkill(objectName()) && data.value<DeathStruct>().who == player))
            || (triggerEvent == EventLoseSkill && data.toString() == objectName())) {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                p->loseAllMarks("@gale");
                p->loseAllMarks("@fog");
            }
        }
        else if (triggerEvent == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == objectName())
                judge->pattern = judge->card->isRed() ? "r" : "b";
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());

        JudgeStruct judge;
        judge.play_animation = false;
        judge.pattern = ".";
        judge.good = true;
        judge.reason = objectName();
        judge.who = player;

        room->judge(judge);

        if (judge.pattern == "r") {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (!p->isFriendWith(player)) {
                    room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                    p->gainMark("@gale", 1);
                }
            }
        }
        else if (judge.pattern == "b") {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->isFriendWith(player)) {
                    room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                    p->gainMark("@fog", 1);
                }
            }
        }
        else
            Q_ASSERT(false);

        return false;
    }
};

class JGBiantianKF : public TriggerSkill{
public:
    JGBiantianKF() : TriggerSkill("#jgbiantian-kf") {
        frequency = Compulsory;
        events << DamageForseen;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        ServerPlayer *zhuge = room->findPlayerBySkillName("jgbiantian");
        if (zhuge == NULL)
            return QStringList();

        ask_who = zhuge;

        DamageStruct damage = data.value<DamageStruct>();
        return (player != NULL && player->getMark("@gale") > 0 && damage.nature == DamageStruct::Fire) ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        return ask_who->hasShownSkill("jgbiantian");
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#GalePower";
        log.from = ask_who;
        log.to << player;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);

        data = QVariant::fromValue(damage);

        return false;
    }
};

class JGBiantianDW : public TriggerSkill{
public:
    JGBiantianDW() : TriggerSkill("#jgbiantian-dw") {
        frequency = Compulsory;
        events << DamageForseen;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        ServerPlayer *zhuge = room->findPlayerBySkillName("jgbiantian");
        if (zhuge == NULL)
            return QStringList();

        ask_who = zhuge;

        DamageStruct damage = data.value<DamageStruct>();
        return (player != NULL && player->getMark("@fog") > 0 && damage.nature != DamageStruct::Thunder) ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        return ask_who->hasShownSkill("jgbiantian");
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#FogProtect";
        log.from = ask_who;
        log.to << player;
        log.arg = QString::number(damage.damage);
        if (damage.nature == DamageStruct::Normal)
            log.arg2 = "normal_nature";
        else if (damage.nature == DamageStruct::Fire)
            log.arg2 = "fire_nature";
        room->sendLog(log);

        return true;
    }
};

class JGGongshen : public PhaseChangeSkill{
public:
    JGGongshen() : PhaseChangeSkill("jggongshen") {
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Finish)
            return QStringList();

        player->tag.remove("jggongshen");

        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->getGeneral()->objectName().contains("machine"))
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->getGeneral()->objectName().contains("machine"))
                players << p;
        }

        if (players.isEmpty())
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, players, objectName(), "@jggongshen", true, true);
        if (target != NULL) {
            player->tag["jggongshen"] = QVariant::fromValue(target);
            return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        ServerPlayer *target = player->tag["jggongshen"].value<ServerPlayer *>();
        player->tag.remove("jggongshen");
        if (target != NULL) {
            Room *room = target->getRoom();
            if (player->isFriendWith(target)) {
                RecoverStruct recover;
                recover.recover = 1;
                recover.who = player;
                room->recover(target, recover);
            }
            else
                room->damage(DamageStruct(objectName(), player, target, 1, DamageStruct::Fire));
        }
        return false;
    }
};

class JGZhinang : public PhaseChangeSkill{
public:
    JGZhinang() : PhaseChangeSkill("jgzhinang") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player) && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        QList<int> ids = room->getNCards(3);
        CardsMoveStruct move(ids, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_SHOW, target->objectName(), objectName(), QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        bool has_equip = false;
        bool has_trick = false;

        foreach(int id, ids){
            switch (Sanguosha->getCard(id)->getTypeId()){
            case Card::TypeTrick:
                has_trick = true;
                break;
            case Card::TypeEquip:
                has_equip = true;
                break;
            default:
                break;
            }
        }

        QStringList choices;
        if (has_equip)
            choices << "Equip";
        if (has_trick)
            choices << "Trick";

        QList<int> remaining_ids = ids;
        if (!choices.isEmpty()){
            QString choice = room->askForChoice(target, objectName(), choices.join("+"), IntList2StringList(ids));
            QList<int> selected_ids;
            foreach(int id, ids) {
                const Card *c = Sanguosha->getCard(id);
                if ((c->getTypeId() == Card::TypeTrick && choice == "Trick") || (c->getTypeId() == Card::TypeEquip && choice == "Equip")) {
                    selected_ids << id;
                    remaining_ids.removeOne(id);
                }
            }

            room->fillAG(selected_ids, target);

            QList<ServerPlayer *> friends;
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->isFriendWith(target))
                    friends << p;
            }

            ServerPlayer *t = room->askForPlayerChosen(target, friends, objectName(), "@jgzhinang:::" + choice);
            if (t == NULL)
                t = friends.at(qrand() % friends.length());

            room->clearAG(target);

            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, target->objectName(), t->objectName());

            CardMoveReason reason(CardMoveReason::S_REASON_GIVE, target->objectName(), objectName(), QString());
            DummyCard dummy(selected_ids);
            room->obtainCard(t, &dummy, reason);
        }

        DummyCard dummy_throw(remaining_ids);
        room->throwCard(&dummy_throw, NULL);

        return false;
    }
};

class JGJingmiao : public TriggerSkill{
public:
    JGJingmiao() : TriggerSkill("jgjingmiao") {
        events << CardFinished;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Nullification"))
            return QStringList();
        ServerPlayer *yueying = room->findPlayerBySkillName(objectName());
        if (!yueying || player->isFriendWith(yueying))
            return QStringList();
        ask_who = yueying;
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        return player->hasShownSkill(this) || ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player));
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = ask_who;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(ask_who, objectName());

        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), player->objectName());

        room->loseHp(player);
        return false;
    }
};

class JGYuhuo : public TriggerSkill{
public:
    explicit JGYuhuo(const QString &owner) : TriggerSkill("jgyuhuo_" + owner) {
        setObjectName("jgyuhuo_" + owner);
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        return (TriggerSkill::triggerable(player) && data.value<DamageStruct>().nature == DamageStruct::Fire) ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());
        LogMessage log;
        log.type = "#YuhuoProtect";
        log.from = player;
        log.arg = QString::number(data.value<DamageStruct>().damage);
        log.arg2 = "fire_nature";
        room->sendLog(log);
        return true;
    }
};

class JGQiwu : public TriggerSkill{
public:
    JGQiwu() : TriggerSkill("jgqiwu") {
        events << CardsMoveOneTime;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == player && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
            bool cardok = false;
            for (int i = 0; i < move.card_ids.length(); ++i) {
                if (Sanguosha->getCard(move.card_ids.at(i))->getSuit() == Card::Club && (move.from_places.at(i) == Player::PlaceHand || move.from_places.at(i) == Player::PlaceEquip)) {
                    cardok = true;
                    break;
                }
            }

            if (!cardok)
                return QStringList();

            foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->isFriendWith(player) && p->isWounded())
                    return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->isFriendWith(player) && p->isWounded())
                players << p;
        }
        ServerPlayer *target = room->askForPlayerChosen(player, players, objectName(), "@jgqiwu", false, true);
        if (target != NULL) {
            player->tag[objectName()] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        ServerPlayer *target = player->tag[objectName()].value<ServerPlayer *>();
        player->tag.remove(objectName());
        if (target != NULL) {
            RecoverStruct rec;
            rec.recover = 1;
            rec.who = player;
            room->recover(target, rec);
        }

        return false;
    }
};

class JGTianyu : public PhaseChangeSkill{
public:
    JGTianyu() : PhaseChangeSkill("jgtianyu") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Finish)
            return QStringList();

        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player) && !p->isChained())
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        QList<ServerPlayer *> targets;

        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player) && !p->isChained()) {
                targets << p;
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
            }
        }

        room->sortByActionOrder(targets);

        foreach (ServerPlayer *p, targets) {
            p->setChained(true);
            room->setEmotion(p, "chain");
            room->broadcastProperty(p, "chained");
            room->getThread()->trigger(ChainStateChanged, room, p);
        }
        return false;
    }
};

class JGJiguan : public TriggerSkill{ //temp method
public:
    explicit JGJiguan(const QString &owner) : TriggerSkill("jgjiguan_" + owner) {
        setObjectName("jgjiguan_" + owner);
        events << TargetConfirming;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card != NULL && use.card->objectName() == "indulgence" && use.to.contains(player) && TriggerSkill::triggerable(player))
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        CardUseStruct use = data.value<CardUseStruct>();
        LogMessage log2;
        if (use.from) {
            log2.type = "$CancelTarget";
            log2.from = use.from;
        }
        else {
            log2.type = "$CancelTargetNoUser";
        }
        log2.to << player;
        log2.arg = use.card->objectName();
        room->sendLog(log2);
        room->setEmotion(player, "cancel");

        use.to.removeOne(player);
        data = QVariant::fromValue(use);
        return false;
    }
};

class JGMojian : public PhaseChangeSkill{
public:
    JGMojian() : PhaseChangeSkill("jgmojian") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Play;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        QList<ServerPlayer *> targets;
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        ArcheryAttack *aa = new ArcheryAttack(Card::NoSuit, 0);
        aa->setSkillName("_" + objectName());
        aa->setShowSkill(objectName());

        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player) && !player->isProhibited(p, aa))
                targets << p;
        }

        room->useCard(CardUseStruct(aa, player, targets));

        return false;
    }
};

class JGZhenwei : public DistanceSkill{
public:
    JGZhenwei() : DistanceSkill("jgzhenwei") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (from->isFriendWith(to))
            return 0;

        foreach (const Player *p, to->getAliveSiblings()) {
            if (p->isFriendWith(to) && p != to && p->hasShownSkill(objectName()))
                return 1;
        }

        return 0;
    }
};

class JGBenlei : public PhaseChangeSkill{
public:
    JGBenlei() : PhaseChangeSkill("jgbenlei") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Start) {
            foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                if (!p->isFriendWith(player) && p->getGeneral()->objectName().contains("machine"))
                    return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        ServerPlayer *victim = NULL;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player) && p->getGeneral()->objectName().contains("machine")) {
                victim = p;
                break;
            }
        }

        if (victim != NULL) {
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), victim->objectName());
            room->damage(DamageStruct(objectName(), player, victim, 2, DamageStruct::Thunder));
        }
        return false;
    }
};

class JGTianyun : public PhaseChangeSkill{
public:
    JGTianyun() : PhaseChangeSkill("jgtianyun") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getHp() > 0 && target->getPhase() == Player::Finish;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player))
                players << p;
        }

        if (players.isEmpty())
            return false;

        player->tag.remove("jgtianyun");
        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jgtianyun", true, true);

        if (victim != NULL) {
            player->tag["jgtianyun"] = QVariant::fromValue(victim);
            room->loseHp(player);
            return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        ServerPlayer *victim = player->tag["jgtianyun"].value<ServerPlayer *>();
        if (victim == NULL)
            return false;

        Room *room = player->getRoom();
        room->damage(DamageStruct(objectName(), player, victim, 2, DamageStruct::Fire));
        QList<const Card *> cards = victim->getEquips();
        if (!cards.isEmpty()) {
            DummyCard dummy;
            dummy.addSubcards(cards);
            room->throwCard(&dummy, victim, player);
        }
        return false;
    }
};

class JGYizhong : public TriggerSkill{
public:
    JGYizhong() : TriggerSkill("jgyizhong") {
        frequency = Compulsory;
        events << SlashEffected;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player) || player->getArmor() != NULL)
            return QStringList();

        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        return effect.slash->isBlack() ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());
        LogMessage log;
        log.type = "#SkillNullify";
        log.from = player;
        log.arg = objectName();
        log.arg2 = data.value<SlashEffectStruct>().slash->objectName();
        room->sendLog(log);
        return true;
    }
};

class JGLingyu : public PhaseChangeSkill{
public:
    JGLingyu() : PhaseChangeSkill("jglingyu") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish) {
            foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->isFriendWith(player) && p->isWounded())
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            player->turnOver();
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> targets;

        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->isFriendWith(player) && p->isWounded()) {
                targets << p;
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
            }
        }

        room->sortByActionOrder(targets);

        foreach (ServerPlayer *p, targets) {
            RecoverStruct rec;
            rec.recover = 1;
            rec.who = player;
            room->recover(p, rec);
        }
        return false;
    }
};

class JGChiying : public TriggerSkill{
public:
    JGChiying() : TriggerSkill("jgchiying") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const{
        ServerPlayer *zidan = room->findPlayerBySkillName(objectName());
        if (zidan != NULL && player != NULL && zidan->isFriendWith(player)) {
            ask_who = zidan;
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.damage >= 2)
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        return ask_who->hasShownSkill(objectName()) || ask_who->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        room->notifySkillInvoked(ask_who, objectName());
        DamageStruct damage = data.value<DamageStruct>();
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), player->objectName());
        LogMessage log;
        log.type = "#JGChiying";
        log.from = ask_who;
        log.arg = QString::number(damage.damage);
        log.arg2 = objectName();
        room->sendLog(log);
        damage.damage = 1;
        data = QVariant::fromValue(damage);
        return false;
    }
};

class JGJingfan : public DistanceSkill{
public:
    JGJingfan() : DistanceSkill("jgjingfan") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (from->isFriendWith(to))
            return 0;

        foreach (const Player *p, from->getAliveSiblings()) {
            if (p->isFriendWith(from) && p != from && p->hasShownSkill(objectName()))
                return -1;
        }

        return 0;
    }
};

class JGChuanyun : public PhaseChangeSkill{
public:
    JGChuanyun() : PhaseChangeSkill("jgchuanyun") {
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Finish)
            return QStringList();

        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->getHp() >= player->getHp())
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->getHp() >= player->getHp())
                players << p;
        }

        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jgchuanyun", true, true);
        if (victim != NULL) {
            player->tag["jgchuanyun"] = QVariant::fromValue(victim);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        ServerPlayer *victim = target->tag["jgchuanyun"].value<ServerPlayer *>();
        target->tag.remove("jgchuanyun");
        if (victim != NULL)
            victim->getRoom()->damage(DamageStruct(objectName(), target, victim, 1));

        return false;
    }
};

class JGLeili : public TriggerSkill{
public:
    JGLeili() : TriggerSkill("jgleili") {
        events << Damage;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card != NULL && damage.card->isKindOf("Slash"))
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player))
                players << p;
        }
        DamageStruct damage = data.value<DamageStruct>();
        players.removeOne(damage.to);// another one

        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jgleili", true, true);
        if (victim != NULL) {
            player->tag["jgleili"] = QVariant::fromValue(victim);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        ServerPlayer *victim = player->tag["jgleili"].value<ServerPlayer *>();
        player->tag.remove("jgleili");
        if (victim != NULL)
            room->damage(DamageStruct(objectName(), player, victim, 1, DamageStruct::Thunder));

        return false;
    }
};

class JGFengxing : public PhaseChangeSkill{
public:
    JGFengxing() : PhaseChangeSkill("jgfengxing") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player))
                players << p;
        }

        player->tag.remove("jgfengxing");
        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jgfengxing", true/*, true*/);
        if (victim != NULL) {
            player->tag["jgfengxing"] = QVariant::fromValue(victim);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        ServerPlayer *victim = target->tag["jgfengxing"].value<ServerPlayer *>();
        target->tag.remove("jgfengxing");
        if (victim != NULL) {
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName("_" + objectName());
            slash->setShowSkill(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = target;
            use.to << victim;
            use.m_addHistory = false;
            victim->getRoom()->useCard(use, false);
        }
        return false;
    }
};

class JGKonghunRecord : public TriggerSkill{
public:
    JGKonghunRecord() : TriggerSkill("#jgkonghun-record") {
        events << DamageDone;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer* &ask_who) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && damage.from->hasSkill("jgkonghun") && damage.reason == "jgkonghun" && !damage.transfer && !damage.chain) {
            ask_who = damage.from;
            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        ask_who->addMark("jgkonghun", 1);
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const{
        Q_ASSERT(false);
        return false;
    }
};

class JGKonghun : public PhaseChangeSkill{
public:
    JGKonghun() : PhaseChangeSkill("jgkonghun") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Play)
            return QStringList();

        int num = 0;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player))
                ++num;
        }

        if (player->getLostHp() >= num)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->setMark("jgkonghun", 0);
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(player)) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                players << p;
            }
        }

        room->sortByActionOrder(players);
        foreach(ServerPlayer *p, players)
            room->damage(DamageStruct(objectName(), player, p, 1, DamageStruct::Thunder));

        int n = player->getMark("jgkonghun");
        player->setMark("jgkonghun", 0);

        if (n > 0) {
            RecoverStruct rec;
            rec.recover = n;
            rec.who = player;
            room->recover(player, rec);
        }

        return false;
    }
};

class JGFanshi : public PhaseChangeSkill{
public:
    JGFanshi() : PhaseChangeSkill("jgfanshi") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Finish/* && target->hasShownSkill(this)*/;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = target;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(target, objectName());
        room->loseHp(target);
        return false;
    }
};

class JGXuanlei : public PhaseChangeSkill{
public:
    JGXuanlei() : PhaseChangeSkill("jgxuanlei") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Start)
            return QStringList();

        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player) && !p->getJudgingArea().isEmpty())
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(player) && !p->getJudgingArea().isEmpty()) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                players << p;
            }
        }

        room->sortByActionOrder(players);
        foreach(ServerPlayer *p, players)
            room->damage(DamageStruct(objectName(), player, p, 1, DamageStruct::Thunder));


        return false;
    }
};

class JGHuodi : public PhaseChangeSkill{
public:
    JGHuodi() : PhaseChangeSkill("jghuodi") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Finish)
            return QStringList();

        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (p->isFriendWith(player) && !p->faceUp())
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(player))
                players << p;
        }

        player->tag.remove("jghuodi");
        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jghuodi", true, true);
        if (victim != NULL) {
            player->tag["jghuodi"] = QVariant::fromValue(victim);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        ServerPlayer *victim = target->tag["jghuodi"].value<ServerPlayer *>();
        target->tag.remove("jghuodi");
        if (victim != NULL)
            victim->turnOver();

        return false;
    }
};

class JGJueji : public TriggerSkill{
public:
    JGJueji() : TriggerSkill("jgjueji") {
        events << DrawNCards;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *zhanghe = room->findPlayerBySkillName(objectName());
        if (zhanghe != NULL && zhanghe->isAlive() && !player->isFriendWith(zhanghe) && player->isWounded() && data.toInt() > 0) {
            QMap<ServerPlayer *, QStringList> m;
            m.insert(zhanghe, QStringList(objectName()));
            return m;
        }

        return QMap<ServerPlayer *, QStringList>();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who /* = NULL */) const{
        return ask_who->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), player->objectName());

        data = data.toInt() - 1;

        return false;
    }
};

class JGDidong : public PhaseChangeSkill{
public:
    JGDidong() : PhaseChangeSkill("jgdidong") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(player))
                players << p;
        }

        player->tag.remove("jgdidong");
        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jgdidong", true, true);
        if (victim != NULL) {
            player->tag["jgdidong"] = QVariant::fromValue(victim);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        ServerPlayer *victim = target->tag["jgdidong"].value<ServerPlayer *>();
        target->tag.remove("jgdidong");
        if (victim != NULL)
            victim->turnOver();

        return false;
    }
};

class JGLianyu : public PhaseChangeSkill{
public:
    JGLianyu() : PhaseChangeSkill("jglianyu") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(player)) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                players << p;
            }
        }

        room->sortByActionOrder(players);
        foreach(ServerPlayer *p, players)
            room->damage(DamageStruct(objectName(), player, p, 1, DamageStruct::Fire));


        return false;
    }
};

class JGTanshi : public DrawCardsSkill{
public:
    JGTanshi() : DrawCardsSkill("jgtanshi") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        Room *room = player->getRoom();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());
        return n - 1;
    }
};

class JGTunshi : public PhaseChangeSkill{
public:
    JGTunshi() : PhaseChangeSkill("jgtunshi"){
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Start)
            return QStringList();

        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(player) && p->getHandcardNum() > player->getHandcardNum())
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = target;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(target, objectName());

        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (!p->isFriendWith(target) && p->getHandcardNum() > target->getHandcardNum()) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, target->objectName(), p->objectName());
                players << p;
            }
        }

        room->sortByActionOrder(players);

        foreach(ServerPlayer *p, players)
            room->damage(DamageStruct(objectName(), target, p));

        return false;
    }
};

class JGNailuo : public PhaseChangeSkill{
public:
    JGNailuo() : PhaseChangeSkill("jgnailuo") {

    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            player->turnOver();
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> targets;

        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!p->isFriendWith(player)) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                targets << p;
            }
        }

        room->sortByActionOrder(targets);

        foreach (ServerPlayer *p, targets)
            p->throwAllEquips();
        return false;
    }
};

JiangeDefensePackage::JiangeDefensePackage()
    : Package("jiange-defense") {

    General *liubei = new General(this, "jg_liubei", "shu", 5, true, true);
    liubei->addSkill(new JGJizhen);
    liubei->addSkill(new JGLingfeng);

    General *zhuge = new General(this, "jg_zhuge", "shu", 4, true, true);
    zhuge->addSkill(new JGBiantian);
    zhuge->addSkill(new JGBiantianDW);
    zhuge->addSkill(new JGBiantianKF);
    insertRelatedSkills("jgbiantian", 2, "#jgbiantian-dw", "#jgbiantian-kf");
    zhuge->addSkill("bazhen");

    General *yueying = new General(this, "jg_yueying", "shu", 4, false, true);
    yueying->addSkill(new JGGongshen);
    yueying->addSkill(new JGZhinang);
    yueying->addSkill(new JGJingmiao);

    General *pangtong = new General(this, "jg_pangtong", "shu", 4, true, true);
    pangtong->addSkill(new JGYuhuo("pangtong"));
    pangtong->addSkill(new JGQiwu);
    pangtong->addSkill(new JGTianyu);

    General *qinglong = new General(this, "jg_qinglong_machine", "shu", 4, true, true);
    qinglong->addSkill(new JGJiguan("qinglong"));
    qinglong->addSkill(new JGMojian);

    General *baihu = new General(this, "jg_baihu_machine", "shu", 4, true, true);
    baihu->addSkill(new JGJiguan("baihu"));
    baihu->addSkill(new JGZhenwei);
    baihu->addSkill(new JGBenlei);

    General *zhuque = new General(this, "jg_zhuque_machine", "shu", 5, false, true);
    zhuque->addSkill(new JGJiguan("zhuque"));
    zhuque->addSkill(new JGYuhuo("zhuque"));
    zhuque->addSkill(new JGTianyun);

    General *xuanwu = new General(this, "jg_xuanwu_machine", "shu", 5, true, true);
    xuanwu->addSkill(new JGJiguan("xuanwu"));
    xuanwu->addSkill(new JGYizhong);
    xuanwu->addSkill(new JGLingyu);

    //------------------------------------------------------------------------------------

    General *caozhen = new General(this, "jg_caozhen", "wei", 5, true, true);
    caozhen->addSkill(new JGChiying);
    caozhen->addSkill(new JGJingfan);

    General *xiahou = new General(this, "jg_xiahou", "wei", 4, true, true);
    xiahou->addSkill(new JGChuanyun);
    xiahou->addSkill(new JGLeili);
    xiahou->addSkill(new JGFengxing);

    General *sima = new General(this, "jg_sima", "wei", 5, true, true);
    sima->addSkill(new JGKonghun);
    sima->addSkill(new JGKonghunRecord);
    insertRelatedSkills("jgkonghun", "#jgkonghun-record");
    sima->addSkill(new JGFanshi);
    sima->addSkill(new JGXuanlei);

    General *zhanghe = new General(this, "jg_zhanghe", "wei", 4, true, true);
    zhanghe->addSkill(new JGHuodi);
    zhanghe->addSkill(new JGJueji);

    General *bian = new General(this, "jg_bian_machine", "wei", 4, true, true);
    bian->addSkill(new JGJiguan("bian"));
    bian->addSkill(new JGDidong);

    General *suanni = new General(this, "jg_suanni_machine", "wei", 3, true, true);
    suanni->addSkill(new JGJiguan("suanni"));
    suanni->addSkill(new JGLianyu);

    General *taotie = new General(this, "jg_chiwen_machine", "wei", 5, true, true);
    taotie->addSkill(new JGJiguan("chiwen"));
    taotie->addSkill(new JGTanshi);
    taotie->addSkill(new JGTunshi);

    General *yazi = new General(this, "jg_yazi_machine", "wei", 4, true, true);
    yazi->addSkill(new JGJiguan("yazi"));
    yazi->addSkill(new JGNailuo);

}

ADD_PACKAGE(JiangeDefense)
