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

#include "formation.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "client.h"
#include "engine.h"
#include "structs.h"
#include "gamerule.h"
#include "settings.h"
#include "json.h"

class Tuntian : public TriggerSkill {
public:
    Tuntian() : TriggerSkill("tuntian") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == CardsMoveOneTime && TriggerSkill::triggerable(player) && player->getPhase() == Player::NotActive) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && (move.from_places.contains(Player::PlaceHand) || move.from_places.contains(Player::PlaceEquip))
                && !(move.to == player && (move.to_place == Player::PlaceHand || move.to_place == Player::PlaceEquip))) {
                if (room->getTag("judge").toInt() > 0) {
                    player->addMark("tuntian_postpone");
                    return QStringList();
                }
                else return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *dengai) const{
        if (dengai->askForSkillInvoke("tuntian", data)){
            room->broadcastSkillInvoke("tuntian", dengai);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *dengai) const{
        JudgeStruct judge;
        judge.pattern = ".|heart";
        judge.good = false;
        judge.reason = "tuntian";
        judge.who = dengai;
        room->judge(judge);
        return false;
    }
};

class TuntianPostpone : public TriggerSkill{
public:
    TuntianPostpone() : TriggerSkill("#tuntian-postpone"){
        events << FinishJudge;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        QList<ServerPlayer *> dengais = room->findPlayersBySkillName("tuntian");
        foreach(ServerPlayer *dengai, dengais) {
            int postponed = dengai->getMark("tuntian_postpone");
            if (postponed > 0){
                dengai->removeMark("tuntian_postpone");
                skill_list.insert(dengai, QStringList("tuntian"));
            }
        }
        return skill_list;
    }
};

class TuntianGotoField : public TriggerSkill{
public:
    TuntianGotoField() : TriggerSkill("#tuntian-gotofield"){
        events << FinishJudge;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer * &ask_who) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        if (judge->who != NULL && judge->who->isAlive() && judge->who->hasSkill("tuntian")){
            if (judge->reason == "tuntian" && judge->isGood() && room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge){
                ask_who = judge->who;
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        return judge->who->askForSkillInvoke("_tuntian", "gotofield");
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        judge->who->addToPile("field", judge->card);

        return false;
    }
};

class TuntianDistance : public DistanceSkill {
public:
    TuntianDistance() : DistanceSkill("#tuntian-dist") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasShownSkill("tuntian"))
            return -from->getPile("field").length();
        else
            return 0;
    }
};

class Jixi : public OneCardViewAsSkill {
public:
    Jixi() : OneCardViewAsSkill("jixi") {
        relate_to_place = "head";
        filter_pattern = ".|.|.|field";
        expand_pile = "field";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("field").isEmpty();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Snatch *shun = new Snatch(originalCard->getSuit(), originalCard->getNumber());
        shun->addSubcard(originalCard);
        shun->setSkillName(objectName());
        shun->setShowSkill(objectName());
        return shun;
    }
};

ZiliangCard::ZiliangCard(){
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void ZiliangCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    source->tag["ziliang"] = subcards.first();
}

class ZiliangVS : public OneCardViewAsSkill{
public:
    ZiliangVS() : OneCardViewAsSkill("ziliang"){
        response_pattern = "@@ziliang";
        filter_pattern = ".|.|.|field";
        expand_pile = "field";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ZiliangCard *c = new ZiliangCard;
        c->addSubcard(originalCard);
        c->setShowSkill(objectName());
        return c;
    }
};

class Ziliang : public TriggerSkill {
public:
    Ziliang() : TriggerSkill("ziliang") {
        events << Damaged;
        relate_to_place = "deputy";
        view_as_skill = new ZiliangVS;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL || player->isDead()) return skill_list;
        QList<ServerPlayer *> dengais = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *dengai, dengais)
            if (!dengai->getPile("field").isEmpty() && dengai->isFriendWith(player))
                skill_list.insert(dengai, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *player = ask_who;
        player->tag.remove("ziliang");
        player->tag["ziliang_aidata"] = data;
        if (room->askForUseCard(player, "@@ziliang", "@ziliang-give", -1, Card::MethodNone))
            return true;

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        ServerPlayer *dengai = ask_who;
        if (!dengai) return false;

        bool ok = false;
        int id = dengai->tag["ziliang"].toInt(&ok);

        if (!ok) return false;

        if (player == dengai) {
            LogMessage log;
            log.type = "$MoveCard";
            log.from = player;
            log.to << player;
            log.card_str = QString::number(id);
            room->sendLog(log);
        }
        else
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, dengai->objectName(), player->objectName());
        room->obtainCard(player, id);

        return false;
    }
};

