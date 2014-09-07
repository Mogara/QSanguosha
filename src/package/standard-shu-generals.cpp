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

#include "standard-shu-generals.h"
#include "structs.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "client.h"
#include "engine.h"
#include "util.h"

RendeCard::RendeCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void RendeCard::extraCost(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *target = card_use.to.first();
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, card_use.from->objectName(), target->objectName(), "rende", QString());
    room->obtainCard(target, this, reason, false);
}

void RendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if (old_value < 3 && new_value >= 3 && source->isWounded()) {
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

class RendeViewAsSkill : public ViewAsSkill {
public:
    RendeViewAsSkill() : ViewAsSkill("rende") {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        RendeCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        rende_card->setShowSkill(objectName());
        return rende_card;
    }
};

class Rende : public TriggerSkill {
public:
    Rende() : TriggerSkill("rende") {
        events << EventPhaseChanging;
        view_as_skill = new RendeViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer * &) const{
        if (target->getMark("rende") > 0) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                room->setPlayerMark(target, "rende", 0);
        }
        return QStringList();
    }
};

class Wusheng : public OneCardViewAsSkill {
public:
    Wusheng() : OneCardViewAsSkill("wusheng") {
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *card) const{
        const Player *lord = Self->getLord();
        if (lord == NULL || !lord->hasLordSkill("shouyue") || !lord->hasShownGeneral1())
            if (!card->isRed())
                return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            Slash *slash = new Slash(Card::SuitToBeDecided, -1);
            slash->addSubcard(card->getEffectiveId());
            slash->deleteLater();
            return slash->isAvailable(Self);
        }
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard->getId());
        slash->setSkillName(objectName());
        slash->setShowSkill(objectName());
        return slash;
    }
};

