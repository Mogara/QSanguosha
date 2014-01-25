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
        rende_card->setShowSkill(objectName());
        return rende_card;
    }
};

class Rende: public TriggerSkill {
public:
    Rende(): TriggerSkill("rende") {
        events << EventPhaseChanging;
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer * &ask_who) const{
        return target != NULL && target->getMark("rende") > 0 && target->hasShownSkill(this);
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        room->setPlayerMark(player, "rende", 0);
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who /* = NULL */) const{
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

class Kongcheng: public TriggerSkill{
public:
    Kongcheng(): TriggerSkill("kongcheng"){
        events << TargetConfirming;
        frequency = Compulsory;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who /* = NULL */) const{
        if (TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who) && player->isKongcheng()){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && (use.card->isKindOf("Slash") || use.card->isKindOf("Duel")) && use.to.contains(player)){
                return true;
            }
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        use.to.removeOne(player);
        data = QVariant::fromValue(use);
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

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who /* = NULL */) const{
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

class NosQicai: public TargetModSkill {
public:
    NosQicai(): TargetModSkill("nosqicai") {
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

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
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


class KuangguGlobal: public TriggerSkill{
public:
    KuangguGlobal(): TriggerSkill("KuangguGlobal"){
        global = true;
        events << PreDamageDone;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who /* = NULL */) const{
        return player != NULL;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return true;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *weiyan = damage.from;
        weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;

        return false;
    }
};

class Kuanggu: public TriggerSkill {
public:
    Kuanggu(): TriggerSkill("kuanggu") {
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who /* = NULL */) const{
        if (TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who)){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            player->tag["InvokeKuanggu"] = false;
            return invoke && player->isWounded();
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        room->broadcastSkillInvoke(objectName());

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        RecoverStruct recover;
        recover.who = player;
        recover.recover = damage.damage;
        room->recover(player, recover);

        return false;
    }
};


class Lianhuan: public OneCardViewAsSkill {
public:
    Lianhuan(): OneCardViewAsSkill("lianhuan") {
        filter_pattern = ".|club|.|hand";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IronChain *chain = new IronChain(originalCard->getSuit(), originalCard->getNumber());
        chain->addSubcard(originalCard);
        chain->setSkillName(objectName());
        chain->setShowSkill(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill {
public:
    Niepan(): TriggerSkill("niepan") {
        events << AskForPeaches;
        frequency = Limited;
        limit_mark = "@nirvana";
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer * &ask_who /* = NULL */){
        if (TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0){
            DyingStruct dying_data = data.value<DyingStruct>();
            if (dying_data.who != target)
                return false;
            return true;
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *pangtong, QVariant &data) const{
        if (pangtong->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$NiepanAnimate");
            room->removePlayerMark(pangtong, "@nirvana");
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *pangtong, QVariant &data) const{
        pangtong->throwAllHandCardsAndEquips();
        QList<const Card *> tricks = pangtong->getJudgingArea();
        foreach (const Card *trick, tricks) {
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

        return false;
    }
};

class Huoji: public OneCardViewAsSkill {
public:
    Huoji(): OneCardViewAsSkill("huoji") {
        filter_pattern = ".|red|.|hand";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FireAttack *fire_attack = new FireAttack(originalCard->getSuit(), originalCard->getNumber());
        fire_attack->addSubcard(originalCard->getId());
        fire_attack->setSkillName(objectName());
        fire_attack->setShowSkill(objectName());
        return fire_attack;
    }
};

class Bazhen: public TriggerSkill {
public:
    Bazhen(): TriggerSkill("bazhen") {
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who /* = NULL */) const{
        if (!TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who))
            return false;

        QString pattern = data.toStringList().first();
        if (pattern != "jink")
            return false;

        if (!player->tag["Qinggang"].toStringList().isEmpty() || player->getMark("Armor_Nullified") > 0
            || player->getMark("Equips_Nullified_to_Yourself") > 0)
            return false;

        if (player->getArmor() == NULL && player->isAlive())
            return true;

        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (!player->hasShownSkill(this)){
            if (player->askForSkillInvoke("bazhen", "showgeneral")){
                if (player->ownSkill(objectName()) && !player->hasShownSkill(this))
                    player->showGeneral(player->inHeadSkills(objectName()));
            }
        }

        if (player->hasArmorEffect("bazhen")){
            return player->askForSkillInvoke("EightDiagram");
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *wolong, QVariant &data) const{
        //此处更改是因为“八阵”是“视为”装备八卦阵，真正发动的技能是八卦阵，而不是八阵。
        JudgeStruct judge;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = "EightDiagram";
        judge.who = wolong;

        room->setEmotion(wolong, "armor/eight_diagram");
        room->judge(judge);

        if (judge.isGood()) {
            Jink *jink = new Jink(Card::NoSuit, 0);
            jink->setSkillName("EightDiagram");
            room->broadcastSkillInvoke(objectName());
            room->provide(jink);
            return true;
        }


        return false;
    }
};

class Kanpo: public OneCardViewAsSkill {
public:
    Kanpo(): OneCardViewAsSkill("kanpo") {
        filter_pattern = ".|black|.|hand";
        response_pattern = "nullification";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());
        ncard->setShowSkill(objectName());
        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        foreach (const Card *card, player->getHandcards()) {
            if (card->isBlack()) return true;
        }
        return false;
    }
};

SavageAssaultAvoid::SavageAssaultAvoid(const QString &avoid_skill): TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill){
    events << CardEffected;
}

bool SavageAssaultAvoid::triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
    if (!TriggerSkill::triggerable(player)) return false;
    CardEffectStruct effect = data.value<CardEffectStruct>();
    if (effect.card->isKindOf("SavageAssault"))
        return true;

    return false;
}

bool SavageAssaultAvoid::cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
    if (player->hasShownSkill(Sanguosha->getSkill(avoid_skill))) return true;
    return player->askForSkillInvoke(avoid_skill);
}

bool SavageAssaultAvoid::effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
    room->broadcastSkillInvoke(avoid_skill);

    LogMessage log;
    log.type = "#SkillNullify";
    log.from = player;
    log.arg = avoid_skill;
    log.arg2 = "savage_assault";
    room->sendLog(log);

    return true;
}

class Huoshou: public TriggerSkill {
public:
    Huoshou(): TriggerSkill("huoshou") {
        events << TargetConfirmed << ConfirmDamage << CardFinished;
        frequency = Compulsory;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (player == NULL) return false;
        if (triggerEvent == TargetConfirmed && TriggerSkill::triggerable(player)) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault") && use.from != player)
                return true;
        } else if (triggerEvent == ConfirmDamage && !room->getTag("HuoshouSource").isNull()) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->isKindOf("SavageAssault"))
                return false;

            ServerPlayer *menghuo = room->getTag("HuoshouSource").value<PlayerStar>();
            damage.from = menghuo->isAlive() ? menghuo : NULL;
            data = QVariant::fromValue(damage);
        } else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault"))
                room->removeTag("HuoshouSource");
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->hasShownSkill(this)) return true;
        return player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        room->notifySkillInvoked(player, objectName());
        room->broadcastSkillInvoke(objectName());
        room->setTag("HuoshouSource", QVariant::fromValue((PlayerStar)player));

        return false;
    }
};

class Zaiqi: public PhaseChangeSkill {
public:
    Zaiqi(): PhaseChangeSkill("zaiqi") {
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *menghuo, QVariant &data, ServerPlayer* &ask_who) const{
        if (menghuo->getPhase() == Player::Draw && menghuo->isWounded())
            return true;
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *menghuo) const{
        Room *room = menghuo->getRoom();
        room->broadcastSkillInvoke(objectName(), 1);

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
            DummyCard *dummy = new DummyCard(card_to_throw);

            RecoverStruct recover;
            recover.who = menghuo;
            recover.recover = card_to_throw.length();
            room->recover(menghuo, recover);

            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, menghuo->objectName(), "zaiqi", QString());
            room->throwCard(dummy, reason, NULL);
            dummy->deleteLater();
            has_heart = true;
        }
        if (!card_to_gotback.isEmpty()) {
            DummyCard *dummy2 = new DummyCard(card_to_gotback);
            CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, menghuo->objectName());
            room->obtainCard(menghuo, dummy2, reason);
            dummy2->deleteLater();
        }