HuyuanCard::HuyuanCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool HuyuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if (!targets.isEmpty())
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void HuyuanCard::onEffect(const CardEffectStruct &effect) const{
    const Card *equip = Sanguosha->getCard(subcards[0]);

    effect.from->tag["huyuan_target"] = QVariant::fromValue(effect.to);

    effect.from->getRoom()->moveCardTo(equip, effect.from, effect.to, Player::PlaceEquip,
        CardMoveReason(CardMoveReason::S_REASON_PUT, effect.from->objectName(), "huyuan", QString()));

    LogMessage log;
    log.type = "$ZhijianEquip";
    log.from = effect.to;
    log.card_str = QString::number(equip->getEffectiveId());
    effect.from->getRoom()->sendLog(log);
}

class HuyuanViewAsSkill : public OneCardViewAsSkill {
public:
    HuyuanViewAsSkill() : OneCardViewAsSkill("huyuan") {
        response_pattern = "@@huyuan";
        filter_pattern = "EquipCard";
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        HuyuanCard *first = new HuyuanCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Huyuan : public PhaseChangeSkill {
public:
    Huyuan() : PhaseChangeSkill("huyuan") {
        view_as_skill = new HuyuanViewAsSkill;
    }

    virtual bool canPreshow() const {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *target, QVariant &, ServerPlayer* &) const {
        if (!PhaseChangeSkill::triggerable(target)) return QStringList();
        if (target->getPhase() == Player::Finish && !target->isNude())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *target, QVariant &, ServerPlayer *) const {
        target->tag.remove("huyuan_equip");
        target->tag.remove("huyuan_target");
        bool invoke = room->askForUseCard(target, "@@huyuan", "@huyuan-equip", -1, Card::MethodNone);
        if (invoke && target->tag.contains("huyuan_target"))
            return true;

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *caohong) const{
        Room *room = caohong->getRoom();

        ServerPlayer *target = caohong->tag["huyuan_target"].value<ServerPlayer *>();

        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getAllPlayers()) {
            if (target->distanceTo(p) == 1 && caohong->canDiscard(p, "he"))
                targets << p;
        }
        if (!targets.isEmpty()) {
            ServerPlayer *to_dismantle = room->askForPlayerChosen(caohong, targets, "huyuan", "@huyuan-discard:" + target->objectName(), true);
            if (to_dismantle != NULL){
                int card_id = room->askForCardChosen(caohong, to_dismantle, "he", "huyuan", false, Card::MethodDiscard);
                room->throwCard(Sanguosha->getCard(card_id), to_dismantle, caohong);
            }
        }
        return false;
    }
};

HeyiSummon::HeyiSummon()
    : ArraySummonCard("heyi")
{
}

class Heyi : public BattleArraySkill {
public:
    Heyi() : BattleArraySkill("heyi", HegemonyMode::Formation) {
        events << GeneralShown << GeneralHidden << GeneralRemoved << Death << RemoveStateChanged;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player == NULL) return QStringList();
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->hasSkill(objectName())) {
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->getMark("feiying") > 0) {
                        room->setPlayerMark(p, "feiying", 0);
                        room->detachSkillFromPlayer(p, "feiying", true, true);
                    }
                }
                return QStringList();
            }
            if (death.who->getMark("feiying") > 0) {
                room->setPlayerMark(death.who, "feiying", 0);
                room->detachSkillFromPlayer(death.who, "feiying", true, true);
            }
        }
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->getMark("feiying") > 0) {
                room->setPlayerMark(p, "feiying", 0);
                room->detachSkillFromPlayer(p, "feiying", true, true);
            }
        }
        if (room->alivePlayerCount() < 4) return QStringList();
        QList<ServerPlayer *> caohongs = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *caohong, caohongs) {
            if (caohong->hasShownSkill(this)) {
                foreach(ServerPlayer *p, room->getOtherPlayers(caohong)) {
                    if (caohong->inFormationRalation(p)) {
                        room->setPlayerMark(p, "feiying", 1);
                        room->attachSkillToPlayer(p, "feiying");
                    }
                }
            }
        }
        return QStringList();
    }
};

class HeyiFeiying : public DistanceSkill {
public:
    HeyiFeiying() : DistanceSkill("#heyi_feiying") {
    }

    virtual int getCorrect(const Player *, const Player *to) const{
        if (to->getMark("feiying") > 0)
            return 1;
        else
            return 0;
    }
};

class Feiying : public TriggerSkill {
public:
    Feiying() : TriggerSkill("feiying") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const {
        return QStringList();
    }
};

TiaoxinCard::TiaoxinCard() {
}