class Paoxiao : public TargetModSkill {
public:
    Paoxiao() : TargetModSkill("paoxiao") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class PaoxiaoArmorNullificaion : public TriggerSkill{
public:
    PaoxiaoArmorNullificaion() : TriggerSkill("#paoxiao-null"){
        events << TargetChosen;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!(player != NULL && player->isAlive() && player->hasSkill("paoxiao")))
            return QStringList();

        ServerPlayer *lord = room->getLord(player->getKingdom());
        if (lord != NULL && lord->hasLordSkill("shouyue") && lord->hasShownGeneral1()){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash") && use.from == player)
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->hasShownSkill("paoxiao"))
            return true;
        else {
            player->tag["paoxiao_use"] = data;
            bool invoke = player->askForSkillInvoke("paoxiao", "armor_nullify");
            player->tag.remove("paoxiao_use");
            if (invoke){
                player->showGeneral(player->inHeadSkills("paoxiao"));
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        ServerPlayer *lord = room->getLord(player->getKingdom());
        room->notifySkillInvoked(lord, "shouyue");
        CardUseStruct use = data.value<CardUseStruct>();
        foreach (ServerPlayer *p, use.to.toSet())
            p->addQinggangTag(use.card);
        return false;
    }
};

class Guanxing : public PhaseChangeSkill {
public:
    Guanxing() : PhaseChangeSkill("guanxing") {
        frequency = Frequent;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return TriggerSkill::triggerable(player) && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (!player->hasSkill("guanxing")){
            if (player->askForSkillInvoke(objectName())) {
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                room->broadcastSkillInvoke("yizhi", player);
                player->showGeneral(false);
                return true;
            } else {
                return false;
            }
        }
        if (!player->hasSkill("yizhi")){
            if (player->askForSkillInvoke(objectName())) {
                room->broadcastSkillInvoke(objectName(), player);
                return true;
            } else {
                return false;
            }
        }
        // if it runs here, it means player own both two skill;
        if (player->askForSkillInvoke(objectName())) {
            bool show1 = player->hasShownSkill(this);
            bool show2 = player->hasShownSkill("yizhi");
            QStringList choices;
            if (!show1)
                choices << "show_head_general";
            if (!show2)
                choices << "show_deputy_general";
            if (choices.length() == 2)
                choices << "show_both_generals";
            if (choices.length() != 3)
                choices << "cancel";
            QString choice = room->askForChoice(player, "TurnStartShowGeneral", choices.join("+"));
            if (choice == "cancel") {
                if (show1) {
                    room->broadcastSkillInvoke(objectName(), player);
                    return true;
                } else {
                    room->broadcastSkillInvoke("yizhi", player);
                    onPhaseChange(player);
                    return false;
                }
            }
            if (choice != "show_head_general")
                player->showGeneral(false);
            if (choice == "show_deputy_general" && !show1) {
                room->broadcastSkillInvoke("yizhi", player);
                player->showGeneral(false);
                onPhaseChange(player);
                return false;
            } else {
                room->broadcastSkillInvoke(objectName(), player);
                return true;
            }
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        Room *room = zhuge->getRoom();
        QList<int> guanxing = room->getNCards(getGuanxingNum(zhuge));

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = zhuge;
        log.card_str = IntList2StringList(guanxing).join("+");
        room->doNotify(zhuge, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());

        room->askForGuanxing(zhuge, guanxing, Room::GuanxingBothSides);


        return false;
    }

    virtual int getGuanxingNum(ServerPlayer *zhuge) const{
        if (zhuge->hasShownSkill(this) && zhuge->hasShownSkill("yizhi")) return 5;
        return qMin(5, zhuge->aliveCount());
    }
};

class Kongcheng : public TriggerSkill{
public:
    Kongcheng() : TriggerSkill("kongcheng"){
        events << TargetConfirming;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (TriggerSkill::triggerable(player) && player->isKongcheng()){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && (use.card->isKindOf("Slash") || use.card->isKindOf("Duel")) && use.to.contains(player)){
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->hasShownSkill(this) || player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        room->notifySkillInvoked(player, objectName());
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

class LongdanVS : public OneCardViewAsSkill {
public:
    LongdanVS() : OneCardViewAsSkill("longdan") {
        response_or_use = true;
    }

    virtual bool viewFilter(const Card *to_select) const{
        const Card *card = to_select;

        switch (Sanguosha->currentRoomState()->getCurrentCardUseReason()) {
        case CardUseStruct::CARD_USE_REASON_PLAY: {
            return card->isKindOf("Jink");
        }
        case CardUseStruct::CARD_USE_REASON_RESPONSE:
        case CardUseStruct::CARD_USE_REASON_RESPONSE_USE: {
            QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            if (pattern == "slash")
                return card->isKindOf("Jink");
            else if (pattern == "jink")
                return card->isKindOf("Slash");
        }
        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        if (originalCard->isKindOf("Slash")) {
            Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
            jink->addSubcard(originalCard);
            jink->setSkillName(objectName());
            jink->setShowSkill(objectName());
            return jink;
        }
        else if (originalCard->isKindOf("Jink")) {
            Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
            slash->addSubcard(originalCard);
            slash->setSkillName(objectName());
            slash->setShowSkill(objectName());
            return slash;
        }
        else
            return NULL;
    }
};

class Longdan : public TriggerSkill{
public:
    Longdan() : TriggerSkill("longdan"){
        view_as_skill = new LongdanVS;
        events << CardUsed << CardResponded;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        ServerPlayer *lord = room->getLord(player->getKingdom());
        if (lord != NULL && lord->hasLordSkill("shouyue") && lord->hasShownGeneral1()){
            const Card *card = NULL;
            if (triggerEvent == CardUsed)
                card = data.value<CardUseStruct>().card;
            else
                card = data.value<CardResponseStruct>().m_card;

            if (card != NULL && card->getSkillName() == "longdan")
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const{
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        ServerPlayer *lord = room->getLord(player->getKingdom());
        room->notifySkillInvoked(lord, "shouyue");
        player->drawCards(1);
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (card->isKindOf("Slash")){
            return 1;
        }
        else {
            return 2;
        }
    }
};

Mashu::Mashu(const QString &owner) : DistanceSkill("mashu_" + owner) {
}

int Mashu::getCorrect(const Player *from, const Player *) const{
    if (from->hasSkill(objectName()) && from->hasShownSkill(this))
        return -1;
    else
        return 0;
}

class Tieqi : public TriggerSkill {
public:
    Tieqi() : TriggerSkill("tieqi") {
        events << TargetChosen;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (TriggerSkill::triggerable(use.from) && use.card != NULL && use.card->isKindOf("Slash") && use.to.contains(player)){
            ask_who = use.from;
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player))) {
            room->broadcastSkillInvoke(objectName(), 1, ask_who);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *machao = ask_who;
        QVariantList jink_list = machao->tag["Jink_" + use.card->toString()].toList();

        doTieqi(player, machao, use, jink_list);

        machao->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }

private:
    static void doTieqi(ServerPlayer *target, ServerPlayer *source, CardUseStruct use, QVariantList &jink_list){
        Room *room = target->getRoom();

        int index = use.to.indexOf(target);

        JudgeStruct judge;

        ServerPlayer *lord = room->getLord(source->getKingdom());
        bool has_lord = false;
        if (lord != NULL && lord->hasLordSkill("shouyue") && lord->hasShownGeneral1()){
            has_lord = true;
            judge.pattern = ".|spade";
            judge.good = false;
        } else {
            judge.pattern = ".|red";
            judge.good = true;
        }
        judge.reason = "tieqi";
        judge.who = source;

        if (has_lord)
            room->notifySkillInvoked(lord, "shouyue");

        target->setFlags("TieqiTarget"); //for AI
        room->judge(judge);
        target->setFlags("-TieqiTarget");

        if (judge.isGood()) {
            LogMessage log;
            log.type = "#NoJink";
            log.from = target;
            room->sendLog(log);

            jink_list.replace(index, QVariant(0));
            room->broadcastSkillInvoke("tieqi", 2, source);
        }
    }
};

class Jizhi : public TriggerSkill {
public:
    Jizhi() : TriggerSkill("jizhi") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card != NULL && use.card->isNDTrick()) {
            if (!use.card->isVirtualCard() || use.card->getSubcards().isEmpty())
                return QStringList(objectName());
            else if (use.card->getSubcards().length() == 1) {
                if (Sanguosha->getCard(use.card->getEffectiveId())->objectName() == use.card->objectName())
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName(), data)){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->drawCards(1);
        return false;
    }
};

class Qicai : public TargetModSkill {
public:
    Qicai() : TargetModSkill("qicai") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class Liegong : public TriggerSkill {
public:
    Liegong() : TriggerSkill("liegong") {
        events << TargetChosen;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (TriggerSkill::triggerable(use.from) && use.from->getPhase() == Player::Play && use.card != NULL && use.card->isKindOf("Slash") && use.to.contains(player)){
            int handcard_num = player->getHandcardNum();
            if (handcard_num >= use.from->getHp() || handcard_num <= use.from->getAttackRange()){
                ask_who = use.from;
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player))) {
            room->broadcastSkillInvoke(objectName(), 1, ask_who);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *huangzhong = ask_who;
        QVariantList jink_list = huangzhong->tag["Jink_" + use.card->toString()].toList();

        doLiegong(player, use, jink_list);

        huangzhong->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }

private:
    static void doLiegong(ServerPlayer *target, CardUseStruct use, QVariantList &jink_list) {
        int index = use.to.indexOf(target);
        LogMessage log;
        log.type = "#NoJink";
        log.from = target;
        target->getRoom()->sendLog(log);
        jink_list.replace(index, QVariant(0));
    }
};

class LiegongRange : public AttackRangeSkill{
public:
    LiegongRange() : AttackRangeSkill("#liegong-for-lord"){
    }

    virtual int getExtra(const Player *target, bool) const{
        if (target->hasShownSkill("liegong")){
            const Player *lord = target->getLord();

            if (lord != NULL && lord->hasLordSkill("shouyue") && lord->hasShownGeneral1()){
                return 1;
            }
        }
        return 0;
    }
};


class KuangguRecord : public TriggerSkill{
public:
    KuangguRecord() : TriggerSkill("#kuanggu-record"){
        events << PreDamageDone;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (player != NULL){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *weiyan = damage.from;
            if (weiyan != NULL){
                if (weiyan->distanceTo(damage.to) != -1 && weiyan->distanceTo(damage.to) <= 1)
                    weiyan->tag["InvokeKuanggu"] = damage.damage;
                else
                    weiyan->tag.remove("InvokeKuanggu");
            }
        }

        return QStringList();
    }
};

class Kuanggu : public TriggerSkill {
public:
    Kuanggu() : TriggerSkill("kuanggu") {
        frequency = Compulsory;
        events << Damage;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (TriggerSkill::triggerable(player)){
            bool ok = false;
            int recorded_damage = player->tag["InvokeKuanggu"].toInt(&ok);
            if (ok && recorded_damage > 0 && player->isWounded()) {
                QStringList skill_list;
                DamageStruct damage = data.value<DamageStruct>();
                for (int i = 0; i < damage.damage; i++)
                    skill_list << objectName();
                return skill_list;
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke) {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);

        return false;
    }
};


class Lianhuan : public OneCardViewAsSkill {
public:
    Lianhuan() : OneCardViewAsSkill("lianhuan") {
        filter_pattern = ".|club|.|hand";
        response_or_use = true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IronChain *chain = new IronChain(originalCard->getSuit(), originalCard->getNumber());
        chain->addSubcard(originalCard);
        chain->setSkillName(objectName());
        chain->setShowSkill(objectName());
        return chain;
    }
};

class Niepan : public TriggerSkill {
public:
    Niepan() : TriggerSkill("niepan") {
        events << AskForPeaches;
        frequency = Limited;
        limit_mark = "@nirvana";
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *target, QVariant &data, ServerPlayer * &) const {
        if (TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0){
            DyingStruct dying_data = data.value<DyingStruct>();

            if (target->getHp() > 0)
                return QStringList();

            if (dying_data.who != target)
                return QStringList();
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *pangtong, QVariant &data, ServerPlayer *) const{
        if (pangtong->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName(), pangtong);
            room->doSuperLightbox("pangtong", objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *pangtong, QVariant &, ServerPlayer *) const{
        room->removePlayerMark(pangtong, "@nirvana");
        pangtong->throwAllHandCardsAndEquips();
        QList<const Card *> tricks = pangtong->getJudgingArea();
        foreach(const Card *trick, tricks) {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, pangtong->objectName());
            room->throwCard(trick, reason, NULL);
        }

        RecoverStruct recover;
        recover.recover = qMin(3, pangtong->getMaxHp()) - pangtong->getHp();
        room->recover(pangtong, recover);

        pangtong->drawCards(3);

        if (pangtong->isChained())
            room->setPlayerProperty(pangtong, "chained", false);

        if (!pangtong->faceUp())
            pangtong->turnOver();

        return false; //return pangtong->getHp() > 0 || pangtong->isDead();
    }
};

class Huoji : public OneCardViewAsSkill {
public:
    Huoji() : OneCardViewAsSkill("huoji") {
        filter_pattern = ".|red|.|hand";
        response_or_use = true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FireAttack *fire_attack = new FireAttack(originalCard->getSuit(), originalCard->getNumber());
        fire_attack->addSubcard(originalCard->getId());
        fire_attack->setSkillName(objectName());
        fire_attack->setShowSkill(objectName());
        return fire_attack;
    }
};

class Bazhen : public TriggerSkill {
public:
    Bazhen() : TriggerSkill("bazhen") {
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        QString pattern = data.toStringList().first();
        if (pattern != "jink")
            return QStringList();

        if (!player->tag["Qinggang"].toStringList().isEmpty() || player->getMark("Armor_Nullified") > 0
            || player->getMark("Equips_Nullified_to_Yourself") > 0)
            return QStringList();

        if (player->hasArmorEffect("bazhen"))
            return QStringList("EightDiagram");

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const{
        return false;
    }
};

class Kanpo : public OneCardViewAsSkill {
public:
    Kanpo() : OneCardViewAsSkill("kanpo") {
        filter_pattern = ".|black|.|hand";
        response_pattern = "nullification";
        response_or_use = true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());
        ncard->setShowSkill(objectName());
        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return !player->isKongcheng() || !player->getPile("wooden_ox").isEmpty();
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        if (!player->hasInnateSkill(objectName()) && player->hasSkill("tianfu"))
            return (qrand() % 2 + 3);
        else
            return (qrand() % 2 + 1);
    }
};

SavageAssaultAvoid::SavageAssaultAvoid(const QString &avoid_skill) : TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill){
    events << CardEffected;
    frequency = Compulsory;
}

QStringList SavageAssaultAvoid::triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
    if (!player || !player->isAlive() || !player->hasSkill(avoid_skill)) return QStringList();
    CardEffectStruct effect = data.value<CardEffectStruct>();
    if (effect.card->isKindOf("SavageAssault"))
        return QStringList(objectName());

    return QStringList();
}

bool SavageAssaultAvoid::cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
    if (player->hasShownSkill(avoid_skill)) return true;
    if (player->askForSkillInvoke(avoid_skill)) {
        room->broadcastSkillInvoke(avoid_skill, 1, player);
        player->showGeneral(player->inHeadSkills(avoid_skill));
        return true;
    }
    return false;
}

bool SavageAssaultAvoid::effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
    LogMessage log;
    log.type = "#SkillNullify";
    log.from = player;
    log.arg = avoid_skill;
    log.arg2 = "savage_assault";
    room->sendLog(log);

    return true;
}

class Huoshou : public TriggerSkill {
public:
    Huoshou() : TriggerSkill("huoshou") {
        events << TargetChosen << ConfirmDamage << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player == NULL) return QStringList();
        if (triggerEvent == TargetChosen && TriggerSkill::triggerable(player)) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault") && use.from != player)
                return QStringList(objectName());
        }
        else if (triggerEvent == ConfirmDamage && !room->getTag("HuoshouSource").isNull()) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->isKindOf("SavageAssault"))
                return QStringList();

            ServerPlayer *menghuo = room->getTag("HuoshouSource").value<ServerPlayer *>();
            damage.from = menghuo->isAlive() ? menghuo : NULL;
            data = QVariant::fromValue(damage);
        }
        else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault"))
                room->removeTag("HuoshouSource");
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName(), 2, player);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());
        room->setTag("HuoshouSource", QVariant::fromValue((ServerPlayer *)player));

        return false;
    }
};

