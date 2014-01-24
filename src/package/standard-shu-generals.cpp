#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"

RendeCard::RendeCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void RendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->broadcastSkillInvoke("rende");
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "rende", QString());
    room->obtainCard(target, this, reason, false);

    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if (old_value < 2 && new_value >= 2) {
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

class RendeViewAsSkill: public ViewAsSkill {
public:
    RendeViewAsSkill(): ViewAsSkill("rende") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (ServerInfo.GameMode == "04_1v3" && selected.length() + Self->getMark("rende") >= 2)
            return false;
        else
            return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (ServerInfo.GameMode == "04_1v3" && player->getMark("rende") >= 2)
           return false;
        return !player->isKongcheng();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        RendeCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public TriggerSkill {
public:
    Rende(): TriggerSkill("rende") {
        events << EventPhaseChanging;
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getMark("rende") > 0;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        room->setPlayerMark(player, "rende", 0);
        return false;
    }
};

class Wusheng: public OneCardViewAsSkill {
public:
    Wusheng(): OneCardViewAsSkill("wusheng") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *card) const{
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

class Paoxiao: public TargetModSkill {
public:
    Paoxiao(): TargetModSkill("paoxiao") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class Guanxing: public PhaseChangeSkill {
public:
    Guanxing(): PhaseChangeSkill("guanxing") {
        frequency = Frequent;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        return TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who) && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->askForSkillInvoke(objectName(), data)){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        Room *room = zhuge->getRoom();
        QList<int> guanxing = room->getNCards(getGuanxingNum(room));

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = zhuge;
        log.card_str = IntList2StringList(guanxing).join("+");
        room->doNotify(zhuge, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

        room->askForGuanxing(zhuge, guanxing, false);


        return false;
    }

    virtual int getGuanxingNum(Room *room) const{
        return qMin(5, room->alivePlayerCount()); //consider jiangwei's yizhi
    }
};

class Kongcheng: public ProhibitSkill {
public:
    Kongcheng(): ProhibitSkill("kongcheng") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && (card->isKindOf("Slash") || card->isKindOf("Duel")) && to->isKongcheng();
    }
};

class KongchengEffect: public TriggerSkill {
public:
    KongchengEffect() :TriggerSkill("#kongcheng-effect") {
        events << CardsMoveOneTime;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->isKongcheng()) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && move.from_places.contains(Player::PlaceHand))
                room->broadcastSkillInvoke("kongcheng");
        }

        return false;
    }
};

class Longdan: public OneCardViewAsSkill {
public:
    Longdan(): OneCardViewAsSkill("longdan") {
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
        } else if (originalCard->isKindOf("Jink")) {
            Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
            slash->addSubcard(originalCard);
            slash->setSkillName(objectName());
            slash->setShowSkill(objectName());
            return slash;
        } else
            return NULL;
    }
};

Mashu::Mashu(const QString &owner): DistanceSkill("mashu_" + owner), owner(owner) {
}

int Mashu::getCorrect(const Player *from, const Player *) const{
        if (from->hasSkill(objectName()) && from->hasShownSkill(this))
            return -1;
        else
            return 0;
}

class Tieji: public TriggerSkill {
public:
    Tieji(): TriggerSkill("tieji") {
        events << TargetConfirmed;
    }

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (!TriggerSkill::triggerable(player))
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if (player != use.from || !use.card->isKindOf("Slash"))
            return false;

        player->tag["TiejiCurrentTarget"] = QVariant::fromValue(use.to.first());
        return true;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (player->askForSkillInvoke(objectName(), player->tag.value("TiejiCurrentTarget"))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> targets = use.to;
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        foreach (ServerPlayer *p, targets) {
            if (targets.indexOf(p) == 0)
                doTieji(p, player, use, jink_list);
            else {
                player->tag["TiejiCurrentTarget"] = QVariant::fromValue(p);
                if (cost(TriggerEvent, room, player, data)) {
                    doTieji(p, player, use, jink_list);
                }
            }
        }

        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        player->tag.remove("TiejiCurrentTarget");
        return false;
    }

private:
    void doTieji(ServerPlayer *target, ServerPlayer *source, CardUseStruct use, QVariantList &jink_list) const {
        Room *room = target->getRoom();
        target->setFlags("TiejiTarget"); // For AI

        int index = use.to.indexOf(target);

        JudgeStruct judge;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = objectName();
        judge.who = source;

        try {
            room->judge(judge);
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                target->setFlags("-TiejiTarget");
            throw triggerEvent;
        }

        if (judge.isGood()) {
            LogMessage log;
            log.type = "#NoJink";
            log.from = target;
            room->sendLog(log);

            jink_list.replace(index, QVariant(0));
        }

        target->setFlags("-TiejiTarget");
    }
};

