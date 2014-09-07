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

#include "standard-wu-generals.h"
#include "engine.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "client.h"
#include "settings.h"
#include "json.h"

ZhihengCard::ZhihengCard() {
    target_fixed = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    if (source->isAlive())
        room->drawCards(source, subcards.length());
}

class Zhiheng : public ViewAsSkill {
public:
    Zhiheng() : ViewAsSkill("zhiheng") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !Self->isJilei(to_select) && selected.length() < Self->getMaxHp();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);
        zhiheng_card->setSkillName(objectName());
        zhiheng_card->setShowSkill(objectName());
        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("ZhihengCard");
    }
};

class Qixi : public OneCardViewAsSkill {
public:
    Qixi() : OneCardViewAsSkill("qixi") {
        response_or_use = true;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        dismantlement->setShowSkill(objectName());
        return dismantlement;
    }
};

class KejiRecord : public TriggerSkill{
public:
    KejiRecord() : TriggerSkill("#keji-record"){
        events << PreCardUsed << CardResponded;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (player != NULL && player->isAlive()){
            const Card *card = NULL;
            if (triggerEvent == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else
                card = data.value<CardResponseStruct>().m_card;
            if (card->isKindOf("Slash") && player->getPhase() == Player::Play)
                player->setFlags("KejiSlashInPlayPhase");
        }
        return QStringList();
    }
};

class Keji : public TriggerSkill {
public:
    Keji() : TriggerSkill("keji") {
        events << EventPhaseChanging;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (TriggerSkill::triggerable(player)){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (!player->hasFlag("KejiSlashInPlayPhase") && change.to == Player::Discard)
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())){
            if (player->getHandcardNum() > player->getMaxCards()) {
                room->broadcastSkillInvoke(objectName(), player);
            }
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *lvmeng, QVariant &, ServerPlayer *) const{
        lvmeng->skip(Player::Discard);
        return false;
    }
};

KurouCard::KurouCard() {
    target_fixed = true;
}

void KurouCard::extraCost(Room *room, const CardUseStruct &card_use) const{
    room->loseHp(card_use.from);
}

void KurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->drawCards(source, 2);
}

class Kurou : public ZeroCardViewAsSkill {
public:
    Kurou() : ZeroCardViewAsSkill("kurou") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHp() > 0;
    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

Yingzi::Yingzi() : DrawCardsSkill("yingzi") {
    frequency = Frequent;
}

bool Yingzi::cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
    if (player->askForSkillInvoke(objectName())){
        room->broadcastSkillInvoke(objectName(), qrand() % 2 + 1, player);
        return true;
    }
    return false;
}

int Yingzi::getDrawNum(ServerPlayer *, int n) const{
    return n + 1;
}

FanjianCard::FanjianCard() {
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "fanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);
    room->showCard(target, card_id);

    if (card->getSuit() != suit)
        room->damage(DamageStruct("fanjian", zhouyu, target));
}

class Fanjian : public ZeroCardViewAsSkill {
public:
    Fanjian() : ZeroCardViewAsSkill("fanjian") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        FanjianCard *fj = new FanjianCard;
        fj->setShowSkill(objectName());
        return fj;
    }
};

class Guose : public OneCardViewAsSkill {
public:
    Guose() : OneCardViewAsSkill("guose") {
        filter_pattern = ".|diamond";
        response_or_use = true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Indulgence *indulgence = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        indulgence->addSubcard(originalCard->getId());
        indulgence->setSkillName(objectName());
        indulgence->setShowSkill(objectName());
        return indulgence;
    }
};

LiuliCard::LiuliCard() {
    //mute = true;
}

bool LiuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->hasFlag("LiuliSlashSource") || to_select == Self)
        return false;

    const Player *from = NULL;
    foreach(const Player *p, Self->getAliveSiblings()) {
        if (p->hasFlag("LiuliSlashSource")) {
            from = p;
            break;
        }
    }

    const Card *slash = Card::Parse(Self->property("liuli").toString());
    if (from && !from->canSlash(to_select, slash, false))
        return false;

    int card_id = subcards.first();
    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getId() == card_id) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - Self->getAttackRange(false);
    }
    else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getId() == card_id) {
        range_fix += 1;
    }
    int distance = Self->distanceTo(to_select, range_fix);
    if (distance == -1)
        return false;
    return distance <= Self->getAttackRange();
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("LiuliTarget");
}

