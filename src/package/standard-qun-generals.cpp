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

#include "standard-qun-generals.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "standard-shu-generals.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

class Jijiu : public OneCardViewAsSkill {
public:
    Jijiu() : OneCardViewAsSkill("jijiu") {
        filter_pattern = ".|red";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach") && !player->hasFlag("Global_PreventPeach") && player->getPhase() == Player::NotActive;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Peach *peach = new Peach(originalCard->getSuit(), originalCard->getNumber());
        peach->addSubcard(originalCard->getId());
        peach->setSkillName(objectName());
        peach->setShowSkill(objectName());
        return peach;
    }
};

QingnangCard::QingnangCard() {
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.isEmpty() && to_select->isWounded();
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

class Qingnang : public OneCardViewAsSkill {
public:
    Qingnang() : OneCardViewAsSkill("qingnang") {
        filter_pattern = ".|.|.|hand!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "h") && !player->hasUsed("QingnangCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(originalCard->getId());
        qingnang_card->setShowSkill(objectName());
        return qingnang_card;
    }
};

class Wushuang : public TriggerSkill {
public:
    Wushuang() : TriggerSkill("wushuang") {
        events << TargetChosen << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (player == NULL) return QStringList();
        if (triggerEvent == TargetChosen) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash") && TriggerSkill::triggerable(use.from) && use.from == player)
                return QStringList(objectName());
            if (use.card->isKindOf("Duel")) {
                if (TriggerSkill::triggerable(use.from) && use.from == player)
                    return QStringList(objectName());
                if (TriggerSkill::triggerable(player) && use.to.contains(player))
                    return QStringList(objectName());
            }
        }
        else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Duel")) {
                foreach(ServerPlayer *lvbu, room->getAllPlayers())
                    if (lvbu->getMark("WushuangTarget") > 0) room->setPlayerMark(lvbu, "WushuangTarget", 0);
            }
            return QStringList();
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName(), data);
        if (invoke){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == TargetChosen) {
            CardUseStruct use = data.value<CardUseStruct>();
            bool can_invoke = false;
            if (use.card->isKindOf("Slash") && TriggerSkill::triggerable(use.from) && use.from == player) {
                can_invoke = true;
                QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
                for (int i = 0; i < use.to.length(); i++) {
                    if (jink_list.at(i).toInt() == 1)
                        jink_list.replace(i, QVariant(2));
                }
                player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
            }
            if (use.card->isKindOf("Duel")) {
                if (TriggerSkill::triggerable(use.from) && use.from == player)
                    can_invoke = true;
                if (TriggerSkill::triggerable(player) && use.to.contains(player))
                    can_invoke = true;
            }
            if (!can_invoke) return false;

            LogMessage log;
            log.from = player;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            if (use.card->isKindOf("Duel"))
                room->setPlayerMark(player, "WushuangTarget", 1);
        }

        return false;
    }
};

LijianCard::LijianCard() {
    mute = true;
}

bool LijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!to_select->isMale())
        return false;

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->deleteLater();
    if (targets.isEmpty() && Self->isProhibited(to_select, duel))
        return false;

    if (targets.length() == 1 && to_select->isCardLimited(duel, Card::MethodUse))
        return false;

    return targets.length() < 2 && to_select != Self;
}

bool LijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void LijianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *diaochan = card_use.from;

    LogMessage log;
    log.from = diaochan;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, diaochan, data);
    room->broadcastSkillInvoke("lijian", diaochan);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, diaochan->objectName(), QString(), "lijian", QString());
    room->moveCardTo(this, diaochan, NULL, Player::PlaceTable, reason, true);

    if (diaochan->ownSkill("lijian") && !diaochan->hasShownSkill("lijian"))
        diaochan->showGeneral(diaochan->inHeadSkills("lijian"));

    QList<int> table_ids = room->getCardIdsOnTable(this);
    if (!table_ids.isEmpty()) {
        DummyCard dummy(table_ids);
        room->moveCardTo(&dummy, diaochan, NULL, Player::DiscardPile, reason, true);
    }

    thread->trigger(CardUsed, room, diaochan, data);
    thread->trigger(CardFinished, room, diaochan, data);
}

void LijianCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName(QString("_%1").arg(getSkillName()));
    if (!from->isCardLimited(duel, Card::MethodUse) && !from->isProhibited(to, duel))
        room->useCard(CardUseStruct(duel, from, to));
    else
        delete duel;
}

class Lijian : public OneCardViewAsSkill {
public:
    Lijian() : OneCardViewAsSkill("lijian") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getAliveSiblings().length() > 1
            && player->canDiscard(player, "he") && !player->hasUsed("LijianCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(originalCard->getId());
        lijian_card->setShowSkill(objectName());
        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        return card->isKindOf("Duel") ? 0 : -1;
    }
};