class Zaiqi : public PhaseChangeSkill {
public:
    Zaiqi() : PhaseChangeSkill("zaiqi") {
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *menghuo, QVariant &, ServerPlayer* &) const{
        if (!PhaseChangeSkill::triggerable(menghuo))
            return QStringList();

        if (menghuo->getPhase() == Player::Draw && menghuo->isWounded())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName(), 1, player);
            return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *menghuo) const{
        Room *room = menghuo->getRoom();

        bool has_heart = false;
        int x = menghuo->getLostHp();
        QList<int> ids = room->getNCards(x, false);
        CardsMoveStruct move(ids, menghuo, Player::PlaceTable,
            CardMoveReason(CardMoveReason::S_REASON_TURNOVER, menghuo->objectName(), "zaiqi", QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        QList<int> card_to_throw;
        QList<int> card_to_gotback;
        for (int i = 0; i < x; i++) {
            if (Sanguosha->getCard(ids[i])->getSuit() == Card::Heart)
                card_to_throw << ids[i];
            else
                card_to_gotback << ids[i];
        }
        if (!card_to_throw.isEmpty()) {
            DummyCard dummy(card_to_throw);

            RecoverStruct recover;
            recover.who = menghuo;
            recover.recover = card_to_throw.length();
            room->recover(menghuo, recover);

            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, menghuo->objectName(), "zaiqi", QString());
            room->throwCard(&dummy, reason, NULL);
            has_heart = true;
        }
        if (!card_to_gotback.isEmpty()) {
            DummyCard dummy2(card_to_gotback);
            CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, menghuo->objectName());
            room->obtainCard(menghuo, &dummy2, reason);
        }

        if (has_heart)
            room->broadcastSkillInvoke(objectName(), 2, menghuo);

        return true;
    }
};