class LiuliViewAsSkill : public OneCardViewAsSkill {
public:
    LiuliViewAsSkill() : OneCardViewAsSkill("liuli") {
        filter_pattern = ".!";
        response_pattern = "@@liuli";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->addSubcard(originalCard);
        liuli_card->setShowSkill(objectName());
        return liuli_card;
    }
};

class Liuli : public TriggerSkill {
public:
    Liuli() : TriggerSkill("liuli") {
        events << TargetConfirming;
        view_as_skill = new LiuliViewAsSkill;
    }

    virtual bool canPreshow() const{
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data, ServerPlayer * &) const {
        if (!TriggerSkill::triggerable(daqiao)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isKindOf("Slash") && use.to.contains(daqiao) && daqiao->canDiscard(daqiao, "he")) {
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(use.from);

            bool can_invoke = false;
            foreach(ServerPlayer *p, players) {
                if (use.from->canSlash(p, use.card, false) && daqiao->inMyAttackRange(p)) {
                    can_invoke = true;
                    break;
                }
            }

            return can_invoke ? QStringList(objectName()) : QStringList();
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QString prompt = "@liuli:" + use.from->objectName();
        room->setPlayerFlag(use.from, "LiuliSlashSource");
        // a temp nasty trick
        daqiao->tag["liuli-card"] = QVariant::fromValue((const Card *)use.card); // for the server (AI)
        room->setPlayerProperty(daqiao, "liuli", use.card->toString()); // for the client (UI)
        const Card *c = room->askForUseCard(daqiao, "@@liuli", prompt, -1, Card::MethodDiscard);
        daqiao->tag.remove("liuli-card");
        room->setPlayerProperty(daqiao, "liuli", QString());
        room->setPlayerFlag(use.from, "-LiuliSlashSource");
        if (c != NULL)
            return true;

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
        foreach(ServerPlayer *p, players) {
            if (p->hasFlag("LiuliTarget")) {
                p->setFlags("-LiuliTarget");
                use.to.removeOne(daqiao);
                use.to.append(p);
                room->sortByActionOrder(use.to);
                data = QVariant::fromValue(use);
                room->getThread()->trigger(TargetConfirming, room, p, data);
                return false;
            }
        }

        return false;
    }
};

class Qianxun : public TriggerSkill {
public:
    Qianxun() : TriggerSkill("qianxun") {
        events << TargetConfirming;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card || use.card->getTypeId() != Card::TypeTrick || (!use.card->isKindOf("Snatch") && !use.card->isKindOf("Indulgence")))
            return QStringList();
        if (!use.to.contains(player)) return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
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
        log.to = use.to;
        log.arg = use.card->objectName();
        room->sendLog(log);

        room->setEmotion(player, "cancel");

        use.to.removeOne(player);
        data = QVariant::fromValue(use);
        return false;
    }
};

class Duoshi : public OneCardViewAsSkill {
public:
    Duoshi() : OneCardViewAsSkill("duoshi") {
        filter_pattern = ".|red|.|hand";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("DuoshiAE") < 4;
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        AwaitExhausted *await = new AwaitExhausted(originalcard->getSuit(), originalcard->getNumber());
        await->addSubcard(originalcard->getId());
        await->setSkillName("duoshi");
        await->setShowSkill(objectName());
        return await;
    }
};

JieyinCard::JieyinCard() {
}

bool JieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    return to_select->isMale() && to_select->isWounded() && to_select != Self;
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

class Jieyin : public ViewAsSkill {
public:
    Jieyin() : ViewAsSkill("jieyin") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= 2 && !player->hasUsed("JieyinCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() > 1 || Self->isJilei(to_select))
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        JieyinCard *jieyin_card = new JieyinCard();
        jieyin_card->addSubcards(cards);
        jieyin_card->setShowSkill(objectName());
        return jieyin_card;
    }
};

class Xiaoji : public TriggerSkill {
public:
    Xiaoji() : TriggerSkill("xiaoji") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(sunshangxiang)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == sunshangxiang && move.from_places.contains(Player::PlaceEquip)) {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const{
        if (room->askForSkillInvoke(sunshangxiang, objectName())){
            room->broadcastSkillInvoke(objectName(), sunshangxiang);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const{
        sunshangxiang->drawCards(2);

        return false;
    }
};

Yinghun::Yinghun() : PhaseChangeSkill("yinghun") {
}

bool Yinghun::canPreshow() const {
    return false;
}

QStringList Yinghun::triggerable(TriggerEvent, Room *, ServerPlayer *target, QVariant &, ServerPlayer * &) const{
    return (PhaseChangeSkill::triggerable(target)
        && target->getPhase() == Player::Start
        && target->isWounded()) ? QStringList(objectName()) : QStringList();
}

bool Yinghun::cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
    ServerPlayer *to = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "yinghun-invoke", true, true);
    if (to) {
        player->tag["yinghun_target"] = QVariant::fromValue(to);
        return true;
    }
    return false;
}