        if (has_heart)
            room->broadcastSkillInvoke(objectName(), 2);

        return true;
    }
};

class Juxiang: public TriggerSkill {
public:
    Juxiang(): TriggerSkill("juxiang") {
        events << CardUsed << BeforeCardsMove;
        frequency = Compulsory;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (player == NULL) return false;
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault")) {
                if (use.card->isVirtualCard() && use.card->subcardsLength() != 1)
                    return false;
                if (Sanguosha->getEngineCard(use.card->getEffectiveId())
                    && Sanguosha->getEngineCard(use.card->getEffectiveId())->isKindOf("SavageAssault"))
                    room->setCardFlag(use.card->getEffectiveId(), "real_SA");
            }
        } else if (TriggerSkill::triggerable(player)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.card_ids.length() == 1 && move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
                && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
                Card *card = Sanguosha->getCard(move.card_ids.first());
                if (card->hasFlag("real_SA") && player != move.from)
                    return true;
            }
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->hasShownSkill(this)) return true;
        return player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        room->notifySkillInvoked(player, objectName());
        room->broadcastSkillInvoke(objectName());
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        player->obtainCard(Sanguosha->getCard(move.card_ids.first()));
        move.card_ids.clear();
        data = QVariant::fromValue(move);

        return false;
    }
};