class Juxiang : public TriggerSkill {
public:
    Juxiang() : TriggerSkill("juxiang") {
        events << CardUsed << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player == NULL) return QStringList();
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault")) {
                if (use.card->isVirtualCard() && use.card->subcardsLength() != 1)
                    return QStringList();
                if (Sanguosha->getEngineCard(use.card->getEffectiveId())
                    && Sanguosha->getEngineCard(use.card->getEffectiveId())->isKindOf("SavageAssault"))
                    room->setCardFlag(use.card->getEffectiveId(), "real_SA");
            }
        }
        else if (TriggerSkill::triggerable(player)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.card_ids.length() == 1 && move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
                && move.reason.m_reason == CardMoveReason::S_REASON_USE && room->getCardPlace(move.card_ids[0]) == Player::DiscardPile) {
                Card *card = Sanguosha->getCard(move.card_ids.first());
                if (card->hasFlag("real_SA") && player != move.from)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName(), 2, player);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        room->notifySkillInvoked(player, objectName());
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        DummyCard sa(move.card_ids);
        player->obtainCard(&sa);

        return false;
    }
};

class Lieren : public TriggerSkill {
public:
    Lieren() : TriggerSkill("lieren") {
        events << Damage;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *zhurong, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(zhurong)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash") && !zhurong->isKongcheng()
            && !damage.to->isKongcheng() && damage.to != zhurong && !damage.chain && !damage.transfer && !damage.to->hasFlag("Global_DFDebut"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *zhurong, QVariant &data, ServerPlayer *) const{
        if (zhurong->askForSkillInvoke(objectName(), data)){
            DamageStruct damage = data.value<DamageStruct>();
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, zhurong->objectName(), damage.to->objectName());
            room->broadcastSkillInvoke(objectName(), 1, zhurong);
            PindianStruct *pd = zhurong->pindianSelect(damage.to, "lieren");
            zhurong->tag["lieren_pd"] = QVariant::fromValue(pd);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *zhurong, QVariant &, ServerPlayer *) const{
        PindianStruct *pd = zhurong->tag["lieren_pd"].value<PindianStruct *>();
        zhurong->tag.remove("lieren_pd");
        if (pd != NULL){
            ServerPlayer *target = pd->to;

            bool success = zhurong->pindian(pd);
            pd = NULL;
            if (!success) return false;

            room->broadcastSkillInvoke(objectName(), 2, zhurong);
            if (!target->isNude()) {
                int card_id = room->askForCardChosen(zhurong, target, "he", objectName());
                CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, zhurong->objectName());
                room->obtainCard(zhurong, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
            }
        }
        else
            Q_ASSERT(false);

        return false;
    }
};

class Xiangle : public TriggerSkill {
public:
    Xiangle() : TriggerSkill("xiangle") {
        events << SlashEffected << TargetConfirming << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == TargetConfirming){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash") && TriggerSkill::triggerable(player) && use.to.contains(player))
                return QStringList(objectName());
        }
        else if (triggerEvent == SlashEffected){
            if (!TriggerSkill::triggerable(player)) return QStringList();
            if (player->getMark("xiangle") > 0) {
                if (!player->hasShownSkill(this))
                    return QStringList();
                return QStringList(objectName());
            }
        }
        else if (triggerEvent == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            foreach(ServerPlayer *p, use.to){
                p->setMark("xiangle", 0);
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == SlashEffected)
            return true;
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName(), data);
        if (invoke){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *liushan, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == SlashEffected){
            liushan->removeMark("xiangle");
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            LogMessage log;
            log.type = "#DanlaoAvoid";
            log.from = effect.to;
            log.arg2 = objectName();
            log.arg = effect.slash->objectName();
            room->sendLog(log);
            return true;
        }