bool TiaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->inMyAttackRange(Self) && to_select != Self;
}

void TiaoxinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool use_slash = false;
    if (effect.to->canSlash(effect.from, NULL, false))
        use_slash = room->askForUseSlashTo(effect.to, effect.from, "@tiaoxin-slash:" + effect.from->objectName());
    if (!use_slash && effect.from->canDiscard(effect.to, "he"))
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "tiaoxin", false, Card::MethodDiscard), effect.to, effect.from);
}

class Tiaoxin : public ZeroCardViewAsSkill {
public:
    Tiaoxin() : ZeroCardViewAsSkill("tiaoxin") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TiaoxinCard");
    }

    virtual const Card *viewAs() const{
        TiaoxinCard *card = new TiaoxinCard;
        card->setShowSkill(objectName());
        return card;
    }
};

class YiZhi : public TriggerSkill {
public:
    YiZhi() : TriggerSkill("yizhi") {
        relate_to_place = "deputy";
        frequency = Compulsory;
        events << GameStart << EventPhaseStart;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (triggerEvent == GameStart) {
            const Skill *guanxing = Sanguosha->getSkill("guanxing");
            if (guanxing != NULL && guanxing->inherits("TriggerSkill")){
                const TriggerSkill *guanxing_trigger = qobject_cast<const TriggerSkill *>(guanxing);
                room->getThread()->addTriggerSkill(guanxing_trigger);
            }
        }
        else if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Start)
            if (!player->hasSkill("guanxing"))
                return QStringList("guanxing");
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const{
        return false;
    }
};

TianfuSummon::TianfuSummon()
    : ArraySummonCard("tianfu")
{
}

class Tianfu : public BattleArraySkill {
public:
    Tianfu() : BattleArraySkill("tianfu", HegemonyMode::Formation) {
        events << EventPhaseStart << Death << EventLoseSkill << EventAcquireSkill
               << GeneralShown << GeneralHidden << GeneralRemoved << RemoveStateChanged;
        relate_to_place = "head";
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (player == NULL) return QStringList();

        if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() != Player::RoundStart)
                return QStringList();
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (player != death.who)
                return QStringList();
        }

        foreach (ServerPlayer *p, room->getPlayers()) {
            if (p->getMark("tianfu_kanpo") > 0 && p->hasSkill("kanpo") && !p->hasInnateSkill("kanpo")) {
                p->setMark("tianfu_kanpo", 0);
                room->detachSkillFromPlayer(p, "kanpo", true, true);
            }
        }

        if (triggerEvent == EventLoseSkill && data.toString() == "tianfu")
            return QStringList();
        if (triggerEvent == GeneralHidden && player->ownSkill(this) && player->inHeadSkills(objectName()) == data.toBool())
            return QStringList();
        if (triggerEvent == GeneralRemoved && data.toString() == "jiangwei")
            return QStringList();
        if (player->aliveCount() < 4)
            return QStringList();

        ServerPlayer *current = room->getCurrent();
        if (current && current->isAlive() && current->getPhase() != Player::NotActive) {
            QList<ServerPlayer *> jiangweis = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *jiangwei, jiangweis) {
                if (jiangwei->hasShownSkill(this) && jiangwei->inFormationRalation(current) && !jiangwei->hasInnateSkill("kanpo")) {
                    jiangwei->setMark("tianfu_kanpo", 1);
                    room->attachSkillToPlayer(jiangwei, "kanpo");
                }
            }
        }

        return QStringList();
    }
};

class Shengxi : public TriggerSkill {
public:
    Shengxi() : TriggerSkill("shengxi") {
        events << DamageDone << EventPhaseEnd;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == EventPhaseEnd) {
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Play) {
                if (!player->hasFlag("ShengxiDamageInPlayPhase"))
                    return QStringList(objectName());
                else
                    player->setFlags("-ShengxiDamageInPlayPhase");
            }
        }
        else if (triggerEvent == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from->getPhase() == Player::Play && !damage.from->hasFlag("ShengxiDamageInPlayPhase"))
                damage.from->setFlags("ShengxiDamageInPlayPhase");
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->drawCards(2);

        return false;
    }
};

class Shoucheng : public TriggerSkill {
public:
    Shoucheng() : TriggerSkill("shoucheng") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from && move.from->isAlive() && move.from->getPhase() == Player::NotActive
            && (move.from->isFriendWith(player) || player->willBeFriendWith(move.from))
            && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName(), data)){
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), data.value<CardsMoveOneTimeStruct>().from->objectName());
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *from = (ServerPlayer *)move.from;
        from->drawCards(1);
        return false;
    }
};