bool Yinghun::onPhaseChange(ServerPlayer *sunjian) const{
    Room *room = sunjian->getRoom();
    ServerPlayer *to = sunjian->tag["yinghun_target"].value<ServerPlayer *>();
    if (to) {
        int x = sunjian->getLostHp();

        if (x == 1) {
            room->broadcastSkillInvoke(objectName(), 1, sunjian);

            to->drawCards(1);
            room->askForDiscard(to, objectName(), 1, 1, false, true);
        } else {
            to->setFlags("YinghunTarget");
            QString choice = room->askForChoice(sunjian, objectName(), "d1tx+dxt1");
            to->setFlags("-YinghunTarget");
            if (choice == "d1tx") {
                room->broadcastSkillInvoke(objectName(), 2, sunjian);

                to->drawCards(1);
                room->askForDiscard(to, objectName(), x, x, false, true);
            } else {
                room->broadcastSkillInvoke(objectName(), 1, sunjian);

                to->drawCards(x);
                room->askForDiscard(to, objectName(), 1, 1, false, true);
            }
        }
    }
    return false;
}

TianxiangCard::TianxiangCard() {
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("tianxiang_target");
    effect.from->setFlags("tianxiang_invoke");
}

class TianxiangViewAsSkill : public OneCardViewAsSkill {
public:
    TianxiangViewAsSkill() : OneCardViewAsSkill("tianxiang") {
        response_pattern = "@@tianxiang";
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        TianxiangCard *tianxiangCard = new TianxiangCard;
        tianxiangCard->addSubcard(originalCard);
        tianxiangCard->setShowSkill(objectName());
        return tianxiangCard;
    }

    virtual bool viewFilter(const Card *to_select) const {
        if (Self->isJilei(to_select)) return false;
        QString pat;
        if (Self->hasSkill("hongyan") && !Self->hasShownSkill("hongyan"))
            pat = ".|heart,spade|.|hand";
        else
            pat = ".|heart|.|hand";
        ExpPattern pattern(pat);
        return pattern.match(Self, to_select);
    }
};

class Tianxiang : public TriggerSkill {
public:
    Tianxiang() : TriggerSkill("tianxiang") {
        events << DamageInflicted;
        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual bool canPreshow() const {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *xiaoqiao, QVariant &, ServerPlayer* &) const{
        if (TriggerSkill::triggerable(xiaoqiao))
            return (xiaoqiao->canDiscard(xiaoqiao, "h")) ? QStringList(objectName()) : QStringList();
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *xiaoqiao, QVariant &data, ServerPlayer *) const{
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            p->setFlags("-tianxiang_target");
        }
        xiaoqiao->setFlags("-tianxiang_invoke");
        xiaoqiao->tag["TianxiangDamage"] = data;
        room->askForUseCard(xiaoqiao, "@@tianxiang", "@tianxiang-card", -1, Card::MethodDiscard);
        xiaoqiao->tag.remove("TianxiangDamage");
        if (xiaoqiao->hasFlag("tianxiang_invoke"))
            return true;

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *xiaoqiao, QVariant &data, ServerPlayer *) const {
        ServerPlayer *target = NULL;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if (p->hasFlag("tianxiang_target")){
                target = p;
                break;
            }
        }
        xiaoqiao->setFlags("-tianxiang_invoke");
        target->setFlags("-tianxiang_target");

        DamageStruct damage = data.value<DamageStruct>();
        damage.transfer = true;
        damage.to = target;
        damage.transfer_reason = "tianxiang";

        xiaoqiao->tag["TransferDamage"] = QVariant::fromValue(damage);

        return true;
    }
};

class TianxiangDraw : public TriggerSkill {
public:
    TianxiangDraw() : TriggerSkill("#tianxiang") {
        events << DamageComplete;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (player == NULL) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (player->isAlive() && damage.transfer && damage.transfer_reason == "tianxiang") {
            player->drawCards(player->getLostHp());
            player->removeMark("TianxiangTarget");
        }
        return QStringList();
    }
};

class HongyanFilter : public FilterSkill {
public:
    HongyanFilter() : FilterSkill("hongyan") {
    }