        CardUseStruct use = data.value<CardUseStruct>();
        room->notifySkillInvoked(liushan, objectName());

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = liushan;
        log.arg = objectName();
        room->sendLog(log);

        QVariant dataforai = QVariant::fromValue(liushan);
        if (!room->askForCard(use.from, ".Basic", "@xiangle-discard:" + liushan->objectName(), dataforai))
            liushan->addMark("xiangle");

        return false;
    }
};

FangquanCard::FangquanCard() {
}

bool FangquanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FangquanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    ServerPlayer *liushan = effect.from, *player = effect.to;

    LogMessage log;
    log.type = "#Fangquan";
    log.from = liushan;
    log.to << player;
    room->sendLog(log);

    room->setTag("FangquanTarget", QVariant::fromValue((ServerPlayer *)player));
}

class FangquanViewAsSkill : public OneCardViewAsSkill {
public:
    FangquanViewAsSkill() : OneCardViewAsSkill("fangquan") {
        filter_pattern = ".|.|.|hand!";
        response_pattern = "@@fangquan";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FangquanCard *fangquan = new FangquanCard;
        fangquan->addSubcard(originalCard);
        fangquan->setShowSkill(objectName());
        return fangquan;
    }
};

class Fangquan : public TriggerSkill {
public:
    Fangquan() : TriggerSkill("fangquan") {
        events << EventPhaseChanging;
        view_as_skill = new FangquanViewAsSkill;
    }