class Jizhi: public TriggerSkill {
public:
    Jizhi(): TriggerSkill("jizhi") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (!TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who))
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card != NULL && use.card->isNDTrick()){
            if (!use.card->isVirtualCard())
                return true;
            else if (use.card->getSubcards().length() == 1){
                if (Sanguosha->getCard(use.card->getEffectiveId())->objectName() == use.card->objectName())
                    return true;
            }
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->askForSkillInvoke(objectName(), data)){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerevent, Room *room, ServerPlayer *player, QVariant &data) const{
        player->drawCards(1);
        return false;
    }
};

class Qicai: public TargetModSkill {
public:
    Qicai(): TargetModSkill("qicai") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class Liegong: public TriggerSkill {
public:
    Liegong(): TriggerSkill("liegong") {
        events << TargetConfirmed;
    }

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (!TriggerSkill::triggerable(player))
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if (player != use.from || player->getPhase() != Player::Play || !use.card->isKindOf("Slash"))
            return false;

        ServerPlayer *first;
        foreach (ServerPlayer *p, use.to) {
            int handcardnum = p->getHandcardNum();
            if (player->getHp() <= handcardnum || player->getAttackRange() >= handcardnum) {
                first = p;
                break;
            }
        }

        if (first != NULL) {
            player->tag["LiegongCurrentTarget"] = QVariant::fromValue(use.to.first());
            return true;
        }

        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (player->askForSkillInvoke(objectName(), player->tag.value("LiegongCurrentTarget"))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> targets = use.to;
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        foreach (ServerPlayer *p, targets) {
            if (targets.indexOf(p) == 0)
                doLiegong(p, use, jink_list);
            else {
                int handcardnum = p->getHandcardNum();
                if (player->getHp() <= handcardnum || player->getAttackRange() >= handcardnum) {
                    player->tag["LiegongCurrentTarget"] = QVariant::fromValue(p);
                    if (cost(TriggerEvent, room, player, data)) {
                        doLiegong(p, use, jink_list);
                    }
                }
            }
        }

        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        player->tag.remove("LiegongCurrentTarget");
        return false;
    }

private:
    void doLiegong(ServerPlayer *target, CardUseStruct use, QVariantList &jink_list) const {
        int index = use.to.indexOf(target);
        LogMessage log;
        log.type = "#NoJink";
        log.from = target;
        target->getRoom()->sendLog(log);
        jink_list.replace(index, QVariant(0));
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

    General *zhugeliang = new General(this, "zhugeliang", "shu", 3); // SHU 004
    zhugeliang->addCompanion("huangyueying");
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);
    zhugeliang->addSkill(new KongchengEffect);
    related_skills.insertMulti("kongcheng", "#kongcheng-effect");

    General *zhaoyun = new General(this, "zhaoyun", "shu"); // SHU 005
    zhaoyun->addCompanion("liushan");
    zhaoyun->addSkill(new Longdan);

    General *machao = new General(this, "machao", "shu"); // SHU 006
    machao->addSkill(new Tieji);
    machao->addSkill(new Mashu("machao"));

    General *huangyueying = new General(this, "huangyueying", "shu", 3, false); // SHU 007
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill("nosqicai");

    General *huangzhong = new General(this, "huangzhong", "shu"); // SHU 008
    huangzhong->addCompanion("weiyan");
    huangzhong->addSkill(new Liegong);

    addMetaObject<RendeCard>();

    skills << new Qicai;
}