ShangyiCard::ShangyiCard() {
    mute = true;
}

bool ShangyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && (!to_select->isKongcheng() || !to_select->hasShownAllGenerals()) && to_select != Self;
}

void ShangyiCard::extraCost(Room *room, const CardUseStruct &card_use) const{
    room->showAllCards(card_use.from, card_use.to.first());
}

void ShangyiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    QStringList choices;
    if (!effect.to->isKongcheng())
        choices << "handcards";
    if (!effect.to->hasShownAllGenerals())
        choices << "hidden_general";

    QString choice = room->askForChoice(effect.from, "shangyi",
        choices.join("+"), QVariant::fromValue(effect.to));
    LogMessage log;
    log.type = "#KnownBothView";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = choice;
    foreach(ServerPlayer *p, room->getOtherPlayers(effect.from, true)){
        room->doNotify(p, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
    }

    if (choice == "handcards") {
        room->broadcastSkillInvoke("shangyi", 1, effect.from);
        QList<int> blacks;
        foreach(int card_id, effect.to->handCards()){
            if (Sanguosha->getCard(card_id)->isBlack())
                blacks << card_id;
        }
        int to_discard = room->doGongxin(effect.from, effect.to, blacks, "shangyi");
        if (to_discard == -1) return;

        effect.from->tag.remove("shangyi");
        room->throwCard(to_discard, effect.to, effect.from);
    } else {
        room->broadcastSkillInvoke("shangyi", 2, effect.from);
        QStringList list;
        if (!effect.to->hasShownGeneral1())
            list << effect.to->getActualGeneral1Name();
        if (!effect.to->hasShownGeneral2())
            list << effect.to->getActualGeneral2Name();
        foreach(QString name, list) {
            LogMessage log;
            log.type = "$KnownBothViewGeneral";
            log.from = effect.from;
            log.to << effect.to;
            QString position = effect.to->getActualGeneral1Name() == name ? "head_general" : "deputy_general";
            log.arg = Sanguosha->translate(position);
            log.arg2 = name;
            room->doNotify(effect.from, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
        }
        JsonArray arg;
        arg << "shangyi";
        arg << JsonUtils::toJsonArray(list);
        room->doNotify(effect.from, QSanProtocol::S_COMMAND_VIEW_GENERALS, arg);
    }
}

class Shangyi : public ZeroCardViewAsSkill {
public:
    Shangyi() : ZeroCardViewAsSkill("shangyi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShangyiCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        ShangyiCard *c = new ShangyiCard;
        c->setShowSkill(objectName());
        return c;
    }
};

NiaoxiangSummon::NiaoxiangSummon()
    : ArraySummonCard("niaoxiang")
{
}

class Niaoxiang : public BattleArraySkill {
public:
    Niaoxiang() : BattleArraySkill("niaoxiang", HegemonyMode::Siege) {
        events << TargetChosen;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (!player->hasShownSkill(this) || player->aliveCount() < 4) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) {
            for (int i = 0; i < use.to.length(); i++) {
                ServerPlayer *victim = use.to.at(i);
                if (use.from->inSiegeRelation(player, victim)) {
                    room->notifySkillInvoked(player, objectName());
                    room->broadcastSkillInvoke(objectName(), player);
                    QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();
                    if (jink_list.at(i).toInt() == 1)
                        jink_list.replace(i, QVariant(2));
                    use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
                }
            }
        }

        return QStringList();
    }
};

class Yicheng : public TriggerSkill {
public:
    Yicheng() : TriggerSkill("yicheng") {
        events << TargetConfirmed;
        frequency = Frequent;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash")) return skill_list;
        if (use.to.contains(player)) {
            foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
                if (p->isFriendWith(player) || p->willBeFriendWith(player))
                    skill_list.insert(p, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        if (ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player))){
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), player->objectName());
            room->broadcastSkillInvoke(objectName(), ask_who);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->drawCards(1);
        if (player->isAlive() && player->canDiscard(player, "he"))
            room->askForDiscard(player, objectName(), 1, 1, false, true);
        return false;
    }
};

QianhuanCard::QianhuanCard(){
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void QianhuanCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    source->tag["qianhuan_cancel"] = subcards.first();
}