    static WrappedCard *changeToHeart(int cardId) {
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("hongyan");
        new_card->setSuit(Card::Heart);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card *to_select) const{
        Room *room = Sanguosha->currentRoom();
        foreach(ServerPlayer *p, room->getPlayers()) {
            if (p->ownSkill(objectName()) && p->hasShownSkill(objectName()))
                return to_select->getSuit() == Card::Spade;
        }
        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToHeart(originalCard->getEffectiveId());
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

class Hongyan : public TriggerSkill {
public:
    Hongyan() : TriggerSkill("hongyan") {
        events << FinishRetrial;
        frequency = Compulsory;
        view_as_skill = new HongyanFilter;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        JudgeStruct *judge = data.value<JudgeStruct *>();

        if (TriggerSkill::triggerable(player) && !player->hasShownSkill(this)
            && judge->who == player && judge->card->getSuit() == Card::Spade)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        return player->askForSkillInvoke(objectName(), data);
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *xiaoqiao, QVariant &data, ServerPlayer *) const {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        QList<const Card *> cards;
        cards << judge->card;
        room->filterCards(xiaoqiao, cards, true);
        judge->updateResult();
        return false;
    }
};

TianyiCard::TianyiCard() {
}

bool TianyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TianyiCard::extraCost(Room *, const CardUseStruct &card_use) const{
    ServerPlayer *dalizi = card_use.from;
    PindianStruct *pd = dalizi->pindianSelect(card_use.to.first(), "tianyi");
    dalizi->tag["tianyi_pd"] = QVariant::fromValue(pd);
}

void TianyiCard::onEffect(const CardEffectStruct &effect) const{
    PindianStruct *pd = effect.from->tag["tianyi_pd"].value<PindianStruct *>();
    effect.from->tag.remove("tianyi_pd");
    if (pd != NULL){
        bool success = effect.from->pindian(pd);
        pd = NULL;

        if (success)
            effect.to->getRoom()->setPlayerFlag(effect.from, "TianyiSuccess");
        else
            effect.to->getRoom()->setPlayerCardLimitation(effect.from, "use", "Slash", true);
    }
    else
        Q_ASSERT(false);
}

class Tianyi : public ZeroCardViewAsSkill {
public:
    Tianyi() : ZeroCardViewAsSkill("tianyi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TianyiCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        TianyiCard *card = new TianyiCard;
        card->setShowSkill(objectName());
        return card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const {
        return 2;
    }
};


class TianyiTargetMod : public TargetModSkill {
public:
    TianyiTargetMod() : TargetModSkill("#tianyi-target") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasFlag("TianyiSuccess"))
            return 1;
        else
            return 0;
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasFlag("TianyiSuccess"))
            return 1000;
        else
            return 0;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *) const{
        if (from->hasFlag("TianyiSuccess"))
            return 1;
        else
            return 0;
    }
};

static bool compareByNumber(int c1, int c2){
    const Card *card1 = Sanguosha->getCard(c1);
    const Card *card2 = Sanguosha->getCard(c2);

    return card1->getNumber() < card2->getNumber();
}

class BuquRemove : public TriggerSkill {
public:
    BuquRemove() : TriggerSkill("#buqu-remove") {
        events << HpRecover;
        frequency = Compulsory;
    }