class Biyue : public PhaseChangeSkill {
public:
    Biyue() : PhaseChangeSkill("biyue") {
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (!PhaseChangeSkill::triggerable(player)) return QStringList();
        if (player->getPhase() == Player::Finish) return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        diaochan->drawCards(1);

        return false;
    }
};

class Luanji : public ViewAsSkill {
public:
    Luanji() : ViewAsSkill("luanji") {
        response_or_use = true;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.isEmpty())
            return !to_select->isEquipped();
        else if (selected.length() == 1) {
            const Card *card = selected.first();
            return !to_select->isEquipped() && to_select->getSuit() == card->getSuit();
        }
        else
            return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 2) {
            ArcheryAttack *aa = new ArcheryAttack(Card::SuitToBeDecided, 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            aa->setShowSkill(objectName());
            return aa;
        }
        else
            return NULL;
    }
};

class ShuangxiongViewAsSkill : public OneCardViewAsSkill {
public:
    ShuangxiongViewAsSkill() :OneCardViewAsSkill("shuangxiong") { //Client::updateProperty() / RoomScene::detachSkill()
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("shuangxiong") != 0 && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card *card) const{
        if (card->isEquipped())
            return false;

        int value = Self->getMark("shuangxiong");
        if (value == 1)
            return card->isBlack();
        else if (value == 2)
            return card->isRed();

        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Duel *duel = new Duel(originalCard->getSuit(), originalCard->getNumber());
        duel->addSubcard(originalCard);
        //duel->setShowSkill("shuangxiong"); // use ShuangxiongViewAsSkill don't cause showing general
        duel->setSkillName("_shuangxiong");
        return duel;
    }
};

class Shuangxiong : public TriggerSkill {
public:
    Shuangxiong() : TriggerSkill("shuangxiong") {
        events << EventPhaseStart << EventPhaseChanging;
        view_as_skill = new ShuangxiongViewAsSkill;
    }

    virtual bool canPreshow() const{
        return true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player != NULL){
            if (triggerEvent == EventPhaseStart) {
                if (player->getPhase() == Player::Start) {
                    room->setPlayerMark(player, "shuangxiong", 0);
                    return QStringList();
                }
                else if (player->getPhase() == Player::Draw && TriggerSkill::triggerable(player))
                    return QStringList(objectName());
            }
            else if (triggerEvent == EventPhaseChanging) {
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.to == Player::NotActive && player->hasFlag("shuangxiong"))
                    room->setPlayerFlag(player, "-shuangxiong");
                return QStringList();
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *shuangxiong, QVariant &, ServerPlayer *) const{
        if (shuangxiong->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName(), 1, shuangxiong);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *shuangxiong, QVariant &, ServerPlayer *) const{
        if (shuangxiong->getPhase() == Player::Draw && TriggerSkill::triggerable(shuangxiong)) {
            room->setPlayerFlag(shuangxiong, "shuangxiong");

            JudgeStruct judge;
            judge.good = true;
            judge.play_animation = false;
            judge.reason = objectName();
            judge.who = shuangxiong;

            room->judge(judge);
            room->setPlayerMark(shuangxiong, "shuangxiong", judge.pattern == "red" ? 1 : 2);

            return true;
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 2;
    }
};

class ShuangxiongGet : public TriggerSkill {
public:
    ShuangxiongGet() : TriggerSkill("#shuangxiong") {
        events << FinishJudge;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player != NULL){
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == "shuangxiong"){
                judge->pattern = judge->card->isRed() ? "red" : "black";
                if (room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        judge->who->obtainCard(judge->card);

        return false;
    }
};

class Wansha : public TriggerSkill { // Gamerule::effect (AskForPeaches)
public:
    Wansha() : TriggerSkill("wansha") {
        events << Dying;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (TriggerSkill::triggerable(player)){
            if (room->getCurrent() == player && player->isAlive() && player->getPhase() != Player::NotActive){
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->hasShownSkill(this) || player->askForSkillInvoke(objectName(), data)){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *jiaxu, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            room->notifySkillInvoked(jiaxu, objectName());

            LogMessage log;
            log.from = jiaxu;
            log.arg = objectName();
            if (jiaxu != dying.who) {
                log.type = "#WanshaTwo";
                log.to << dying.who;
            }
            else {
                log.type = "#WanshaOne";
            }
            room->sendLog(log);
        }
        return false;
    }
};

LuanwuCard::LuanwuCard() {
    target_fixed = true;
    mute = true;
}

void LuanwuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->removePlayerMark(card_use.from, "@chaos");
    room->broadcastSkillInvoke("luanwu", card_use.from);
    room->doSuperLightbox("jiaxu", "luanwu");

    CardUseStruct new_use = card_use;
    new_use.to << room->getOtherPlayers(card_use.from);

    Card::onUse(room, new_use);
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach(ServerPlayer *player, players) {
        int distance = effect.to->distanceTo(player);
        distance_list << distance;
        if (distance != -1)
            nearest = qMin(nearest, distance);
    }

