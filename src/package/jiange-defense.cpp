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

        room->notifySkillInvoked(target, objectName());

        QList<ServerPlayer *> draw_list;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (p->isFriendWith(target) && p->isWounded())
                draw_list << p;
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

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        return (TriggerSkill::triggerable(player) && player->getPhase() == Player::Draw) ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        QList<int> cardids = room->getNCards(2);
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

            ServerPlayer *victim = room->askForPlayerChosen(target, players, objectName() + "-losehp", "@jglingfeng");
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
        else if (player != NULL && (triggerEvent == Death && player->hasSkill(objectName()) && data.value<DeathStruct>().who == player)
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
                if (!p->isFriendWith(player))
                    p->gainMark("@gale", 1);
            }
        }
        else if (judge.pattern == "b") {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->isFriendWith(player))
                    p->gainMark("@fog", 1);
            }
        }
        else
            Q_ASSERT(false);

        return false;
    }
};

class JGBiantianKF : public TriggerSkill{
public:
    JGBiantianKF() : TriggerSkill("#jgbiantian_kf") {
        frequency = Compulsory;
        events << DamageForseen;
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#GalePower";
        log.from = player;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);

        data = QVariant::fromValue(damage);

        return false;
    }
};

class JGBiantianDW : public TriggerSkill{
public:
    JGBiantianDW() : TriggerSkill("#jgbiantian_dw") {
        frequency = Compulsory;
        events << DamageForseen;
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#FogProtect";
        log.from = player;
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
                room->damage(DamageStruct(objectName(), player, target));
        }
        return false;
    }
};

class JGZhinang : public PhaseChangeSkill{
public:
    JGZhinang() : PhaseChangeSkill("jgzhinang") {

    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player) && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        int id = room->getNCards(1).first();
        CardsMoveStruct move(id, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_SHOW, target->objectName(), objectName(), QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        const Card *c = Sanguosha->getCard(id);
        bool discard = (c->getTypeId() == Card::TypeBasic);

        if (c->getTypeId() != Card::TypeBasic) {
            QList<ServerPlayer *> friends;
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->isFriendWith(target))
                    friends << p;
            }
            ServerPlayer *to_give = room->askForPlayerChosen(target, friends, objectName() + "-give", "@jgzhinang", true);
            if (to_give == NULL)
                discard = true;
            else {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, target->objectName(), to_give->objectName());
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, target->objectName(), to_give->objectName(), objectName(), QString());
                room->obtainCard(to_give, c, reason);
            }
        }

        if (discard) {
            CardMoveReason reason(CardMoveReason::S_REASON_PUT, target->objectName());
            room->moveCardTo(c, NULL, Player::DiscardPile, reason, true);
        }

        return false;
    }
};

class JGJingmiaoRecord : public TriggerSkill{
public:
    JGJingmiaoRecord() : TriggerSkill("#jgjingmiao-record") {
        global = true;
        events << CardUsed;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer* &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card != NULL && use.from != NULL && use.card->isKindOf("Nullification"))
            room->setTag("jgjingmiao_use", data);

        return QStringList();
    }
};

class JGJingmiao : public TriggerSkill{
public:
    JGJingmiao() : TriggerSkill("jgjingmiao") {
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        QVariant jingmiao_use_tag = room->getTag("jgjingmiao_use");
        if (jingmiao_use_tag.isNull() || !jingmiao_use_tag.canConvert<CardUseStruct>())
            return QStringList();
        CardUseStruct jingmiao_use = jingmiao_use_tag.value<CardUseStruct>();

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_USE) || move.to_place != Player::DiscardPile)
            return QStringList();

        QList<int> card_ids;
        if (jingmiao_use.card->isVirtualCard())
            card_ids = jingmiao_use.card->getSubcards();
        else
            card_ids << jingmiao_use.card->getEffectiveId();

        foreach(int id, card_ids) {
            if (!move.card_ids.contains(id))
                return QStringList();
        }

        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (!(player->hasShownSkill(this) || player->askForSkillInvoke(objectName()))){
            room->removeTag("jgjingmiao_use");
            return false;
        }
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());
        CardUseStruct jingmiao_use = room->getTag("jgjingmiao_use").value<CardUseStruct>();
        room->removeTag("jgjingmiao_use");
        room->loseHp(jingmiao_use.from);
        return false;
    }
};

class JGYuhuo : public TriggerSkill{
public:
    JGYuhuo(const QString &owner) : TriggerSkill("jgyuhuo_" + owner){
        setObjectName("jgyuhuo_" + owner);
        events << DamageInflicted;
        frequency = Compulsory;
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
        log.type = "#ShixinProtect";
        log.from = player;
        log.arg = QString::number(data.value<DamageStruct>().damage);
        log.arg2 = "fire_nature";
        room->sendLog(log);
        return true;
    }
};