    static void Remove(ServerPlayer *zhoutai) {
        Room *room = zhoutai->getRoom();
        QList<int> buqu(zhoutai->getPile("buqu"));
        qSort(buqu.begin(), buqu.end(), compareByNumber);

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "buqu", QString());
        int need = 1 - zhoutai->getHp();
        if (need <= 0) {
            // clear all the buqu cards
            foreach(int card_id, buqu) {
                LogMessage log;
                log.type = "$BuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        } else {
            int to_remove = buqu.length() - need;
            for (int i = 0; i < to_remove; i++) {
                room->fillAG(buqu, zhoutai);
                int aidelay = Config.AIDelay;
                Config.AIDelay = 0;
                int card_id = room->askForAG(zhoutai, buqu, false, "buqu");
                Config.AIDelay = aidelay;
                LogMessage log;
                log.type = "$BuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                buqu.removeOne(card_id);
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                room->clearAG(zhoutai);
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *zhoutai, QVariant &, ServerPlayer* &) const{
        if (!zhoutai || !zhoutai->isAlive() || !zhoutai->hasSkill("buqu")) return QStringList();
        if (zhoutai->getPile("buqu").length() > 0)
            Remove(zhoutai);

        return QStringList();
    }
};

class Buqu : public TriggerSkill {
public:
    Buqu() : TriggerSkill("buqu") {
        events << PostHpReduced << AskForPeachesDone;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(zhoutai)) return QStringList();
        if (triggerEvent == PostHpReduced && zhoutai->getHp() < 1)
            return QStringList(objectName());
        else if (triggerEvent == AskForPeachesDone) {
            const QList<int> &buqu = zhoutai->getPile("buqu");

            if (zhoutai->getHp() > 0)
                return QStringList();
            if (room->getTag("Buqu").toString() != zhoutai->objectName())
                return QStringList();
            room->setTag("Buqu", QVariant());

            QList<int> duplicate_numbers;
            QSet<int> numbers;
            foreach(int card_id, buqu) {
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if (numbers.contains(number) && !duplicate_numbers.contains(number))
                    duplicate_numbers << number;
                else
                    numbers << number;
            }

            if (duplicate_numbers.isEmpty()) {
                room->broadcastSkillInvoke("buqu", zhoutai);
                room->setPlayerFlag(zhoutai, "-Global_Dying");
                return QStringList(objectName());
            } else {
                LogMessage log;
                log.type = "#BuquDuplicate";
                log.from = zhoutai;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for (int i = 0; i < duplicate_numbers.length(); i++) {
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#BuquDuplicateGroup";
                    log.from = zhoutai;
                    log.arg = QString::number(i + 1);
                    if (number == 10)
                        log.arg2 = "10";
                    else {
                        const char *number_string = "-A23456789-JQK";
                        log.arg2 = QString(number_string[number]);
                    }
                    room->sendLog(log);

                    foreach(int card_id, buqu) {
                        const Card *card = Sanguosha->getCard(card_id);
                        if (card->getNumber() == number) {
                            LogMessage log;
                            log.type = "$BuquDuplicateItem";
                            log.from = zhoutai;
                            log.card_str = QString::number(card_id);
                            room->sendLog(log);
                        }
                    }
                }
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == AskForPeachesDone)
            return true;
        if (room->askForSkillInvoke(zhoutai, objectName(), data)) {
            room->broadcastSkillInvoke(objectName(), zhoutai);
            const QList<int> &buqu = zhoutai->getPile("buqu");
            int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
            int n = need - buqu.length();
            if (n > 0) {
                QList<int> card_ids = room->getNCards(n, false);
                zhoutai->addToPile("buqu", card_ids);
            }
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &, ServerPlayer *) const{
        if (triggerEvent == AskForPeachesDone)
            return true;
        if (zhoutai->getHp() < 1) {
            room->setTag("Buqu", zhoutai->objectName());

            const QList<int> &buqunew = zhoutai->getPile("buqu");
            QList<int> duplicate_numbers;

            QSet<int> numbers;
            foreach(int card_id, buqunew) {
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if (numbers.contains(number))
                    duplicate_numbers << number;
                else
                    numbers << number;
            }

            if (duplicate_numbers.isEmpty()) {
                room->setTag("Buqu", QVariant());
                return true;
            }
        }
        return false;
    }
};

class BuquClear : public DetachEffectSkill {
public:
    BuquClear() : DetachEffectSkill("buqu") {
        frequency = Compulsory;
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        if (player->getHp() <= 0)
            room->enterDying(player, NULL);
    }
};

HaoshiCard::HaoshiCard() {
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
    m_skillName = "_haoshi";
}

bool HaoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    return to_select->getHandcardNum() == Self->getMark("haoshi");
}

void HaoshiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(),
        targets.first()->objectName(), "haoshi", QString());
    room->moveCardTo(this, targets.first(), Player::PlaceHand, reason);
}

class HaoshiViewAsSkill : public ViewAsSkill {
public:
    HaoshiViewAsSkill() : ViewAsSkill("haoshi") {
        response_pattern = "@@haoshi!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        HaoshiCard *card = new HaoshiCard;
        card->addSubcards(cards);
        return card;
    }
};

class Haoshi : public DrawCardsSkill {
public:
    Haoshi() : DrawCardsSkill("haoshi") {
        view_as_skill = new HaoshiViewAsSkill;
    }

    virtual bool canPreshow() const {
        return true;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke("haoshi")){
            room->broadcastSkillInvoke("haoshi", player);
            return true;
        }
        return false;
    }

    virtual int getDrawNum(ServerPlayer *lusu, int n) const{
        lusu->setFlags("haoshi");
        return n + 2;
    }
};