class Lieren: public TriggerSkill {
public:
    Lieren(): TriggerSkill("lieren") {
        events << Damage;
    }
    
    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhurong, QVariant &data, ServerPlayer* &ask_who) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash") && !zhurong->isKongcheng()
            && !damage.to->isKongcheng() && damage.to != zhurong && !damage.chain && !damage.transfer)
            return true;
        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *zhurong, QVariant &data) const{
        return zhurong->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;

        room->broadcastSkillInvoke(objectName(), 1);

        bool success = zhurong->pindian(target, "lieren", NULL);
        if (!success) return false;

        room->broadcastSkillInvoke(objectName(), 2);
        if (!target->isNude()) {
            int card_id = room->askForCardChosen(zhurong, target, "he", objectName());
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, zhurong->objectName());
            room->obtainCard(zhurong, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
        }

        return false;
    }
};

class Shushen: public TriggerSkill {
public:
    Shushen(): TriggerSkill("shushen") {
        events << HpRecover;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "shushen-invoke", true, true);
        if (target != NULL){
            player->tag["shushen_invoke"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover_struct = data.value<RecoverStruct>();

        doShushen(player);
        int recover = recover_struct.recover;
        for (int i = 1; i < recover; i++) {
            if (cost(triggerEvent, room, player, data)){
                doShushen(player);
            }
            else
                break;
        }
        return false;
    }

private:
    void doShushen(ServerPlayer *ganfuren) const{
        ServerPlayer *target = ganfuren->tag["shushen_invoke"].value<ServerPlayer *>();
        if (target != NULL)
            target->drawCards(1);
        ganfuren->tag.remove("shushen_invoke");
    }
};

class Shenzhi: public PhaseChangeSkill {
public:
    Shenzhi(): PhaseChangeSkill("shenzhi") {

    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const{
        if (!PhaseChangeSkill::triggerable(triggerEvent, room, player, data, ask_who))
            return false;
        if (player->getPhase() != Player::Start || player->isKongcheng())
            return false;

        foreach (const Card *card, player->getHandcards()){
            if (player->isJilei(card))
                return false;
        }
        return true;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->askForSkillInvoke(objectName())){
            int handcard_num = player->getHandcardNum();
            player->tag["shenzhi_num"] = handcard_num;
            player->throwAllHandCards();
            return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *ganfuren) const{
        int handcard_num = ganfuren->tag["shenzhi_num"].toInt();
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

    General *zhugeliang = new General(this, "zhugeliang", "shu", 3); // SHU 004
    zhugeliang->addCompanion("huangyueying");
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);

    General *zhaoyun = new General(this, "zhaoyun", "shu"); // SHU 005
    zhaoyun->addCompanion("liushan");
    zhaoyun->addSkill(new Longdan);

    General *machao = new General(this, "machao", "shu"); // SHU 006
    machao->addSkill(new Tieji);
    machao->addSkill(new Mashu("machao"));

    General *huangyueying = new General(this, "huangyueying", "shu", 3, false); // SHU 007
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill(new NosQicai);

    General *huangzhong = new General(this, "huangzhong", "shu"); // SHU 008
    huangzhong->addCompanion("weiyan");
    huangzhong->addSkill(new Liegong);

    General *weiyan = new General(this, "weiyan", "shu"); // SHU 009
    weiyan->addSkill(new Kuanggu);

    General *pangtong = new General(this, "pangtong", "shu", 3); // SHU 010
    pangtong->addSkill(new Lianhuan);
    pangtong->addSkill(new Niepan);

    General *wolong = new General(this, "wolong", "shu", 3); // SHU 011
    wolong->addCompanion("huangyueying");
    wolong->addCompanion("pangtong");
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Kanpo);
    wolong->addSkill(new Bazhen);

    General *menghuo = new General(this, "menghuo", "shu"); // SHU 014
    menghuo->addCompanion("zhurong");
    menghuo->addSkill(new SavageAssaultAvoid("huoshou"));
    menghuo->addSkill(new Huoshou);
    menghuo->addSkill(new Zaiqi);
    related_skills.insertMulti("huoshou", "#sa_avoid_huoshou");

    General *zhurong = new General(this, "zhurong", "shu", 4, false); // SHU 015
    zhurong->addSkill(new SavageAssaultAvoid("juxiang"));
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);
    related_skills.insertMulti("juxiang", "#sa_avoid_juxiang");

    General *ganfuren = new General(this, "ganfuren", "shu", 3, false); // SHU 016
    ganfuren->addSkill(new Shushen);
    ganfuren->addSkill(new Shenzhi);

    addMetaObject<RendeCard>();

    skills << new Qicai << new KuangguGlobal;
}