class QianhuanVS : public OneCardViewAsSkill{
public:
    QianhuanVS() : OneCardViewAsSkill("qianhuan"){
        filter_pattern = ".|.|.|sorcery";
        response_pattern = "@@qianhuan";
        expand_pile = "sorcery";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QianhuanCard *c = new QianhuanCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class Qianhuan : public TriggerSkill {
public:
    Qianhuan() : TriggerSkill("qianhuan") {
        events << Damaged << TargetConfirming;
        view_as_skill = new QianhuanVS;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        QList<ServerPlayer *> yujis = room->findPlayersBySkillName(objectName());
        if (triggerEvent == Damaged && player->isAlive()) {
            foreach(ServerPlayer *yuji, yujis)
                if (yuji->isFriendWith(player) || yuji->willBeFriendWith(player))
                    skill_list.insert(yuji, QStringList(objectName()));
        } else if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card || use.card->getTypeId() == Card::TypeEquip
                || use.card->getTypeId() == Card::TypeSkill || !use.to.contains(player))
                return skill_list;
            if (use.to.length() != 1) return skill_list;
            foreach(ServerPlayer *yuji, yujis) {
                if (yuji->getPile("sorcery").isEmpty()) continue;
                if (yuji->isFriendWith(use.to.first()) || yuji->willBeFriendWith(use.to.first()))
                    skill_list.insert(yuji, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *yuji = ask_who;
        if (yuji == NULL)
            return false;
        yuji->tag["qianhuan_data"] = data;

        bool invoke = false;

        if (triggerEvent == Damaged){
            if (yuji->askForSkillInvoke(objectName(), "gethuan")){
                invoke = true;
                room->broadcastSkillInvoke(objectName(), yuji);
            }
        } else {
            QString prompt;
            QStringList prompt_list;
            prompt_list << "@qianhuan-cancel";
            CardUseStruct use = data.value<CardUseStruct>();
            prompt_list << "";
            prompt_list << use.to.first()->objectName();
            prompt_list << use.card->objectName();
            prompt = prompt_list.join(":");
            yuji->tag.remove("qianhuan_cancel");
            if (room->askForUseCard(yuji, "@@qianhuan", prompt, -1, Card::MethodNone)) {
                int id = yuji->tag["qianhuan_cancel"].toInt();
                yuji->tag.remove("qianhuan_cancel");
                CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                room->throwCard(Sanguosha->getCard(id), reason, NULL);

                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, yuji->objectName(), use.to.first()->objectName());
                invoke = true;
            }
        }

        yuji->tag.remove("qianhuan_data");

        return invoke;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *yuji = ask_who;
        if (!yuji) return false;
        if (triggerEvent == Damaged) {
            int id = room->drawCard();
            Card::Suit suit = Sanguosha->getCard(id)->getSuit();
            bool duplicate = false;
            foreach(int card_id, yuji->getPile("sorcery")) {
                if (Sanguosha->getCard(card_id)->getSuit() == suit) {
                    duplicate = true;
                    break;
                }
            }
            yuji->addToPile("sorcery", id);
            if (duplicate) {
                CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                room->throwCard(Sanguosha->getCard(id), reason, NULL);
            }
        } else if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();

            LogMessage log;
            if (use.from) {
                log.type = "$CancelTarget";
                log.from = use.from;
            }
            else {
                log.type = "$CancelTargetNoUser";
            }
            log.to = use.to;
            log.arg = use.card->objectName();
            room->sendLog(log);

            room->setEmotion(use.to.first(), "cancel");

            use.to.clear();
            data = QVariant::fromValue(use);
        }

        return false;
    }
};


class Zhendu : public TriggerSkill {
public:
    Zhendu() : TriggerSkill("zhendu") {
        events << EventPhaseStart;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        if (player->getPhase() != Player::Play) return skill_list;
        QList<ServerPlayer *> hetaihous = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *hetaihou, hetaihous) {
            if (hetaihou->canDiscard(hetaihou, "h") && hetaihou != player)
                skill_list.insert(hetaihou, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        ServerPlayer *hetaihou = ask_who;
        if (hetaihou && room->askForDiscard(hetaihou, objectName(), 1, 1, true, false, "@zhendu-discard")){
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, hetaihou->objectName(), player->objectName());
            room->notifySkillInvoked(player, objectName());
            room->broadcastSkillInvoke(objectName(), hetaihou);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        ServerPlayer *hetaihou = ask_who;

        if (!hetaihou) return false;

        Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
        analeptic->setSkillName("_zhendu");
        room->useCard(CardUseStruct(analeptic, player, QList<ServerPlayer *>(), true));
        if (player->isAlive())
            room->damage(DamageStruct(objectName(), hetaihou, player));

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *c) const{
        if (c->isKindOf("Analeptic"))
            return 0;
        return -1;
    }
};

class Qiluan : public TriggerSkill {
public:
    Qiluan() : TriggerSkill("qiluan") {
        events << Death << EventPhaseStart;
        frequency = Frequent;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return skill_list;

            if (!death.damage || !death.damage->from)
                return skill_list;

            ServerPlayer *current = room->getCurrent();
            if (current && (current->isAlive() || death.who == current) && current->getPhase() != Player::NotActive){
                foreach(ServerPlayer *p, room->getAllPlayers())
                    if (TriggerSkill::triggerable(p) && death.damage->from == p)
                        room->setPlayerMark(p, objectName(), 1);
            }

            return skill_list;
        }
        else {
            if (player->getPhase() == Player::NotActive){
                foreach(ServerPlayer *p, room->getAllPlayers()){
                    if (p->getMark(objectName()) > 0 && TriggerSkill::triggerable(p)) {
                        room->setPlayerMark(p, objectName(), 0);
                        if (p->isAlive())
                            skill_list.insert(p, QStringList(objectName()));
                    }
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        ServerPlayer *hetaihou = ask_who;
        if (hetaihou && hetaihou->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName(), hetaihou);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        ServerPlayer *hetaihou = ask_who;
        if (hetaihou)
            hetaihou->drawCards(3);

        return false;
    }
};


class Zhangwu : public TriggerSkill{
public:
    Zhangwu() : TriggerSkill("zhangwu"){
        events << CardsMoveOneTime << BeforeCardsMove;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to_place == Player::DrawPileBottom)
            return QStringList();
        int fldfid = -1;
        foreach(int id, move.card_ids){
            if (Sanguosha->getCard(id)->isKindOf("DragonPhoenix")){
                fldfid = id;
                break;
            }
        }

        if (fldfid == -1)
            return QStringList();

        if (triggerEvent == CardsMoveOneTime){
            if (move.to_place == Player::DiscardPile || (move.to_place == Player::PlaceEquip && move.to != player))
                return QStringList(objectName());
        }
        else if (triggerEvent == BeforeCardsMove){
            if ((move.from == player && (move.from_places[move.card_ids.indexOf(fldfid)] == Player::PlaceHand || move.from_places[move.card_ids.indexOf(fldfid)] == Player::PlaceEquip))
                && (move.to != player || (move.to_place != Player::PlaceHand && move.to_place != Player::PlaceEquip)))
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
            if (triggerEvent == CardsMoveOneTime){
                CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
                if (move.to != NULL)
                    room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), move.to->objectName());
            }

            room->broadcastSkillInvoke(objectName(), (triggerEvent == BeforeCardsMove) ? 1 : 2, player);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        const Card *dragonPhoenix = NULL;
        int dragonPhoenixId = -1;
        foreach(int id, move.card_ids){
            const Card *card = Sanguosha->getCard(id);
            if (card->isKindOf("DragonPhoenix")){
                dragonPhoenixId = id;
                dragonPhoenix = card;
                break;
            }
        }

        if (triggerEvent == CardsMoveOneTime){
            player->obtainCard(dragonPhoenix);
        }
        else {
            room->showCard(player, dragonPhoenixId);
            player->setFlags("fldf_removing");
            move.from_places.removeAt(move.card_ids.indexOf(dragonPhoenixId));
            move.card_ids.removeOne(dragonPhoenixId);
            data = QVariant::fromValue(move);

            room->moveCardTo(dragonPhoenix, NULL, Player::DrawPileBottom);
        }
        return false;
    }
};

class Zhangwu_Draw : public TriggerSkill{
public:
    Zhangwu_Draw() : TriggerSkill("#zhangwu-draw"){
        frequency = Compulsory;
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!(player != NULL && player->isAlive() && player->hasSkill("zhangwu")))
            return QStringList();

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to_place == Player::DrawPileBottom){
            int fldfid = -1;
            foreach(int id, move.card_ids){
                if (Sanguosha->getCard(id)->isKindOf("DragonPhoenix")){
                    fldfid = id;
                    break;
                }
            }

            if (fldfid == -1)
                return QStringList();

            if (player->hasFlag("fldf_removing")){
                player->setFlags("-fldf_removing");
                return QStringList(objectName());
            }

        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        return player->hasShownSkill(this);
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->drawCards(2);
        return false;
    }
};

class Shouyue : public TriggerSkill{
public:
    Shouyue() : TriggerSkill("shouyue$"){
        frequency = Compulsory;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const{
        return QStringList();
    }
};

class Jizhao : public TriggerSkill{
public:
    Jizhao() : TriggerSkill("jizhao"){
        events << AskForPeaches;
        frequency = Limited;
        limit_mark = "@jizhao";
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (player->getMark("@jizhao") == 0 || player->getHp() > 0)
            return QStringList();

        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who != player)
            return QStringList();

        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName(), data)){
            player->broadcastSkillInvoke(objectName());
            room->doSuperLightbox("lord_liubei", objectName());
            player->loseMark(limit_mark);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->getHandcardNum() < player->getMaxHp())
            room->drawCards(player, player->getMaxHp() - player->getHandcardNum());