class HaoshiGive : public TriggerSkill {
public:
    HaoshiGive() : TriggerSkill("#haoshi-give") {
        events << AfterDrawNCards;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *lusu, QVariant &, ServerPlayer * &) const{
        if (!lusu || !lusu->isAlive() || !lusu->hasShownSkill("haoshi")) return QStringList();
        if (lusu->hasFlag("haoshi")) {
            lusu->setFlags("-haoshi");

            if (lusu->getHandcardNum() <= 5)
                return QStringList();

            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *lusu, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> other_players = room->getOtherPlayers(lusu);
        int least = 1000;
        foreach(ServerPlayer *player, other_players)
            least = qMin(player->getHandcardNum(), least);
        room->setPlayerMark(lusu, "haoshi", least);

        if (!room->askForUseCard(lusu, "@@haoshi!", "@haoshi", -1, Card::MethodNone)) {
            // force lusu to give his half cards
            ServerPlayer *beggar = NULL;
            foreach(ServerPlayer *player, other_players) {
                if (player->getHandcardNum() == least) {
                    beggar = player;
                    break;
                }
            }

            int n = lusu->getHandcardNum() / 2;
            QList<int> to_give = lusu->handCards().mid(0, n);
            HaoshiCard haoshi_card;
            haoshi_card.addSubcards(to_give);
            QList<ServerPlayer *> targets;
            targets << beggar;
            haoshi_card.use(room, lusu, targets);
        }
        return false;
    }
};

DimengCard::DimengCard() {
}

bool DimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self)
        return false;

    if (targets.isEmpty())
        return true;

    if (targets.length() == 1) {
        return qAbs(to_select->getHandcardNum() - targets.first()->getHandcardNum()) == subcardsLength();
    }

    return false;
}

bool DimengCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void DimengCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);
    a->setFlags("DimengTarget");
    b->setFlags("DimengTarget");

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    try {
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (p != a && p != b)
                room->doNotify(p, QSanProtocol::S_COMMAND_EXCHANGE_KNOWN_CARDS,
                JsonArray() << a->objectName() << b->objectName());
        }
        QList<CardsMoveStruct> exchangeMove;
        CardsMoveStruct move1(a->handCards(), b, Player::PlaceHand,
            CardMoveReason(CardMoveReason::S_REASON_SWAP, a->objectName(), b->objectName(), "dimeng", QString()));
        CardsMoveStruct move2(b->handCards(), a, Player::PlaceHand,
            CardMoveReason(CardMoveReason::S_REASON_SWAP, b->objectName(), a->objectName(), "dimeng", QString()));
        exchangeMove.push_back(move1);
        exchangeMove.push_back(move2);
        room->moveCards(exchangeMove, false);

        LogMessage log;
        log.type = "#Dimeng";
        log.from = a;
        log.to << b;
        log.arg = QString::number(n1);
        log.arg2 = QString::number(n2);
        room->sendLog(log);
        room->getThread()->delay();

        a->setFlags("-DimengTarget");
        b->setFlags("-DimengTarget");
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
            a->setFlags("-DimengTarget");
            b->setFlags("-DimengTarget");
        }
        throw triggerEvent;
    }
}

class Dimeng : public ViewAsSkill {
public:
    Dimeng() : ViewAsSkill("dimeng") {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        DimengCard *card = new DimengCard;
        card->addSubcards(cards);
        card->setShowSkill(objectName());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DimengCard");
    }
};

ZhijianCard::ZhijianCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhijianCard::extraCost(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *erzhang = card_use.from;
    room->moveCardTo(this, erzhang, card_use.to.first(), Player::PlaceEquip, CardMoveReason(CardMoveReason::S_REASON_PUT, erzhang->objectName(), "zhijian", QString()));

    LogMessage log;
    log.type = "$ZhijianEquip";
    log.from = card_use.to.first();
    log.card_str = QString::number(getEffectiveId());
    room->sendLog(log);
}

void ZhijianCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->drawCards(1);
}

class Zhijian : public OneCardViewAsSkill {
public:
    Zhijian() :OneCardViewAsSkill("zhijian") {
        filter_pattern = "EquipCard|.|.|hand";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ZhijianCard *zhijian_card = new ZhijianCard;
        zhijian_card->addSubcard(originalCard);
        zhijian_card->setShowSkill(objectName());
        return zhijian_card;
    }
};