    virtual bool canPreshow() const{
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Play){
            return (player->isSkipped(Player::Play)) ? QStringList() : QStringList(objectName());
        }
        else if (change.to == Player::NotActive){
            return (player->hasFlag(objectName()) && player->canDiscard(player, "h")) ? QStringList(objectName()) : QStringList();
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Play){
            if (player->askForSkillInvoke(objectName())){
                player->skip(Player::Play);
                room->broadcastSkillInvoke(objectName(), 1, player);
                return true;
            }
        } else if (change.to == Player::NotActive)
            room->askForUseCard(player, "@@fangquan", "@fangquan-discard", -1, Card::MethodDiscard);
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *liushan, QVariant &data, ServerPlayer *) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Play){
            liushan->setFlags(objectName());
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 2;
    }
};

class FangquanGive : public PhaseChangeSkill {
public:
    FangquanGive() : PhaseChangeSkill("#fangquan-give") {
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer * &ask_who) const{
        if (player != NULL && player->isAlive()) {
            if (!room->getTag("FangquanTarget").isNull()){
                ServerPlayer *target = room->getTag("FangquanTarget").value<ServerPlayer *>();
                ask_who = target;
                return (target->isAlive()) ? QStringList(objectName()) : QStringList();
            }
        }
        return QStringList();
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        Room *room = liushan->getRoom();
        ServerPlayer *target = room->getTag("FangquanTarget").value<ServerPlayer *>();
        room->removeTag("FangquanTarget");
        target->gainAnExtraTurn();
        return false;
    }
};