    QList<ServerPlayer *> luanwu_targets;
    for (int i = 0; i < distance_list.length(); i++) {
        if (distance_list[i] == nearest && effect.to->canSlash(players[i], NULL, false))
            luanwu_targets << players[i];
    }

    if (luanwu_targets.isEmpty() || !room->askForUseSlashTo(effect.to, luanwu_targets, "@luanwu-slash")){
        room->loseHp(effect.to);
        room->getThread()->delay(Config.AIDelay);
    }
}

class Luanwu : public ZeroCardViewAsSkill {
public:
    Luanwu() : ZeroCardViewAsSkill("luanwu") {
        frequency = Limited;
        limit_mark = "@chaos";
    }

    virtual const Card *viewAs() const{
        LuanwuCard *card = new LuanwuCard;
        card->setShowSkill(objectName());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@chaos") >= 1;
    }
};

class Weimu : public TriggerSkill {
public:
    Weimu() : TriggerSkill("weimu") {
        events << TargetConfirming;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card || use.card->getTypeId() != Card::TypeTrick || !use.card->isBlack()) return QStringList();
        if (!use.to.contains(player)) return QStringList();
        return QStringList(objectName());
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

class Mengjin : public TriggerSkill {
public:
    Mengjin() :TriggerSkill("mengjin") {
        events << SlashMissed;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.to->isAlive() && player->canDiscard(effect.to, "he")) return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *pangde, QVariant &data, ServerPlayer *) const{
        if (pangde->askForSkillInvoke(objectName(), data)){
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, pangde->objectName(), data.value<SlashEffectStruct>().to->objectName());
            room->broadcastSkillInvoke(objectName(), pangde);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *pangde, QVariant &data, ServerPlayer *) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int to_throw = room->askForCardChosen(pangde, effect.to, "he", objectName(), false, Card::MethodDiscard);
        room->throwCard(Sanguosha->getCard(to_throw), effect.to, pangde);

        return false;
    }
};

class Leiji : public TriggerSkill {
public:
    Leiji() : TriggerSkill("leiji") {
        events << CardResponded;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        const Card *card_star = data.value<CardResponseStruct>().m_card;
        if (card_star->isKindOf("Jink")) return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "leiji-invoke", true, true);
        if (target) {
            player->tag["leiji-target"] = QVariant::fromValue(target);
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        else {
            player->tag.remove("leiji-target");
            return false;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *zhangjiao, QVariant &data, ServerPlayer *) const{
        const Card *card_star = data.value<CardResponseStruct>().m_card;
        if (card_star->isKindOf("Jink")) {
            ServerPlayer *target = zhangjiao->tag["leiji-target"].value<ServerPlayer *>();
            if (target) {

                JudgeStruct judge;
                judge.pattern = ".|spade";
                judge.good = false;
                judge.negative = true;
                judge.reason = objectName();
                judge.who = target;

                room->judge(judge);

                if (judge.isBad())
                    room->damage(DamageStruct(objectName(), zhangjiao, target, 2, DamageStruct::Thunder));
            }
            zhangjiao->tag.remove("leiji-target");
        }
        return false;
    }
};

class Guidao : public TriggerSkill {
public:
    Guidao() : TriggerSkill("guidao") {
        events << AskForRetrial;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *target, QVariant &, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(target))
            return QStringList();

        if (target->isKongcheng()) {
            bool has_black = false;
            for (int i = 0; i < 4; i++) {
                const EquipCard *equip = target->getEquip(i);
                if (equip && equip->isBlack()) {
                    has_black = true;
                    break;
                }
            }
            return (has_black) ? QStringList(objectName()) : QStringList();
        }
        else
            return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
            << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");

        const Card *card = room->askForCard(player, ".|black", prompt, data, Card::MethodResponse, judge->who, true);

        if (card) {
            room->broadcastSkillInvoke(objectName(), player);
            room->retrial(card, player, judge, objectName(), true);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        judge->updateResult();
        return false;
    }
};

class Beige : public TriggerSkill {
public:
    Beige() : TriggerSkill("beige") {
        events << Damaged << FinishJudge;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        if (triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card == NULL || !damage.card->isKindOf("Slash") || damage.to->isDead())
                return skill_list;

            QList<ServerPlayer *> caiwenjis = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *caiwenji, caiwenjis)
                if (caiwenji->canDiscard(caiwenji, "he"))
                    skill_list.insert(caiwenji, QStringList(objectName()));
            return skill_list;
        }
        else {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason != objectName()) return skill_list;
            judge->pattern = QString::number(int(judge->card->getSuit()));
            return skill_list;
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *caiwenji = ask_who;