class GuzhengRecord : public TriggerSkill {
public:
    GuzhengRecord() : TriggerSkill("#guzheng-record") {
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *erzhang, QVariant &data, ServerPlayer * &) const{
        if (!erzhang || !erzhang->isAlive() || !erzhang->hasSkill("guzheng")) return QStringList();
        ServerPlayer *current = room->getCurrent();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();

        if (erzhang == current)
            return QStringList();

        if (current->getPhase() == Player::Discard) {
            QVariantList guzhengToGet = erzhang->tag["GuzhengToGet"].toList();
            QVariantList guzhengOther = erzhang->tag["GuzhengOther"].toList();

            if ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                foreach (int card_id, move.card_ids) {
                    if (move.from == current) {
                        if (!guzhengToGet.contains(card_id))
                            guzhengToGet << card_id;
                    } else {
                        if (!guzhengOther.contains(card_id))
                            guzhengOther << card_id;
                    }
                }
            }

            erzhang->tag["GuzhengToGet"] = guzhengToGet;
            erzhang->tag["GuzhengOther"] = guzhengOther;
        }

        return QStringList();
    }
};

class Guzheng : public TriggerSkill {
public:
    Guzheng() : TriggerSkill("guzheng") {
        events << EventPhaseEnd;
        frequency = Frequent;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL || player->getPhase() != Player::Discard) return skill_list;
        QList<ServerPlayer *> erzhangs = room->findPlayersBySkillName(objectName());

        foreach(ServerPlayer *erzhang, erzhangs) {
            QVariantList guzheng_cardsToGet = erzhang->tag["GuzhengToGet"].toList();
            QVariantList guzheng_cardsOther = erzhang->tag["GuzhengOther"].toList();

            if (player->isDead())
                return QMap<ServerPlayer *, QStringList>();

            QList<int> cardsToGet;
            foreach(QVariant card_data, guzheng_cardsToGet) {
                int card_id = card_data.toInt();
                if (room->getCardPlace(card_id) == Player::DiscardPile)
                    cardsToGet << card_id;
            }
            QList<int> cardsOther;
            foreach(QVariant card_data, guzheng_cardsOther) {
                int card_id = card_data.toInt();
                if (room->getCardPlace(card_id) == Player::DiscardPile)
                    cardsOther << card_id;
            }

            if (cardsToGet.isEmpty()) {
                foreach(ServerPlayer *erzhang, erzhangs) {
                    erzhang->tag.remove("GuzhengToGet");
                    erzhang->tag.remove("GuzhengOther");
                }
                return QMap<ServerPlayer *, QStringList>();
            }

            skill_list.insert(erzhang, QStringList(objectName()));
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *erzhang) const {
        QVariantList guzheng_cardsToGet = erzhang->tag["GuzhengToGet"].toList();
        QVariantList guzheng_cardsOther = erzhang->tag["GuzhengOther"].toList();

        QList<int> cardsToGet;
        foreach(QVariant card_data, guzheng_cardsToGet) {
            int card_id = card_data.toInt();
            if (room->getCardPlace(card_id) == Player::DiscardPile)
                cardsToGet << card_id;
        }
        QList<int> cardsOther;
        foreach(QVariant card_data, guzheng_cardsOther) {
            int card_id = card_data.toInt();
            if (room->getCardPlace(card_id) == Player::DiscardPile)
                cardsOther << card_id;
        }

        QList<int> cards = cardsToGet + cardsOther;
        erzhang->tag.remove("GuzhengToGet");
        erzhang->tag.remove("GuzhengOther");
        if (erzhang->askForSkillInvoke(objectName(), cards.length())) {
            room->broadcastSkillInvoke(objectName(), erzhang);
            room->fillAG(cards, erzhang, cardsOther);

            int to_back = room->askForAG(erzhang, cardsToGet, false, objectName());
            player->obtainCard(Sanguosha->getCard(to_back));

            cards.removeOne(to_back);

            room->clearAG(erzhang);
            erzhang->tag["GuzhengCards"] = IntList2VariantList(cards);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *erzhang) const{
        QList<int> cards = VariantList2IntList(erzhang->tag["GuzhengCards"].toList());
        erzhang->tag.remove("GuzhengCards");
        if (!cards.isEmpty()) {
            DummyCard dummy(cards);
            room->obtainCard(erzhang, &dummy);
        }

        return false;
    }
};

class Duanbing : public TriggerSkill {  // Slash::targetFilter()
public:
    Duanbing() : TriggerSkill("duanbing") {
        frequency = Compulsory;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const{
        return QStringList();
    }
};

FenxunCard::FenxunCard() {
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FenxunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->tag["FenxunTarget"] = QVariant::fromValue(effect.to);
    room->setFixedDistance(effect.from, effect.to, 1);
}

class FenxunViewAsSkill : public OneCardViewAsSkill {
public:
    FenxunViewAsSkill() : OneCardViewAsSkill("fenxun") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("FenxunCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        FenxunCard *first = new FenxunCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        first->setShowSkill(objectName());
        return first;
    }
};

class Fenxun : public TriggerSkill {
public:
    Fenxun() : TriggerSkill("fenxun") {
        events << EventPhaseChanging << Death;
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *dingfeng, QVariant &data, ServerPlayer* &) const{
        if (dingfeng == NULL || dingfeng->tag["FenxunTarget"].value<ServerPlayer *>() == NULL) return QStringList();
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return QStringList();
        }
        else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != dingfeng)
                return QStringList();
        }
        ServerPlayer *target = dingfeng->tag["FenxunTarget"].value<ServerPlayer *>();