        if (player->getHp() < 2){
            RecoverStruct rec;
            rec.recover = 2 - player->getHp();
            rec.who = player;
            room->recover(player, rec);
        }

        room->handleAcquireDetachSkills(player, "-shouyue|rende");
        return false; //return player->getHp() > 0 || player->isDead();
    }
};


FormationPackage::FormationPackage()
    : Package("formation")
{
    General *dengai = new General(this, "dengai", "wei"); // WEI 015
    dengai->addSkill(new Tuntian);
    dengai->addSkill(new TuntianPostpone);
    dengai->addSkill(new TuntianGotoField);
    dengai->addSkill(new TuntianDistance);
    dengai->addSkill(new Jixi);
    dengai->setHeadMaxHpAdjustedValue(-1);
    dengai->addSkill(new Ziliang);
    insertRelatedSkills("tuntian", 3, "#tuntian-dist", "#tuntian-postpone", "#tuntian-gotofield");

    General *caohong = new General(this, "caohong", "wei"); // WEI 018
    caohong->addCompanion("caoren");
    caohong->addSkill(new Huyuan);
    caohong->addSkill(new Heyi);
    caohong->addSkill(new HeyiFeiying);
    insertRelatedSkills("heyi", "#heyi_feiying");

    General *jiangwei = new General(this, "jiangwei", "shu"); // SHU 012 G
    jiangwei->addSkill(new Tiaoxin);
    jiangwei->addSkill(new YiZhi);
    jiangwei->setDeputyMaxHpAdjustedValue(-1);
    jiangwei->addSkill(new Tianfu);

    General *jiangwanfeiyi = new General(this, "jiangwanfeiyi", "shu", 3); // SHU 018
    jiangwanfeiyi->addSkill(new Shengxi);
    jiangwanfeiyi->addSkill(new Shoucheng);

    General *jiangqin = new General(this, "jiangqin", "wu"); // WU 017
    jiangqin->addCompanion("zhoutai");
    jiangqin->addSkill(new Shangyi);
    jiangqin->addSkill(new Niaoxiang);

    General *xusheng = new General(this, "xusheng", "wu"); // WU 020
    xusheng->addCompanion("dingfeng");
    xusheng->addSkill(new Yicheng);

    General *yuji = new General(this, "yuji", "qun", 3); // QUN 011 G
    yuji->addSkill(new Qianhuan);

    General *hetaihou = new General(this, "hetaihou", "qun", 3, false); // QUN 020
    hetaihou->addSkill(new Zhendu);
    hetaihou->addSkill(new Qiluan);

    General *liubei = new General(this, "lord_liubei$", "shu", 4, true, true);
    liubei->addSkill(new Zhangwu);
    liubei->addSkill(new Zhangwu_Draw);
    insertRelatedSkills("zhangwu", "#zhangwu-draw");
    liubei->addSkill(new Shouyue);
    liubei->addSkill(new Jizhao);

    addMetaObject<HuyuanCard>();
    addMetaObject<ZiliangCard>();
    addMetaObject<TiaoxinCard>();
    addMetaObject<ShangyiCard>();
    addMetaObject<HeyiSummon>();
    addMetaObject<TianfuSummon>();
    addMetaObject<NiaoxiangSummon>();
    addMetaObject<QianhuanCard>();

    skills << new Feiying;
}