class Shushen : public TriggerSkill {
public:
    Shushen() : TriggerSkill("shushen") {
        events << HpRecover;
    }

    virtual bool canPreshow() const{
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (TriggerSkill::triggerable(player)) {
            QList<ServerPlayer *> friends;
            foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                if (player->isFriendWith(p) || player->willBeFriendWith(p))
                    friends << p;
            }

            if (friends.isEmpty()) return QStringList();

            QStringList trigger_list;
            RecoverStruct recover = data.value<RecoverStruct>();
            for (int i = 1; i <= recover.recover; i++){
                trigger_list << objectName();
            }

            return trigger_list;
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> friends;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if (player->isFriendWith(p) || player->willBeFriendWith(p))
                friends << p;
        }

        if (friends.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, friends, objectName(), "shushen-invoke", true, true);
        if (target != NULL){
            room->broadcastSkillInvoke(objectName(), player);

            QStringList target_list = player->tag["shushen_target"].toStringList();
            target_list.append(target->objectName());
            player->tag["shushen_target"] = target_list;

            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        QStringList target_list = player->tag["shushen_target"].toStringList();
        QString target_name = target_list.last();
        target_list.removeLast();
        player->tag["shushen_target"] = target_list;

        ServerPlayer *to = NULL;

        foreach(ServerPlayer *p, player->getRoom()->getPlayers()){
            if (p->objectName() == target_name){
                to = p;
                break;
            }
        }
        if (to != NULL)
            to->drawCards(1);
        return false;
    }
};