        if (target) {
            room->setFixedDistance(dingfeng, target, -1);
            dingfeng->tag.remove("FenxunTarget");
        }
        return QStringList();
    }
};

void StandardPackage::addWuGenerals()
{
    General *sunquan = new General(this, "sunquan", "wu"); // WU 001
    sunquan->addCompanion("zhoutai");
    sunquan->addSkill(new Zhiheng);

    General *ganning = new General(this, "ganning", "wu"); // WU 002
    ganning->addSkill(new Qixi);

    General *lvmeng = new General(this, "lvmeng", "wu"); // WU 003
    lvmeng->addSkill(new Keji);
    lvmeng->addSkill(new KejiRecord);
    insertRelatedSkills("keji", "#keji-record");

    General *huanggai = new General(this, "huanggai", "wu"); // WU 004
    huanggai->addSkill(new Kurou);

    General *zhouyu = new General(this, "zhouyu", "wu", 3); // WU 005
    zhouyu->addCompanion("huanggai");
    zhouyu->addCompanion("xiaoqiao");
    zhouyu->addSkill(new Yingzi);
    zhouyu->addSkill(new Fanjian);

    General *daqiao = new General(this, "daqiao", "wu", 3, false); // WU 006
    daqiao->addCompanion("xiaoqiao");
    daqiao->addSkill(new Guose);
    daqiao->addSkill(new Liuli);

    General *luxun = new General(this, "luxun", "wu", 3); // WU 007
    luxun->addSkill(new Qianxun);
    luxun->addSkill(new Duoshi);

    General *sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false); // WU 008
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    General *sunjian = new General(this, "sunjian", "wu"); // WU 009
    sunjian->addSkill(new Yinghun);

    General *xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false); // WU 011
    xiaoqiao->addSkill(new Tianxiang);
    xiaoqiao->addSkill(new TianxiangDraw);
    xiaoqiao->addSkill(new Hongyan);
    insertRelatedSkills("tianxiang", "#tianxiang");

    General *taishici = new General(this, "taishici", "wu"); // WU 012
    taishici->addSkill(new Tianyi);
    taishici->addSkill(new TianyiTargetMod);
    insertRelatedSkills("tianyi", "#tianyi-target");

    General *zhoutai = new General(this, "zhoutai", "wu");
    zhoutai->addSkill(new Buqu);
    zhoutai->addSkill(new BuquRemove);
    zhoutai->addSkill(new BuquClear);
    insertRelatedSkills("buqu", 2, "#buqu-remove", "#buqu-clear");

    General *lusu = new General(this, "lusu", "wu", 3); // WU 014
    lusu->addSkill(new Haoshi);
    lusu->addSkill(new HaoshiGive);
    lusu->addSkill(new Dimeng);
    insertRelatedSkills("haoshi", "#haoshi-give");

    General *erzhang = new General(this, "erzhang", "wu", 3); // WU 015
    erzhang->addSkill(new Zhijian);
    erzhang->addSkill(new GuzhengRecord);
    insertRelatedSkills("guzheng", "#guzheng-record");

    General *dingfeng = new General(this, "dingfeng", "wu"); // WU 016
    dingfeng->addSkill(new Duanbing);
    dingfeng->addSkill(new Fenxun);

    addMetaObject<ZhihengCard>();
    addMetaObject<KurouCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<TianxiangCard>();
    addMetaObject<TianyiCard>();
    addMetaObject<HaoshiCard>();
    addMetaObject<DimengCard>();
    addMetaObject<ZhijianCard>();
    addMetaObject<FenxunCard>();
}