        if (caiwenji != NULL){
            caiwenji->tag["beige_data"] = data;
            bool invoke = room->askForDiscard(caiwenji, objectName(), 1, 1, true, true, "@beige");
            caiwenji->tag.remove("beige_data");

            if (invoke){
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, caiwenji->objectName(), data.value<DamageStruct>().to->objectName());
                room->notifySkillInvoked(caiwenji, objectName());
                room->broadcastSkillInvoke(objectName(), caiwenji);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *caiwenji = ask_who;
        if (caiwenji == NULL) return false;
        DamageStruct damage = data.value<DamageStruct>();

        JudgeStruct judge;
        judge.good = true;
        judge.play_animation = false;
        judge.who = player;
        judge.reason = objectName();

        room->judge(judge);

        Card::Suit suit = (Card::Suit)(judge.pattern.toInt());
        switch (suit) {
        case Card::Heart: {
            RecoverStruct recover;
            recover.who = caiwenji;
            room->recover(player, recover);

            break;
        }
        case Card::Diamond: {
            player->drawCards(2);
            break;
        }
        case Card::Club: {
            if (damage.from && damage.from->isAlive())
                room->askForDiscard(damage.from, "beige_discard", 2, 2, false, true);

            break;
        }
        case Card::Spade: {
            if (damage.from && damage.from->isAlive())
                damage.from->turnOver();

            break;
        }
        default:
            break;
        }
        return false;
    }
};

class Duanchang : public TriggerSkill {
public:
    Duanchang() : TriggerSkill("duanchang") {
        events << Death;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player == NULL || !player->hasSkill(objectName())) return QStringList();
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return QStringList();

        if (death.damage && death.damage->from){
            ServerPlayer *target = death.damage->from;
            if (!(target->getGeneral()->objectName().contains("sujiang") && target->getGeneral2()->objectName().contains("sujiang")))
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->broadcastSkillInvoke(objectName(), player);
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());

        DeathStruct death = data.value<DeathStruct>();
        ServerPlayer *target = death.damage->from;
        QString choice = "head_general";

        if (player->getAI()) {
            QStringList choices;
            if (!target->getGeneral()->objectName().contains("sujiang"))
                choices << "head_general";

            if (!target->getGeneral2()->objectName().contains("sujiang"))
                choices << "deputy_general";

            choice = room->askForChoice(player, objectName(), choices.join("+"), QVariant::fromValue(target));
        } else {
            QStringList generals;
            if (!target->getGeneral()->objectName().contains("sujiang"))
                generals << target->getGeneral()->objectName();

            if (!target->getGeneral2()->objectName().contains("sujiang"))
                generals << target->getGeneral2()->objectName();

            QString general = generals.first();
            if (generals.length() == 2)
                general = room->askForGeneral(player, generals.join("+"), generals.first(), true, objectName(), QVariant::fromValue(target));

            if (general == target->getGeneral()->objectName())
                choice = "head_general";
            else
                choice = "deputy_general";

        }
        LogMessage log;
        log.type = choice == "head_general" ? "#DuanchangLoseHeadSkills" : "#DuanchangLoseDeputySkills";
        log.from = player;
        log.to << target;
        log.arg = objectName();
        room->sendLog(log);

        if (choice == "head_general")
            room->setPlayerProperty(target, "Duanchang", "head");
        else
            room->setPlayerProperty(target, "Duanchang", "deputy");

        QList<const Skill *> skills = choice == "head_general" ? target->getActualGeneral1()->getVisibleSkillList()
            : target->getActualGeneral2()->getVisibleSkillList();
        foreach(const Skill *skill, skills){
            if (!skill->isAttachedLordSkill())
                room->detachSkillFromPlayer(target, skill->objectName(), !target->hasShownSkill(skill));
        }
        if (death.damage->from->isAlive())
            death.damage->from->gainMark("@duanchang");

        return false;
    }
};

XiongyiCard::XiongyiCard() {
    mute = true;
    target_fixed = true;
}

void XiongyiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->removePlayerMark(card_use.from, "@arise");
    room->broadcastSkillInvoke("xiongyi", card_use.from);
    room->doSuperLightbox("mateng", "xiongyi");
    SkillCard::onUse(room, card_use);
}

void XiongyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *p, room->getAllPlayers()) {
        if (p->isFriendWith(source)) {
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, source->objectName(), p->objectName());
            targets << p;
        }
    }
    room->sortByActionOrder(targets);
    Card::use(room, source, targets);

    QStringList kingdom_list = Sanguosha->getKingdoms();
    kingdom_list << "careerist";
    bool invoke = true;
    int n = source->getPlayerNumWithSameKingdom("xiongyi", QString(), MaxCardsType::Normal);
    foreach(QString kingdom, Sanguosha->getKingdoms()) {
        if (kingdom == "god") continue;
        if (source->getRole() == "careerist") {
            if (kingdom == "careerist")
                continue;
        } else if (source->getKingdom() == kingdom)
            continue;
        int other_num = source->getPlayerNumWithSameKingdom("xiongyi", kingdom, MaxCardsType::Normal);
        if (other_num > 0 && other_num < n) {
            invoke = false;
            break;
        }
    }

    if (invoke && source->isWounded()) {
        RecoverStruct recover;
        recover.who = source;
        room->recover(source, recover);
    }
}

void XiongyiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(3);
}

class Xiongyi : public ZeroCardViewAsSkill {
public:
    Xiongyi() : ZeroCardViewAsSkill("xiongyi") {
        frequency = Limited;
        limit_mark = "@arise";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@arise") >= 1;
    }

    virtual const Card *viewAs() const{
        XiongyiCard *card = new XiongyiCard;
        card->setShowSkill(objectName());
        return card;
    }
};

class Mingshi : public TriggerSkill {
public:
    Mingshi() : TriggerSkill("mingshi") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && !damage.from->hasShownAllGenerals())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName(), data);
        if (invoke){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        room->notifySkillInvoked(player, objectName());

        LogMessage log;
        log.type = "#Mingshi";
        log.from = player;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(--damage.damage);
        room->sendLog(log);

        if (damage.damage < 1)
            return true;
        data = QVariant::fromValue(damage);

        return false;
    }
};

class Lirang : public TriggerSkill {
public:
    Lirang() : TriggerSkill("lirang") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != player)
            return QStringList();
        if ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
            if (move.to_place == Player::PlaceTable) {
                int i = 0;
                QList<int> lirang_card;
                foreach(int card_id, move.card_ids) {
                    if (room->getCardPlace(card_id) == Player::PlaceTable
                        && (move.from_places[i] == Player::PlaceHand || move.from_places[i] == Player::PlaceEquip)) {
                        lirang_card << card_id;
                    }
                    i++;
                }
                if (!lirang_card.isEmpty())
                    player->tag["lirang_to_judge"] = IntList2VariantList(lirang_card);
            } else if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile) {
                QList<int> lirang_card = VariantList2IntList(player->tag["lirang_to_judge"].toList());
                player->tag.remove("lirang_to_judge");
                QList<int> lirangs;
                foreach (int id, lirang_card) {
                    if (room->getCardPlace(id) == Player::DiscardPile)
                        lirangs << id;
                }
                if (lirangs.isEmpty())
                    return QStringList();
                player->tag["lirang"] = QVariant::fromValue(IntList2VariantList(lirangs));
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (!(player->hasShownSkill(this) || room->askForSkillInvoke(player, objectName()))){
            player->tag.remove("lirang");
            return false;
        }
        if (!player->hasShownSkill(this))
            player->setMark("lirang_notcancelable", 1);
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *kongrong, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != kongrong)
            return false;
        if (move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)) {

            QList<int> lirang_card = VariantList2IntList(kongrong->tag["lirang"].toList());
            kongrong->tag.remove("lirang");

            QList<int> lirang_copy = lirang_card;
            foreach(int id, lirang_copy){
                if (room->getCardPlace(id) != Player::DiscardPile)
                    lirang_card.removeOne(id);
            }

            if (lirang_card.isEmpty())
                return false;

            CardMoveReason preview_reason(CardMoveReason::S_REASON_PREVIEW, kongrong->objectName(), objectName(), QString());
            CardsMoveStruct lirang_preview(lirang_card, NULL, kongrong, Player::DiscardPile, Player::PlaceHand, preview_reason);
            QList<CardsMoveStruct> lirang_preview_l;
            lirang_preview_l << lirang_preview;
            QList<ServerPlayer *> _kongrong;
            _kongrong << kongrong;

            room->setPlayerFlag(kongrong, "lirang_InTempMoving"); // a trick to disabled log
            room->notifyMoveCards(true, lirang_preview_l, false, _kongrong);
            room->notifyMoveCards(false, lirang_preview_l, false, _kongrong);
            room->setPlayerFlag(kongrong, "-lirang_InTempMoving");

            QList<int> original_lirang = lirang_card;
            CardMoveReason lirang_reason(CardMoveReason::S_REASON_PREVIEWGIVE, kongrong->objectName());
            bool lirang_cancelable = true;
            if (kongrong->getMark("lirang_notcancelable") > 0){
                kongrong->setMark("lirang_notcancelable", 0);
                lirang_cancelable = false;
            }
            while (room->askForYiji(kongrong, lirang_card, objectName(), true, true, lirang_cancelable, -1,
                QList<ServerPlayer *>(), lirang_reason, "@lirang-distribute", lirang_cancelable)) {
                lirang_cancelable = true;
                CardsMoveStruct lirang_give_preview(QList<int>(), kongrong, NULL, Player::PlaceHand, Player::PlaceTable, preview_reason);
                foreach(int id, original_lirang){
                    if (room->getCardPlace(id) != Player::DiscardPile){
                        lirang_give_preview.card_ids << id;
                        lirang_card.removeOne(id);
                    }
                }

                original_lirang = lirang_card;

                QList<CardsMoveStruct> lirang_give_preview_l;
                lirang_give_preview_l << lirang_give_preview;
                room->notifyMoveCards(true, lirang_give_preview_l, true, _kongrong);
                room->notifyMoveCards(false, lirang_give_preview_l, true, _kongrong);
                if (kongrong->isDead()) break;
            }

            if (!lirang_card.isEmpty()){
                CardsMoveStruct lirang_return_preview(lirang_card, kongrong, NULL, Player::PlaceHand, Player::DiscardPile, preview_reason);
                QList<CardsMoveStruct> lirang_return_preview_l;
                lirang_return_preview_l << lirang_return_preview;
                room->notifyMoveCards(true, lirang_return_preview_l, true, _kongrong);
                room->notifyMoveCards(false, lirang_return_preview_l, false, _kongrong);
            }
        }
        return false;
    }
};