class Shenzhi : public PhaseChangeSkill {
public:
    Shenzhi() : PhaseChangeSkill("shenzhi") {
        frequency = Frequent;
        //This skill can't be frequent in game actually.
        //because the frequency = Frequent has no effect in UI currently, we use this to reduce the AI delay
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer * &) const{
        if (!PhaseChangeSkill::triggerable(player))
            return QStringList();
        if (player->getPhase() != Player::Start || player->isKongcheng())
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *ganfuren) const{
        int handcard_num = 0;
        foreach(const Card *card, ganfuren->getHandcards()) {
            if (!ganfuren->isJilei(card))
                handcard_num++;
        }
        ganfuren->throwAllHandCards();
        if (handcard_num >= ganfuren->getHp()) {
            RecoverStruct recover;
            recover.who = ganfuren;
            ganfuren->getRoom()->recover(ganfuren, recover);
        }
        return false;
    }
};

void StandardPackage::addShuGenerals()
{
    General *liubei = new General(this, "liubei", "shu"); // SHU 001
    liubei->addCompanion("guanyu");
    liubei->addCompanion("zhangfei");
    liubei->addCompanion("ganfuren");
    liubei->addSkill(new Rende);

    General *guanyu = new General(this, "guanyu", "shu", 5); // SHU 002
    guanyu->addSkill(new Wusheng);

    General *zhangfei = new General(this, "zhangfei", "shu"); // SHU 003
    zhangfei->addSkill(new Paoxiao);
    zhangfei->addSkill(new PaoxiaoArmorNullificaion);
    insertRelatedSkills("paoxiao", "#paoxiao-null");

    General *zhugeliang = new General(this, "zhugeliang", "shu", 3); // SHU 004
    zhugeliang->addCompanion("huangyueying");
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);

    General *zhaoyun = new General(this, "zhaoyun", "shu"); // SHU 005
    zhaoyun->addCompanion("liushan");
    zhaoyun->addSkill(new Longdan);

    General *machao = new General(this, "machao", "shu"); // SHU 006
    machao->addSkill(new Tieqi);
    machao->addSkill(new Mashu("machao"));

    General *huangyueying = new General(this, "huangyueying", "shu", 3, false); // SHU 007
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill(new Qicai);

    General *huangzhong = new General(this, "huangzhong", "shu"); // SHU 008
    huangzhong->addCompanion("weiyan");
    huangzhong->addSkill(new Liegong);
    huangzhong->addSkill(new LiegongRange);
    insertRelatedSkills("liegong", "#liegong-for-lord");

    General *weiyan = new General(this, "weiyan", "shu"); // SHU 009
    weiyan->addSkill(new Kuanggu);
    weiyan->addSkill(new KuangguRecord);
    insertRelatedSkills("kuanggu", "#kuanggu-record");

    General *pangtong = new General(this, "pangtong", "shu", 3); // SHU 010
    pangtong->addSkill(new Lianhuan);
    pangtong->addSkill(new Niepan);

    General *wolong = new General(this, "wolong", "shu", 3); // SHU 011
    wolong->addCompanion("huangyueying");
    wolong->addCompanion("pangtong");
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Kanpo);
    wolong->addSkill(new Bazhen);

    General *liushan = new General(this, "liushan", "shu", 3); // SHU 013
    liushan->addSkill(new Xiangle);
    liushan->addSkill(new Fangquan);
    liushan->addSkill(new FangquanGive);
    insertRelatedSkills("fangquan", "#fangquan-give");

    General *menghuo = new General(this, "menghuo", "shu"); // SHU 014
    menghuo->addCompanion("zhurong");
    menghuo->addSkill(new SavageAssaultAvoid("huoshou"));
    menghuo->addSkill(new Huoshou);
    menghuo->addSkill(new Zaiqi);
    insertRelatedSkills("huoshou", "#sa_avoid_huoshou");

    General *zhurong = new General(this, "zhurong", "shu", 4, false); // SHU 015
    zhurong->addSkill(new SavageAssaultAvoid("juxiang"));
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);
    insertRelatedSkills("juxiang", "#sa_avoid_juxiang");

    General *ganfuren = new General(this, "ganfuren", "shu", 3, false); // SHU 016
    ganfuren->addSkill(new Shushen);
    ganfuren->addSkill(new Shenzhi);

    addMetaObject<RendeCard>();
    addMetaObject<FangquanCard>();
}