ADD_PACKAGE(Formation)


DragonPhoenix::DragonPhoenix(Suit suit, int number) : Weapon(suit, number, 2)
{
    setObjectName("DragonPhoenix");
}

class DragonPhoenixSkill : public WeaponSkill{
public:
    DragonPhoenixSkill() : WeaponSkill("DragonPhoenix"){
        events << TargetChosen;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from == player && use.card->isKindOf("Slash")){
            foreach(ServerPlayer *to, use.to){
                if (to->canDiscard(to, "he") && player->askForSkillInvoke(objectName(), QVariant::fromValue(to))){
                    room->setEmotion(use.from, "weapon/dragonphoenix");
                    room->askForDiscard(to, objectName(), 1, 1, false, true, "@dragonphoenix-discard");
                }
            }
        }
        return false;
    }
};

class DragonPhoenixSkill2 : public WeaponSkill{
public:
    DragonPhoenixSkill2() : WeaponSkill("#DragonPhoenix"){
        events << BuryVictim;
    }

    virtual int getPriority() const{
        return -4;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        ServerPlayer *dfowner = NULL;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (p->hasWeapon("DragonPhoenix")) {
                dfowner = p;
                break;
            }
        }
        if (dfowner == NULL || dfowner->getRole() == "careerist" || !dfowner->hasShownOneGeneral())
            return false;