class Shuangren : public PhaseChangeSkill {
public:
    Shuangren() : PhaseChangeSkill("shuangren") {
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *jiling, QVariant &, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(jiling)) return QStringList();
        if (jiling->getPhase() == Player::Play && !jiling->isKongcheng()) {
            Room *room = jiling->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(jiling);
            foreach(ServerPlayer *player, other_players) {
                if (!player->isKongcheng()) {
                    can_invoke = true;
                    break;
                }
            }

            return can_invoke ? QStringList(objectName()) : QStringList();
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *jiling, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(jiling)){
            if (!p->isKongcheng())
                targets << p;
        }
        ServerPlayer *victim;
        if ((victim = room->askForPlayerChosen(jiling, targets, "shuangren", "@shuangren", true, true)) != NULL){
            room->broadcastSkillInvoke(objectName(), 1, jiling);
            PindianStruct *pd = jiling->pindianSelect(victim, "shuangren");
            jiling->tag["shuangren_pd"] = QVariant::fromValue(pd);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *jiling) const{
        PindianStruct *pd = jiling->tag["shuangren_pd"].value<PindianStruct *>();
        jiling->tag.remove("shuangren_pd");
        if (pd != NULL){
            ServerPlayer *target = pd->to;
            bool success = jiling->pindian(pd);
            pd = NULL;
            Room *room = jiling->getRoom();
            if (success) {
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *p, room->getAlivePlayers()){
                    if (jiling->canSlash(p, NULL, false) && (p->isFriendWith(target) || target == p))
                        targets << p;
                }
                if (targets.isEmpty())
                    return false;

                ServerPlayer *slasher = room->askForPlayerChosen(jiling, targets, "shuangren-slash", "@dummy-slash");
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName("_shuangren");
                room->useCard(CardUseStruct(slash, jiling, slasher), false);
            }
            else {
                room->broadcastSkillInvoke("shuangren", 3, jiling);
                return true;
            }
        }
        else
            Q_ASSERT(false);
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 2;
    }
};

class Sijian : public TriggerSkill {
public:
    Sijian() : TriggerSkill("sijian") {
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *tianfeng, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(tianfeng)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == tianfeng && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard) {
            QList<ServerPlayer *> other_players = room->getOtherPlayers(tianfeng);
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *p, other_players) {
                if (tianfeng->canDiscard(p, "he"))
                    return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *tianfeng, QVariant &, ServerPlayer *) const{
        QList<ServerPlayer *> other_players = room->getOtherPlayers(tianfeng);
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, other_players) {
            if (tianfeng->canDiscard(p, "he"))
                targets << p;
        }
        ServerPlayer *to = room->askForPlayerChosen(tianfeng, targets, objectName(), "sijian-invoke", true, true);
        if (to) {
            tianfeng->tag["sijian_target"] = QVariant::fromValue(to);
            room->broadcastSkillInvoke(objectName(), tianfeng);
            return true;
        }
        else tianfeng->tag.remove("sijian_target");
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *tianfeng, QVariant &, ServerPlayer *) const{
        ServerPlayer *to = tianfeng->tag["sijian_target"].value<ServerPlayer *>();
        tianfeng->tag.remove("sijian_target");
        if (to && tianfeng->canDiscard(to, "he")) {
            int card_id = room->askForCardChosen(tianfeng, to, "he", objectName(), false, Card::MethodDiscard);
            room->throwCard(card_id, to, tianfeng);
        }
        return false;
    }
};