class JGQiwuRecord : public TriggerSkill{
public:
    JGQiwuRecord() : TriggerSkill("#jgqiwu-record") {
        global = true;
        events << CardUsed << CardResponded << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent != CardFinished){
            ServerPlayer *user = NULL;
            const Card *card = NULL;
            if (triggerEvent == CardUsed){
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.card != NULL && use.from != NULL && (use.card->getTypeId() == Card::TypeBasic || use.card->isNDTrick()) && use.card->getSuit() == Card::Club){
                    user = use.from;
                    card = use.card;
                }
            }
            else if (triggerEvent == CardResponded){
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_isUse && resp.m_card != NULL && player != NULL && (resp.m_card->getTypeId() == Card::TypeBasic || resp.m_card->isNDTrick()) && resp.m_card->getSuit() == Card::Club){
                    user = player;
                    card = resp.m_card;
                }
            }

            if (user != NULL && card != NULL){
                QStringList l = player->tag["jgqiwu"].toStringList();
                if (!l.contains(card->toString()))
                    l << card->toString();
                player->tag["jgqiwu"] = l;
            }
        }
        else {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.from != NULL){
                QStringList l = player->tag["jgqiwu"].toStringList();
                l.removeOne(use.card->toString());
                player->tag["jgqiwu"] = l;
            }
        }
        return QStringList();
    }
};

class JGQiwu : public TriggerSkill{
public:
    JGQiwu() : TriggerSkill("jgqiwu"){
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_USE) || move.to_place != Player::DiscardPile)
            return QStringList();

        if (TriggerSkill::triggerable(player)){
            QStringList l = player->tag["jgqiwu"].toStringList();
            foreach(QString str, l){
                const Card *c = Card::Parse(str);
                QList<int> card_ids;
                if (c->isVirtualCard())
                    card_ids = c->getSubcards();
                else
                    card_ids << c->getEffectiveId();

                bool cont = false;
                foreach(int id, card_ids) {
                    if (!move.card_ids.contains(id)){
                        cont = true;
                        break;
                    }
                }
                if (cont)
                    continue;

                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());
        RecoverStruct rec;
        rec.recover = 1;
        rec.who = player;
        room->recover(player, rec);
        return false;
    }
};

class JGTianyu : public PhaseChangeSkill{
public:
    JGTianyu() : PhaseChangeSkill("jgtianyu"){
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
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
        room->notifySkillInvoked(player, objectName());
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if (!p->isFriendWith(player) && !p->isChained())
                room->setPlayerProperty(p, "chained", true);
        }
        return false;
    }
};

class JGJiguan : public TriggerSkill{ //temp method
public:
    JGJiguan(const QString &owner) : TriggerSkill("jgjiguan_" + owner){
        setObjectName("jgjiguan_" + owner);
        events << TargetConfirming;
        frequency = Compulsory;
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
        room->notifySkillInvoked(player, objectName());

        CardUseStruct use = data.value<CardUseStruct>();
        LogMessage log;
        if (use.from) {
            log.type = "$CancelTarget";
            log.from = use.from;
        }
        else {
            log.type = "$CancelTargetNoUser";
        }
        log.to << player;
        log.arg = use.card->objectName();
        room->sendLog(log);
        room->setEmotion(player, "cancel");

        use.to.removeOne(player);
        data = QVariant::fromValue(use);
        return false;
    }
};

class JGMojian : public PhaseChangeSkill{
public:
    JGMojian() : PhaseChangeSkill("jgmojian"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Play;
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if (!p->isFriendWith(player))
                targets << p;
        }

        ArcheryAttack *aa = new ArcheryAttack(Card::NoSuit, 0);
        aa->setSkillName("_" + objectName());
        aa->setShowSkill(objectName());
        CardUseStruct use;
        use.card = aa;
        use.from = player;
        use.to = targets;

        player->getRoom()->useCard(use);

        return false;
    }
};

class JGZhenwei : public DistanceSkill{
public:
    JGZhenwei() : DistanceSkill("jgzhenwei"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (from->isFriendWith(to))
            return 0;

        foreach(const Player *p, to->getAliveSiblings()){
            if (p->isFriendWith(to) && p != to && p->hasShownSkill(objectName()))
                return 1;
        }

        return 0;
    }
};

class JGBenlei : public PhaseChangeSkill{
public:
    JGBenlei() : PhaseChangeSkill("jgbenlei"){
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Start){
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if (!p->isFriendWith(player) && p->getGeneral()->objectName().contains("machine"))
                    return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *, ServerPlayer *player, QVariant &, ServerPlayer * ) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->notifySkillInvoked(player, objectName());

        ServerPlayer *victim = NULL;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if (!p->isFriendWith(player) && p->getGeneral()->objectName().contains("machine")){
                victim = p;
                break;
            }
        }

        if (victim != NULL){
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), victim->objectName());
            room->damage(DamageStruct(objectName(), player, victim, 1, DamageStruct::Thunder));
        }
        return false;
    }
};

class JGTianyun : public PhaseChangeSkill{
public:
    JGTianyun() : PhaseChangeSkill("jgtianyun"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getHp() > 0;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if (!p->isFriendWith(player))
                players << p;
        }

        if (players.isEmpty())
            return false;

        player->tag.remove("jgtianyun");
        ServerPlayer *victim = room->askForPlayerChosen(player, players, objectName(), "@jgtianyun", true, true);

        if (victim != NULL){
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
        room->damage(DamageStruct(objectName(), player, victim, 1, DamageStruct::Fire));
        QList<const Card *> cards = victim->getEquips();
        if (!cards.isEmpty()){
            DummyCard dummy;
            dummy.addSubcards(cards);
            room->throwCard(&dummy, victim, player);
        }
        return false;
    }
};


JiangeDefensePackage::JiangeDefensePackage()
    : Package("jiange-defense", Package::MixedPack){

}


ADD_PACKAGE(JiangeDefense)