        DeathStruct death = data.value<DeathStruct>();
        DamageStruct *damage = death.damage;
        if (!damage || !damage->from || damage->from != dfowner) return false;
        if (!damage->card || !damage->card->isKindOf("Slash")) return false;

        QStringList kingdom_list = Sanguosha->getKingdoms();
        kingdom_list << "careerist";
        bool broken = false;
        int n = dfowner->getPlayerNumWithSameKingdom("DragonPhoenix", QString(), MaxCardsType::Min); // could be canceled later
        foreach (QString kingdom, Sanguosha->getKingdoms()) {
            if (kingdom == "god") continue;
            if (dfowner->getRole() == "careerist") {
                if (kingdom == "careerist")
                    continue;
            } else if (dfowner->getKingdom() == kingdom)
                continue;
            int other_num = dfowner->getPlayerNumWithSameKingdom("DragonPhoenix", kingdom, MaxCardsType::Normal);
            if (other_num > 0 && other_num < n) {
                broken = true;
                break;
            }
        }

        if (broken)
            return false;

        QStringList generals = Sanguosha->getLimitedGeneralNames();
        QStringList avaliable_generals;

        foreach(QString general, generals){
            if (Sanguosha->getGeneral(general)->getKingdom() != dfowner->getKingdom())
                continue;

            bool continue_flag = false;
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                QStringList generals_of_player = room->getTag(p->objectName()).toStringList();
                if (generals_of_player.contains(general)){
                    continue_flag = true;
                    break;
                }
            }

            if (continue_flag)
                continue;

            avaliable_generals << general;
        }

        if (avaliable_generals.isEmpty())
            return false;

        int aidelay = Config.AIDelay;
        Config.AIDelay = 0;
        bool invoke = room->askForSkillInvoke(dfowner, "DragonPhoenix", data) && room->askForSkillInvoke(player, "DragonPhoenix", "revive");
        Config.AIDelay = aidelay;
        if (invoke){
            room->setEmotion(dfowner, "weapon/dragonphoenix");
            room->setPlayerProperty(player, "Duanchang", "");
            QString to_change = room->askForGeneral(player, avaliable_generals, QString(), true, "DragonPhoenix", dfowner->getKingdom());

            if (!to_change.isEmpty()){
                QStringList change_list;
                change_list << to_change;

                player->removeGeneral(false);
                foreach(const Skill *skill, player->getSkills())
                    player->loseSkill(skill->objectName());
                player->detachAllSkills();
                room->setPlayerProperty(player, "general1_showed", true);
                foreach(const Skill *skill, Sanguosha->getGeneral(to_change)->getSkillList(true, true)) {
                    player->addSkill(skill->objectName());
                    JsonArray args;
                    args << QSanProtocol::S_GAME_EVENT_ADD_SKILL;
                    args << player->objectName();
                    args << skill->objectName();
                    args << true;
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
                }
                room->changeHero(player, to_change, false, true, false, true);
                player->setSkillsPreshowed("h");

                room->setPlayerProperty(player, "actual_general1", to_change);

                change_list << player->getActualGeneral2Name();

                room->revivePlayer(player);
                room->setPlayerFlag(player, "Global_DFDebut");

                room->setPlayerProperty(player, "hp", 2);

                player->setChained(false);
                room->broadcastProperty(player, "chained");

                player->setFaceUp(true);
                room->broadcastProperty(player, "faceup");

                room->setTag(player->objectName(), change_list);

                room->setPlayerProperty(player, "kingdom", dfowner->getKingdom());
                room->setPlayerProperty(player, "role", HegemonyMode::GetMappedRole(dfowner->getKingdom()));

                foreach(const Skill *skill, Sanguosha->getGeneral(to_change)->getSkillList(true, true)) {
                    if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty())
                        room->setPlayerMark(player, skill->getLimitMark(), 1);
                }

                player->drawCards(1);
            }
        }
        return false;
    }
};


FormationEquipPackage::FormationEquipPackage() : Package("formation_equip", CardPack){
    DragonPhoenix *dp = new DragonPhoenix();
    dp->setParent(this);

    skills << new DragonPhoenixSkill << new DragonPhoenixSkill2;
    insertRelatedSkills("DragonPhoenix", "#DragonPhoenix");
}

ADD_PACKAGE(FormationEquip)