class Suishi : public TriggerSkill {
public:
    Suishi() : TriggerSkill("suishi") {
        events << Dying << Death;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        ServerPlayer *target = NULL;
        if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.damage && dying.damage->from)
                target = dying.damage->from;
            if (dying.who != player && target && (target->isFriendWith(player) || player->willBeFriendWith(target)))
                return QStringList(objectName());
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            target = death.who;
            if (target && (target->isFriendWith(player) || player->willBeFriendWith(target)))
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName(), (int)triggerEvent);
        if (invoke){
            if (triggerEvent == Dying)
                room->broadcastSkillInvoke(objectName(), 1, player);
            else if (triggerEvent == Death)
                room->broadcastSkillInvoke(objectName(), 2, player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (triggerEvent == Dying)
            player->drawCards(1);
        else if (triggerEvent == Death)
            room->loseHp(player);
        return false;
    }
};

class Kuangfu : public TriggerSkill {
public:
    Kuangfu() : TriggerSkill("kuangfu") {
        events << Damage;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *panfeng, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(panfeng)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if (damage.card && damage.card->isKindOf("Slash") && target->hasEquip() && !damage.chain && !damage.transfer && !damage.to->hasFlag("Global_DFDebut")) {
            QStringList equiplist;
            for (int i = 0; i < 5; i++) {
                if (!target->getEquip(i)) continue;
                if (panfeng->canDiscard(target, target->getEquip(i)->getEffectiveId()) || panfeng->getEquip(i) == NULL)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *panfeng, QVariant &data, ServerPlayer *) const{
        if (panfeng->askForSkillInvoke(objectName(), data)){
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, panfeng->objectName(), data.value<DamageStruct>().to->objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *panfeng, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;

        QStringList equiplist;
        for (int i = 0; i < 5; i++) {
            if (!target->getEquip(i)) continue;
            if (panfeng->canDiscard(target, target->getEquip(i)->getEffectiveId()) || panfeng->getEquip(i) == NULL)
                equiplist << QString::number(i);
        }

        int equip_index = room->askForChoice(panfeng, "kuangfu_equip", equiplist.join("+"), QVariant::fromValue((ServerPlayer *)target)).toInt();
        const Card *card = target->getEquip(equip_index);
        int card_id = card->getEffectiveId();

        QStringList choicelist;
        if (panfeng->canDiscard(target, card_id))
            choicelist << "throw";
        if (equip_index > -1 && panfeng->getEquip(equip_index) == NULL)
            choicelist << "move";

        QString choice = room->askForChoice(panfeng, "kuangfu", choicelist.join("+"));

        if (choice == "move") {
            room->broadcastSkillInvoke(objectName(), 2, panfeng);
            room->moveCardTo(card, panfeng, Player::PlaceEquip);
        } else {
            room->broadcastSkillInvoke(objectName(), 1, panfeng);
            room->throwCard(card, target, panfeng);
        }

        return false;
    }
};

HuoshuiCard::HuoshuiCard() {
    target_fixed = true;
}

class HuoshuiVS : public ZeroCardViewAsSkill {
public:
    HuoshuiVS() : ZeroCardViewAsSkill("huoshui") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasShownSkill(objectName());
    }

    virtual const Card *viewAs() const{
        HuoshuiCard *card = new HuoshuiCard;
        card->setShowSkill(objectName());
        return card;
    }
};

class Huoshui : public TriggerSkill {
public:
    Huoshui() : TriggerSkill("huoshui") {
        events << GeneralShown << GeneralHidden << GeneralRemoved << EventPhaseStart << Death << EventAcquireSkill << EventLoseSkill;
        view_as_skill = new HuoshuiVS;
    }

    void doHuoshui(Room *room, ServerPlayer *zoushi, bool set) const{
        if (set && zoushi->tag["huoshui"].toBool() == false){
            foreach(ServerPlayer *p, room->getOtherPlayers(zoushi)){
                room->setPlayerDisableShow(p, "hd", "huoshui");
            }
            zoushi->tag["huoshui"] = true;
        }
        else if (!set && zoushi->tag["huoshui"].toBool() == true) {
            foreach(ServerPlayer *p, room->getOtherPlayers(zoushi)){
                room->removePlayerDisableShow(p, "huoshui");
            }
            zoushi->tag["huoshui"] = false;
        }
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> r;
        if (player == NULL)
            return r;
        if (triggerEvent != Death && !player->isAlive())
            return r;
        ServerPlayer *c = room->getCurrent();
        if (c == NULL || (triggerEvent != EventPhaseStart && c->getPhase() == Player::NotActive) || c != player)
            return r;

        if ((triggerEvent == GeneralShown || triggerEvent == EventPhaseStart || triggerEvent == EventAcquireSkill) && !player->hasShownSkill(this))
            return r;
        if ((triggerEvent == GeneralShown || triggerEvent == GeneralHidden) && (!player->ownSkill(this) || player->inHeadSkills(this) != data.toBool()))
            return r;
        if (triggerEvent == GeneralRemoved && data.toString() != "zoushi")
            return r;
        if (triggerEvent == EventPhaseStart && !(player->getPhase() == Player::RoundStart || player->getPhase() == Player::NotActive))
            return r;
        if (triggerEvent == Death && (data.value<DeathStruct>().who != player || !player->hasShownSkill(this)))
            return r;
        if ((triggerEvent == EventAcquireSkill || triggerEvent == EventLoseSkill) && data.toString() != objectName())
            return r;

        bool set = false;
        if (triggerEvent == GeneralShown || triggerEvent == EventAcquireSkill || (triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart))
            set = true;

        doHuoshui(room, player, set);

        return r;
    }
};

QingchengCard::QingchengCard() {
    handling_method = Card::MethodDiscard;
}

bool QingchengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if ((to_select->isLord() || to_select->getGeneralName().contains("sujiang")) && to_select->getGeneral2() != NULL && to_select->getGeneral2Name().contains("sujiang")) return false;
    return targets.isEmpty() && to_select != Self && to_select->hasShownAllGenerals();
}

void QingchengCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *player = effect.from, *to = effect.to;
    Room *room = player->getRoom();

    QStringList choices;
    if (!to->isLord() && !to->getGeneralName().contains("sujiang"))
        choices << to->getGeneral()->objectName();
    if (to->getGeneral2() != NULL && !to->getGeneral2Name().contains("sujiang"))
        choices << to->getGeneral2()->objectName();

    if (choices.length() == 0)
        return;
    QString choice = choices.first();
    if (choices.length() == 2)
        choice = room->askForGeneral(player, choices, QString(), true, "qingcheng");

    to->hideGeneral(choice == to->getGeneral()->objectName());
}

class Qingcheng : public OneCardViewAsSkill {
public:
    Qingcheng() : OneCardViewAsSkill("qingcheng") {
        filter_pattern = "EquipCard!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        QingchengCard *first = new QingchengCard;
        first->addSubcard(originalcard->getId());
        first->setShowSkill(objectName());
        return first;
    }
};

void StandardPackage::addQunGenerals()
{
    General *huatuo = new General(this, "huatuo", "qun", 3); // QUN 001
    huatuo->addSkill(new Jijiu);
    huatuo->addSkill(new Qingnang);

    General *lvbu = new General(this, "lvbu", "qun", 5); // QUN 002
    lvbu->addCompanion("diaochan");
    lvbu->addSkill(new Wushuang);

    General *diaochan = new General(this, "diaochan", "qun", 3, false); // QUN 003
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);

    General *yuanshao = new General(this, "yuanshao", "qun"); // QUN 004
    yuanshao->addCompanion("yanliangwenchou");
    yuanshao->addSkill(new Luanji);

    General *yanliangwenchou = new General(this, "yanliangwenchou", "qun"); // QUN 005
    yanliangwenchou->addSkill(new Shuangxiong);
    yanliangwenchou->addSkill(new ShuangxiongGet);
    insertRelatedSkills("shuangxiong", "#shuangxiong");

    General *jiaxu = new General(this, "jiaxu", "qun", 3); // QUN 007
    jiaxu->addSkill(new Wansha);
    jiaxu->addSkill(new Luanwu);
    jiaxu->addSkill(new Weimu);

    General *pangde = new General(this, "pangde", "qun"); // QUN 008
    pangde->addSkill(new Mashu("pangde"));
    pangde->addSkill(new Mengjin);

    General *zhangjiao = new General(this, "zhangjiao", "qun", 3); // QUN 010
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Guidao);

    General *caiwenji = new General(this, "caiwenji", "qun", 3, false); // QUN 012
    caiwenji->addSkill(new Beige);
    caiwenji->addSkill(new Duanchang);

    General *mateng = new General(this, "mateng", "qun"); // QUN 013
    mateng->addSkill(new Mashu("mateng"));
    mateng->addSkill(new Xiongyi);

    General *kongrong = new General(this, "kongrong", "qun", 3); // QUN 014
    kongrong->addSkill(new Mingshi);
    kongrong->addSkill(new Lirang);

    General *jiling = new General(this, "jiling", "qun"); // QUN 015
    jiling->addSkill(new Shuangren);
    jiling->addSkill(new SlashNoDistanceLimitSkill("shuangren"));
    insertRelatedSkills("shuangren", "#shuangren-slash-ndl");

    General *tianfeng = new General(this, "tianfeng", "qun", 3); // QUN 016
    tianfeng->addSkill(new Sijian);
    tianfeng->addSkill(new Suishi);

    General *panfeng = new General(this, "panfeng", "qun"); // QUN 017
    panfeng->addSkill(new Kuangfu);

    General *zoushi = new General(this, "zoushi", "qun", 3, false); // QUN 018
    zoushi->addSkill(new Huoshui);
    zoushi->addSkill(new Qingcheng);

    addMetaObject<QingnangCard>();
    addMetaObject<LijianCard>();
    addMetaObject<LuanwuCard>();
    addMetaObject<XiongyiCard>();
    addMetaObject<HuoshuiCard>();
    addMetaObject<QingchengCard>();
}
