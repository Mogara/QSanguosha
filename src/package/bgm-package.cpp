#include "bgm-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class ChongZhen: public TriggerSkill{
public:
    ChongZhen(): TriggerSkill("chongzhen"){
        events << CardResponsed  << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == CardResponsed){
            ResponsedStruct resp = data.value<ResponsedStruct>();
            QVariant toChongzhen = QVariant::fromValue((PlayerStar)resp.m_who);
            if(resp.m_card->getSkillName() == "longdan"
                    && resp.m_who != NULL && !resp.m_who->isKongcheng()
                    && player->askForSkillInvoke(objectName(), toChongzhen))
            {
                room->broadcastSkillInvoke(objectName());
                int card_id = room->askForCardChosen(player, resp.m_who, "h", objectName());
                CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
            }
        }
        else{
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.from == player && use.card->getSkillName() == "longdan"){
                foreach(ServerPlayer *p, use.to){
                    QVariant toChongzhen = QVariant::fromValue((PlayerStar)p);
                    if(p->isKongcheng() || !player->askForSkillInvoke(objectName(), toChongzhen)) continue;
                    room->broadcastSkillInvoke(objectName());
                    int card_id = room->askForCardChosen(player, p, "h", objectName());
                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                    room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                }
            }
        }
        return false;
    }
};

LihunCard::LihunCard(){
    mute = true;
}

bool LihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!to_select->isMale())
        return false;

    if(!targets.isEmpty())
        return false;

    return true;
}

void LihunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->turnOver();

    DummyCard *dummy_card = new DummyCard;
    foreach(const Card *cd, effect.to->getHandcards()){
        dummy_card->addSubcard(cd);
    }
    if (!effect.to->isKongcheng())
    {
        if(effect.to->getGeneralName().contains("lvbu"))
            room->broadcastSkillInvoke("lihun", 2);
        else
            room->broadcastSkillInvoke("lihun", 1);
        CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, effect.from->objectName(),
            effect.to->objectName(), "lihun", QString());
        room->moveCardTo(dummy_card, effect.to, effect.from, Player::PlaceHand, reason, false);
    }
    effect.to->setFlags("LihunTarget");
}

class LihunSelect: public OneCardViewAsSkill{
public:
    LihunSelect():OneCardViewAsSkill("lihun"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LihunCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new LihunCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Lihun: public TriggerSkill{
public:
    Lihun():TriggerSkill("lihun"){
        events << EventPhaseEnd;
        view_as_skill = new LihunSelect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasUsed("LihunCard");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *diaochan, QVariant &) const{
        if(diaochan->getPhase() == Player::Play){
            ServerPlayer *target = NULL;
            foreach(ServerPlayer *other, room->getOtherPlayers(diaochan)){
                if(other->hasFlag("LihunTarget")){
                    other->setFlags("-LihunTarget");
                    target = other;
                    break;
                }
            }

            if(!target || target->getHp() < 1 || diaochan->isNude())
                return false;

            DummyCard *to_goback;
            if(diaochan->getCardCount(true) <= target->getHp())
            {
                to_goback = diaochan->isKongcheng() ? new DummyCard : diaochan->wholeHandCards();
                for (int i = 0;i < 4; i++)
                    if(diaochan->getEquip(i))
                        to_goback->addSubcard(diaochan->getEquip(i)->getEffectiveId());
            }
            else
                to_goback = (DummyCard*)room->askForExchange(diaochan, objectName(), target->getHp(), true, "LihunGoBack");

            CardMoveReason reason(CardMoveReason::S_REASON_GIVE, diaochan->objectName(),
                                  target->objectName(), objectName(), QString());
            reason.m_playerId = target->objectName();
            room->moveCardTo(to_goback, diaochan, target, Player::PlaceHand, reason);
            delete to_goback;
        }

        return false;
    }
};

class Kuiwei: public TriggerSkill{
public:
    Kuiwei(): TriggerSkill("kuiwei"){
        events << EventPhaseStart;
    }
	
    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->isAlive() && (target->hasSkill(objectName()) || target->getMark("@kuiwei")>0);
    }

    virtual int getPriority() const{
        return 3;
    }

    int getWeaponCount(ServerPlayer *caoren) const{
        int n = 0;
        foreach(ServerPlayer *p, caoren->getRoom()->getAlivePlayers()){
            if(p->getWeapon())
                n ++;
        }

        return n;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *caoren, QVariant &) const{
        if(caoren->hasSkill(objectName()) && caoren->getPhase() == Player::Finish){
            if(!caoren->askForSkillInvoke(objectName()))
                return false;

            int n = getWeaponCount(caoren);
            caoren->drawCards(n+2);
            caoren->turnOver();
            room->broadcastSkillInvoke("kuiwei", 1);
            if(caoren->getMark("@kuiwei") == 0)
                caoren->gainMark("@kuiwei");
        }
        else if(caoren->getPhase() == Player::Draw){
            if(caoren->getMark("@kuiwei") == 0)
                return false;

            int n = getWeaponCount(caoren);
            if(n > 0){
                LogMessage log;
                log.type = "#KuiweiDiscard";
                log.from = caoren;
                log.arg = QString::number(n);
                log.arg2 = objectName();
                room->sendLog(log);
                room->broadcastSkillInvoke("kuiwei", 2);
                if(caoren->getCards("he").length() <= n){
                    caoren->throwAllHandCardsAndEquips();
                }
                else{
                    room->askForDiscard(caoren, objectName(), n, n, false, true);
                }
            }

            caoren->loseMark("@kuiwei");
        }
        return false;
    }
};

class Yanzheng: public OneCardViewAsSkill{
public:
    Yanzheng():OneCardViewAsSkill("yanzheng"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" && player->getHandcardNum() > player->getHp();
    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());

        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        foreach(const Card *card, player->getHandcards()){
            if(card->objectName() == "nullification")
                return true;
        }

        return player->getHandcardNum() > player->getHp() && !player->getEquips().isEmpty();
    }
};

class Manjuan: public TriggerSkill{
public:
    Manjuan(): TriggerSkill("manjuan"){
        events << CardsMoveOneTime << CardDrawing;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return 2;
    }

    void doManjuan(ServerPlayer *sp_pangtong, int card_id) const{
        Room *room = sp_pangtong->getRoom();
        sp_pangtong->setFlags("ManjuanInvoke");
        QList<int> DiscardPile = room->getDiscardPile(), toGainList;
        const Card *card = Sanguosha->getCard(card_id);
        foreach(int id, DiscardPile){
            const Card *cd = Sanguosha->getCard(id);
            if(cd->getNumber() == card->getNumber())
                toGainList << id;
        }

        room->fillAG(toGainList, sp_pangtong);
        int id = room->askForAG(sp_pangtong, toGainList, false, objectName());
        if(id != -1)
            room->moveCardTo(Sanguosha->getCard(id), sp_pangtong, Player::PlaceHand, true);

        sp_pangtong->invoke("clearAG");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *sp_pangtong, QVariant &data) const{
        if(sp_pangtong->hasFlag("ManjuanInvoke")){
            sp_pangtong->setFlags("-ManjuanInvoke");
            return false;
        }

        int card_id = -1;
        CardMoveReason reason(CardMoveReason::S_REASON_PUT, sp_pangtong->objectName(), "manjuan", QString());
        if(triggerEvent == CardsMoveOneTime){
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if(move->to != sp_pangtong || move->to_place != Player::PlaceHand || move->to == move->from)
                return false;
            room->broadcastSkillInvoke(objectName());
            foreach(int card_id, move->card_ids){
                const Card* card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, NULL, NULL, Player::DiscardPile, reason);
            }
        }
        else if(triggerEvent == CardDrawing){
            if(room->getTag("FirstRound").toBool())
                return false;
            room->broadcastSkillInvoke(objectName());
            card_id = data.toInt();
            const Card* card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, NULL, NULL, Player::DiscardPile, reason);
        }

        LogMessage log;
        log.type = "$ManjuanGot";
        log.from = sp_pangtong;
        if(triggerEvent == CardsMoveOneTime)
        {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            foreach(int _card_id, move->card_ids)
                if(log.card_str.isEmpty())
                    log.card_str = QString::number(_card_id);
                else
                    log.card_str += "+" + QString::number(_card_id);
        }
        else
            log.card_str = Sanguosha->getCard(card_id)->toString();
        room->sendLog(log);

        if(sp_pangtong->getPhase() == Player::NotActive || !sp_pangtong->askForSkillInvoke(objectName(), data))
            return triggerEvent == CardsMoveOneTime ? false : true;

        if(triggerEvent == CardsMoveOneTime)
        {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            foreach(int _card_id, move->card_ids)
                doManjuan(sp_pangtong, _card_id);
        }
        else doManjuan(sp_pangtong, card_id);
        return triggerEvent == CardsMoveOneTime ? false : true;
    }
};

class Zuixiang: public TriggerSkill{
public:
    Zuixiang(): TriggerSkill("zuixiang"){
        events << EventPhaseStart << CardEffected ;
        frequency = Limited;

        type[Card::Basic] = "BasicCard";
        type[Card::Trick] = "TrickCard";
        type[Card::Equip] = "EquipCard";
    }

    void doZuixiang(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->broadcastSkillInvoke("zuixiang");
        if (player->getPile("dream").isEmpty()) {
            room->broadcastInvoke("animate", "lightbox:$zuixiang:3000");
            room->getThread()->delay(3000);
        }

        QList<int> ids = room->getNCards(3);
        player->addToPile("dream", ids, true);
        QSet<QString> lockedCategories;
        foreach(int id, ids){
            const Card *cd = Sanguosha->getCard(id);
            lockedCategories.insert(type[cd->getTypeId()]);            
        }
        foreach (QString s, lockedCategories)
            room->setPlayerCardLock(player, s);
        room->getThread()->delay();

        QList<int> zuixiang = player->getPile("dream");
        QSet<int> numbers;
        bool zuixiangDone = false;
        foreach(int id, zuixiang){
            const Card *card = Sanguosha->getCard(id);
            if(numbers.contains(card->getNumber())){
                zuixiangDone = true;
                break;
            }            
            numbers.insert(card->getNumber());
        }
        if (zuixiangDone)
        {
            player->addMark("zuixiangHasTrigger");
            room->setPlayerCardLock(player, ".");
            CardMoveReason reason(CardMoveReason::S_REASON_PUT, player->objectName(), QString(), "zuixiang", "");
            CardsMoveStruct move(zuixiang, player, Player::PlaceHand, reason);
            room->moveCardsAtomic(move, true);
        }
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *sp_pangtong, QVariant &data) const{
        QList<int> zuixiang = sp_pangtong->getPile("dream");

        if(triggerEvent == EventPhaseStart && sp_pangtong->getMark("zuixiangHasTrigger") == 0){
            if(sp_pangtong->getPhase() == Player::Start){
                if(sp_pangtong->getMark("@sleep") == 1){
                    if(!sp_pangtong->askForSkillInvoke(objectName()))
                        return false;
                    sp_pangtong->loseMark("@sleep", 1);
                    doZuixiang(sp_pangtong);
                }else
                    doZuixiang(sp_pangtong);
            }
        }
        else if(triggerEvent == CardEffected){
            if(zuixiang.isEmpty())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(sp_pangtong->hasCardLock(type[effect.card->getTypeId()])){
                LogMessage log;
                log.type = effect.from ? "#ZuiXiang1" : "#ZuiXiang2";
                log.from = effect.to;
                if(effect.from)
                    log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();

                room->sendLog(log);
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        return false;
    }

private:
    QMap<Card::CardType, QString> type;
};

class Jie: public TriggerSkill{
public:
    Jie():TriggerSkill("jie"){
        events << ConfirmDamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->isKindOf("Slash") || !damage.card->isRed())
            return false;

        LogMessage log;
        log.type = "#Jie";
        log.from = player;
        log.to << damage.to;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);
        data = QVariant::fromValue(damage);

        return false;
    }
};

DaheCard::DaheCard(){
    once = true;
    will_throw = false;
}

bool DaheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void DaheCard::use(Room *room, ServerPlayer *zhangfei, QList<ServerPlayer *> &targets) const{
    zhangfei->pindian(targets.first(), "dahe", this);
}

class DaheViewAsSkill: public OneCardViewAsSkill{
public:
    DaheViewAsSkill():OneCardViewAsSkill("dahe"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaheCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *daheCard = new DaheCard;
        daheCard->addSubcard(originalCard);
        return daheCard;
    }
};

class Dahe: public TriggerSkill{
public:
    Dahe():TriggerSkill("dahe"){
        events << SlashProceed << EventPhaseStart;
        view_as_skill = new DaheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *bgm_zhangfei = room->findPlayerBySkillName(objectName());
        if(!bgm_zhangfei)
            return false;
        if(triggerEvent == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!effect.to->hasFlag(objectName()))
                return false;
            const Card *jink = room->askForCard(effect.to, "jink",
                                                QString("@dahe-jink:%1:%2:%3")
                                                .arg(effect.from->objectName())
                                                .arg(bgm_zhangfei->objectName())
                                                .arg(objectName()),
                                                data,
                                                CardUsed,
                                                bgm_zhangfei);
            if(jink && jink->getSuit() != Card::Heart){
                LogMessage log;
                log.type = "#DaheEffect";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = jink->getSuitString();
                log.arg2 = objectName();
                room->sendLog(log);

                room->slashResult(effect, NULL);
            } else
                room->slashResult(effect, jink);

            return true;
        }
        else if(triggerEvent == EventPhaseStart && bgm_zhangfei->getPhase() == Player::NotActive){
            foreach(ServerPlayer *other, room->getOtherPlayers(player))
                if(other->hasFlag(objectName()))
                    room->setPlayerFlag(other, "-" + objectName());
        }
        return false;
    }
};

class DahePindian: public TriggerSkill{
public:
    DahePindian():TriggerSkill("#dahe"){
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason != "dahe" || !pindian->from->hasSkill(objectName()))
            return false;

        if(pindian->isSuccess()){
            room->setPlayerFlag(pindian->to, "dahe");
            QList<ServerPlayer *> to_givelist = room->getAlivePlayers();
            foreach(ServerPlayer *p, to_givelist){
                if(p->getHp() > pindian->from->getHp())
                    to_givelist.removeOne(p);
            }
            QString choice = room->askForChoice(pindian->from, "dahe", "yes+no");
            if(!to_givelist.isEmpty() && choice == "yes"){
                ServerPlayer *to_give = room->askForPlayerChosen(pindian->from, to_givelist, "dahe");
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, pindian->from->objectName());
                reason.m_playerId = to_give->objectName();
                to_give->obtainCard(pindian->to_card);
            }
        }else{
            if(!pindian->from->isKongcheng()){
                room->showAllCards(pindian->from);
                room->askForDiscard(pindian->from, "dahe", 1, 1, false, false);
            }
        }
        return false;
    }
};

TanhuCard::TanhuCard(){
    once = true;
    will_throw = false;
}

bool TanhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TanhuCard::use(Room *room, ServerPlayer *lvmeng, QList<ServerPlayer *> &targets) const{
    bool success = lvmeng->pindian(targets.first(), "tanhu", this);
    if(success){
        room->broadcastSkillInvoke("tanhu", 2);
        room->setPlayerFlag(targets.first(), "TanhuTarget");
        room->setFixedDistance(lvmeng, targets.first(), 1);
    }
    else {
        room->broadcastSkillInvoke("tanhu", 3);
    }
}

class TanhuViewAsSkill: public OneCardViewAsSkill{
public:
    TanhuViewAsSkill():OneCardViewAsSkill("tanhu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TanhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *newCard = new TanhuCard;
        newCard->addSubcard(originalCard);
        return newCard;
    }
};

class Tanhu: public PhaseChangeSkill{
public:
    Tanhu():PhaseChangeSkill("tanhu"){
        view_as_skill = new TanhuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAlivePlayers();

            foreach(ServerPlayer *player, players){
                if(player->hasFlag("TanhuTarget")){
                    room->setPlayerFlag(player, "-TanhuTarget");
                    room->setFixedDistance(target, player, -1);
                }
            }
        }

        return false;
    }
	
	virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class MouduanStart: public GameStartSkill{
public:
    MouduanStart():GameStartSkill("#mouduan"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual void onGameStart(ServerPlayer *lvmeng) const{
        Room *room = lvmeng->getRoom();
        lvmeng->gainMark("@wu");
        room->acquireSkill(lvmeng, "jiang");
        room->acquireSkill(lvmeng, "qianxun");
    }
};

class Mouduan: public TriggerSkill{
public:
    Mouduan():TriggerSkill("mouduan"){
        events << TurnStart << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *lvmeng = room->findPlayerBySkillName(objectName());

        if(triggerEvent == CardsMoveOneTime){
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if(move->from == player && (player->getMark("@wu") > 0) && player->getHandcardNum() <= 2){
                room->broadcastSkillInvoke(objectName());
                player->loseMark("@wu");
                player->gainMark("@wen");
                room->detachSkillFromPlayer(player, "jiang");
                room->detachSkillFromPlayer(player, "qianxun");
                room->acquireSkill(player, "yingzi");
                room->acquireSkill(player, "keji");
            }
        }
        else{
            if((lvmeng && lvmeng->getMark("@wen") > 0) && !lvmeng->isNude() && lvmeng->askForSkillInvoke(objectName())){
                room->askForDiscard(lvmeng, "mouduan", 1, 1, false, true);
                if(lvmeng->getHandcardNum() > 2)
                {
                    room->broadcastSkillInvoke(objectName());
                    lvmeng->loseMark("@wen");
                    lvmeng->gainMark("@wu");
                    room->detachSkillFromPlayer(lvmeng, "yingzi");
                    room->detachSkillFromPlayer(lvmeng, "keji");
                    room->acquireSkill(lvmeng, "jiang");
                    room->acquireSkill(lvmeng, "qianxun");
                }
            }
        }
        return false;
    }
};

class MouduanClear: public TriggerSkill{
public:
    MouduanClear():TriggerSkill("#mouduan-clear"){
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("mouduan");
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (data.toString() == "mouduan") {
            if (player->getMark("@wu") > 0) {
                player->loseMark("@wu");
                room->detachSkillFromPlayer(player, "jiang");
                room->detachSkillFromPlayer(player, "qianxun");
            }
            else if (player->getMark("@wen") > 0) {
                player->loseMark("@wen");
                room->detachSkillFromPlayer(player, "yingzi");
                room->detachSkillFromPlayer(player, "keji");
            }
        }
        return false;
    }
};

class Zhaolie: public DrawCardsSkill{
public:
    Zhaolie():DrawCardsSkill("zhaolie"){
    }

    virtual int getDrawNum(ServerPlayer *liubei, int n) const{
        Room *room = liubei->getRoom();
        QList<ServerPlayer *> targets = room->getOtherPlayers(liubei);
        QList<ServerPlayer *> victims;
        foreach(ServerPlayer *p, targets){
            if(liubei->inMyAttackRange(p)){
                victims << p;
            }
        }
        if(victims.empty())
            return n;
        if(room->askForSkillInvoke(liubei, objectName())){
            room->broadcastSkillInvoke(objectName());
            room->setPlayerFlag(liubei, "Invoked");
            return n-1;
        }else
            return n;
    }
};

class ZhaolieAct: public TriggerSkill{
public:
    ZhaolieAct():TriggerSkill("#zhaolie"){
        events << CardDrawnDone;
    }


    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *liubei, QVariant &data) const{
        int no_basic = 0;
        QList<const Card *> cards;
        QList<ServerPlayer *> targets = room->getOtherPlayers(liubei);
        QList<ServerPlayer *> victims;
        foreach(ServerPlayer *p, targets){
            if(liubei->inMyAttackRange(p)){
                victims << p;
            }
        }
        if(liubei->getPhase() == Player::Draw && liubei->hasFlag("Invoked")){
            room->setPlayerFlag(liubei, "-Invoked");
            ServerPlayer *victim = room->askForPlayerChosen(liubei, victims, "zhaolie");
            QList<int> cardIds = room->drawCards(3);
            Q_ASSERT(cardIds.size() == 3);
            CardsMoveStruct move;
            move.card_ids = cardIds;
            move.to_place = Player::PlaceTable;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, liubei->objectName(), QString(), "zhaolie", QString());
            room->moveCards(move, true);
            room->getThread()->delay();
            for(int i = 0; i < 3; i++)
            {
                int card_id = cardIds[i];
                const Card *card = Sanguosha->getCard(card_id);
                if(!card->isKindOf("BasicCard") || card->isKindOf("Peach"))
                {
                    if(!card->isKindOf("BasicCard")){
                        no_basic++;
                    }
                    CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "zhaolie", QString());
                    room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                }
                else
                {
                    cards << card;
                }
            }
            QStringList choicelist;
            choicelist << "damage";
            if (victim->getCards("he").length() >= no_basic){
                choicelist << "throw";
            }
            QString choice;
            if (choicelist.length() >=2){
                QVariant data = QVariant::fromValue(no_basic);
                choice = room->askForChoice(victim, "zhaolie", choicelist.join("+"), data);
            }
            else{
                choice = "damage";
            }
            if(choice == "damage"){
                if(no_basic > 0){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.from = liubei;
                    damage.to = victim;
                    damage.damage = no_basic;

                    room->damage(damage);
                }
                if(!cards.empty()){
                    foreach(const Card *c, cards){
                        CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, victim->objectName());
                        if(victim->isAlive())
                            room->obtainCard(victim, c, reason, true);
                        else{
                            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, victim->objectName(), "zhaolie", QString());
                            room->throwCard(c, reason, NULL);
                        }
                    }
                }
            }
            else{
                if(no_basic > 0){
                    while(no_basic > 0){
                        room->askForDiscard(victim, "zhaolie", 1, 1, false, true);
                        no_basic--;
                    }
                }
                if(!cards.empty()){
                    foreach(const Card *c, cards){
                        CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, liubei->objectName());
                        room->obtainCard(liubei, c, reason);
                    }
                }
            }
        }
        return false;
    }
};

class Shichou: public TriggerSkill{
public:
    Shichou(): TriggerSkill("shichou$"){
        events << GameStart << EventPhaseStart << DamageInflicted << Dying << DamageComplete;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case GameStart:{
            if(player->hasLordSkill(objectName()))
                room->setPlayerMark(player, "@hate", 1);
            break;
        }
        case EventPhaseStart:{
            if(player->hasLordSkill(objectName()) && player->getPhase() == Player::Start &&
                    player->getMark("shichouInvoke") == 0 && player->getCards("he").length() > 1)
            {
                if(player->getMark("@hate") == 0)
                    room->setPlayerMark(player, "@hate", 1);

                QList<ServerPlayer *> victims;

                foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                    if(p->getKingdom() == "shu"){
                        if(!p->tag.value("ShichouTarget").isNull()
                                && p->tag.value("ShichouTarget").value<PlayerStar>() == player)
                            continue;
                        else
                            victims << p;
                    }
                }
                if(victims.empty())
                    return false;

                if(player->askForSkillInvoke(objectName())){
                    player->loseMark("@hate", 1);
                    room->setPlayerMark(player, "shichouInvoke", 1);
                    room->broadcastSkillInvoke(objectName());
                    ServerPlayer *victim = room->askForPlayerChosen(player, victims, objectName());
                    room->setPlayerMark(victim, "@chou", 1);
                    player->tag["ShichouTarget"] = QVariant::fromValue((PlayerStar)victim);

                    const Card *card = room->askForExchange(player, objectName(), 2, true, "ShichouGive");
                    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, player->objectName());
                    reason.m_playerId = victim->objectName();
                    room->obtainCard(victim, card, reason, false);
                }
            }
            break;
        }
        case DamageInflicted:{
            if(player->hasLordSkill(objectName(), true) && !player->tag.value("ShichouTarget").isNull())
            {
                ServerPlayer *target = player->tag.value("ShichouTarget").value<PlayerStar>();

                LogMessage log;
                log.type = "#ShichouProtect";
                log.arg = objectName();
                log.from = player;
                log.to << target;
                room->sendLog(log);
                room->setPlayerFlag(target, "Shichou");

                if(player != target)
                {
                    DamageStruct damage = data.value<DamageStruct>();
                    damage.to = target;
                    damage.transfer = true;
                    room->damage(damage);
                    return true;
                }
            }
            break;
        }
        case DamageComplete:{
            if(player->hasFlag("Shichou")){
                DamageStruct damage = data.value<DamageStruct>();
                player->drawCards(damage.damage);
                room->setPlayerFlag(player, "-Shichou");
            }
            break;
        }
        case Dying:{
            if(player->getMark("@chou") > 0)
            {
                player->loseMark("@chou");
                foreach(ServerPlayer *lord, room->getAlivePlayers())
                {
                    if(lord->hasLordSkill(objectName(), true)
                            && lord->tag.value("ShichouTarget").value<PlayerStar>() == player)
                        lord->tag.remove("ShichouTarget");
                }
            }
            break;
        }
        default:
            break;
        }
        return false;
    }
};

YanxiaoCard::YanxiaoCard(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("YanxiaoCard");
}

bool YanxiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    return true;
}

void YanxiaoCard::takeEffect(ServerPlayer *) const{
}

class YanxiaoVeiwAsSkill: public OneCardViewAsSkill{
public:
    YanxiaoVeiwAsSkill():OneCardViewAsSkill("yanxiao"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        YanxiaoCard *yanxiao = new YanxiaoCard(originalCard->getSuit(), originalCard->getNumber());
        yanxiao->addSubcard(originalCard->getId());
        yanxiao->setSkillName(objectName());
        return yanxiao;
    }
};


class Yanxiao: public PhaseChangeSkill{
public:
    Yanxiao():PhaseChangeSkill("yanxiao"){
        view_as_skill = new YanxiaoVeiwAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getPhase() == Player::Judge && target->containsTrick("YanxiaoCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        CardsMoveStruct move;
        foreach(const Card* deleyed_trick, target->getJudgingArea())
            move.card_ids << deleyed_trick->getId();
        move.to = target;
        move.to_place = Player::PlaceHand;
        target->getRoom()->moveCardsAtomic(move, true);
        return false;
    }
};

class Anxian: public TriggerSkill{
public:
    Anxian():TriggerSkill("anxian"){
        events << DamageCaused << TargetConfirming << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *daqiao, QVariant &data) const{
        if(event == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();

            if(damage.card && damage.card->isKindOf("Slash") &&
               !damage.chain && !damage.transfer && !damage.to->isKongcheng()
                && daqiao->askForSkillInvoke(objectName(), data)){

                LogMessage log;
                log.type = "#Anxian";
                log.from = daqiao;
                log.arg = objectName();
                room->sendLog(log);
                room->askForDiscard(damage.to, "anxian", 1, 1);
                daqiao->drawCards(1);
                return true;
            }
        }
        else if(event == TargetConfirming){

            CardUseStruct use = data.value<CardUseStruct>();

            if(use.card && use.card->isKindOf("Slash")){
                if(room->askForCard(daqiao, ".", "@anxian-discard", QVariant(), CardDiscarded)){
                    daqiao->addMark("anxian");
                    use.from->drawCards(1);
                    LogMessage log;
                    log.type = "#AnxianAvoid";
                    log.from = use.from;
                    log.to << daqiao;
                    log.arg = objectName();
                    room->sendLog(log);
                }

            }
        }
        else {
            if(daqiao->getMark("anxian") > 0){
                daqiao->setMark("anxian", daqiao->getMark("anxian")-1);
                return true;
            }
        }
        return false;
    }
};

YinlingCard::YinlingCard()
{
}

bool YinlingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void YinlingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (effect.to->isNude() || effect.from->getPile("brocade").length() >= 4)
        return;
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "yinling");
    effect.from->addToPile("brocade", card_id);
}

class Yinling: public OneCardViewAsSkill{
public:
    Yinling():OneCardViewAsSkill("yinling"){
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("brocade").length() < 4;
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        YinlingCard *card = new YinlingCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class YinlingClear: public TriggerSkill{
public:
    YinlingClear():TriggerSkill("#yinling-clear"){
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* , ServerPlayer *player, QVariant &data) const{
        if (data.toString() != "yinling")
            return false;
        player->removePileByName("brocade");
        return false;
    }
};

class JunweiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->isKindOf("Jink");
    }

    virtual bool willThrow() const{
        return false;
    }
};

class Junwei:public TriggerSkill{
public:
    Junwei():TriggerSkill("junwei") {
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *ganning, QVariant &) const{
        if (ganning->getPhase() == Player::Finish && ganning->getPile("brocade").length() >= 3 && ganning->askForSkillInvoke(objectName())) {
            QList<int> brocade = ganning->getPile("brocade");
            room->broadcastSkillInvoke(objectName());

            int ai_delay = Config.AIDelay;
            Config.AIDelay = 0;

            for(int i = 0; i < 3; i++) {
                int card_id = 0;
                room->fillAG(brocade, ganning);
                if (brocade.length() == 3 - i)
                    card_id = brocade.first();
                else
                    card_id = room->askForAG(ganning, brocade, false, objectName());
                ganning->invoke("clearAG");

                brocade.removeOne(card_id);

                CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }

            Config.AIDelay = ai_delay;

            ServerPlayer *target = room->askForPlayerChosen(ganning, room->getAllPlayers(), objectName());
            QVariant ai_data = QVariant::fromValue((PlayerStar)ganning);
            const Card *card = room->askForCard(target, ".junwei", "@junwei-show", ai_data, NonTrigger);
            if (card) {
                room->showCard(target, card->getEffectiveId());
                ServerPlayer *receiver = room->askForPlayerChosen(ganning, room->getAllPlayers(), "junweigive");
                if (receiver != target)
                    receiver->obtainCard(card);
            } else {
                room->loseHp(target, 1);
                if (!target->isAlive()) return false;
                if (target->hasEquip()) {
                    int card_id = room->askForCardChosen(ganning, target, "e", objectName());
                    target->addToPile("junwei-equip", card_id);
                }
            }
        }
        return false;
    }
};

class JunweiGot: public TriggerSkill {
public:
    JunweiGot(): TriggerSkill("#junwei-got") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive || player->getPile("junwei-equip").length() == 0)
            return false;
        foreach(int card_id, player->getPile("junwei-equip")){
            const Card *card = Sanguosha->getCard(card_id);

            int equip_index = -1;
            const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
            equip_index = static_cast<int>(equip->location());

            QList<CardsMoveStruct> exchangeMove;
            CardsMoveStruct move1;
            move1.card_ids << card_id;
            move1.to = player;
            move1.to_place = Player::PlaceEquip;
            move1.reason = CardMoveReason(CardMoveReason::S_REASON_EXCHANGE_FROM_PILE, player->objectName());
            exchangeMove.push_back(move1);
            if(player->getEquip(equip_index) != NULL)
            {
                CardsMoveStruct move2;
                move2.card_ids << player->getEquip(equip_index)->getId();
                move2.to = NULL;
                move2.to_place = Player::DiscardPile;
                move2.reason = CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, player->objectName());
                exchangeMove.push_back(move2);
            }
            LogMessage log;
            log.from = player;
            log.type = "$JunweiGot";
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->moveCardsAtomic(exchangeMove, true);
        }
        return false;
    }
};

class Fenyong: public TriggerSkill{
public:
    Fenyong(): TriggerSkill("fenyong") {
        events << Damaged << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const {
        if (event == Damaged) {
            if (player->getMark("@fenyong") == 0 && room->askForSkillInvoke(player, objectName()))
                player->gainMark("@fenyong");
        } else if (event == DamageInflicted) {
            if (player->getMark("@fenyong") > 0) {
                LogMessage log;
                log.type = "#FenyongAvoid";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class FenyongClear: public TriggerSkill{
public:
    FenyongClear():TriggerSkill("#fenyong-clear"){
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("fenyong") && target->getMark("@fenyong") > 0;
    }

    virtual bool trigger(TriggerEvent, Room* , ServerPlayer *player, QVariant &) const{
        player->loseAllMarks("@fenyong");
        return false;
    }
};

class Xuehen: public TriggerSkill{
public:
    Xuehen(): TriggerSkill("xuehen") {
        events << EventPhaseStart;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        ServerPlayer *xiahou = room->findPlayerBySkillName(objectName());
        if(xiahou == NULL)
            return false;
        if(player->getPhase() == Player::Finish && xiahou->getMark("@fenyong") > 0){
            LogMessage log;
            log.from = xiahou;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);

            xiahou->loseMark("@fenyong");
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(xiahou))
                if (xiahou->canSlash(p, NULL, false))
                    targets << p;
            QString choice;
            if (targets.isEmpty())
                choice = "discard";
            else
                choice = room->askForChoice(xiahou, objectName(), "discard+slash");
            if (choice == "slash") {
                ServerPlayer *victim = room->askForPlayerChosen(xiahou, targets, objectName());

                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                CardUseStruct card_use;
                card_use.from = xiahou;
                card_use.to << victim;
                card_use.card = slash;
                room->useCard(card_use, false);
            } else {
                room->setPlayerFlag(player, "XuehenTarget_InTempMoving");
                DummyCard *dummy = new DummyCard;
                QList<int> card_ids;
                QList<Player::Place> original_places;
                for (int i = 0; i < xiahou->getLostHp(); i++) {
                    if (player->isNude())
                        break;
                    card_ids << room->askForCardChosen(xiahou, player, "he", objectName());
                    original_places << room->getCardPlace(card_ids[i]);
                    dummy->addSubcard(card_ids[i]);
                    player->addToPile("#xuehen", card_ids[i], false);
                }
                if (dummy->subcardsLength() == 0)
                    return false;
                else
                    for (int i = 0; i < dummy->subcardsLength(); i++)
                        room->moveCardTo(Sanguosha->getCard(card_ids[i]), player, original_places[i], false);
                room->setPlayerFlag(player, "-XuehenTarget_InTempMoving");
                room->throwCard(dummy, player, xiahou);
                dummy->deleteLater();
            }
        }
        return false;
    }
};

class XuehenAvoidTriggeringCardsMove: public TriggerSkill{
public:
    XuehenAvoidTriggeringCardsMove():TriggerSkill("#xuehen-avoid-triggering-cards-move"){
        events << CardsMoving << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &) const{
        foreach(ServerPlayer *p, room->getAllPlayers())
            if (p->hasFlag("XuehenTarget_InTempMoving"))
                return true;
        return false;
    }
};

BGMPackage::BGMPackage():Package("BGM"){
    General *bgm_zhaoyun = new General(this, "bgm_zhaoyun", "qun", 3);
    bgm_zhaoyun->addSkill("longdan");
    bgm_zhaoyun->addSkill(new ChongZhen);

    General *bgm_diaochan = new General(this, "bgm_diaochan", "qun", 3, false);
    bgm_diaochan->addSkill(new Lihun);
    bgm_diaochan->addSkill("biyue");

    General *bgm_caoren = new General(this, "bgm_caoren", "wei");
    bgm_caoren->addSkill(new Kuiwei);
    bgm_caoren->addSkill(new Yanzheng);

    General *bgm_pangtong = new General(this, "bgm_pangtong", "qun", 3);
    bgm_pangtong->addSkill(new Manjuan);
    bgm_pangtong->addSkill(new Zuixiang);
    bgm_pangtong->addSkill(new MarkAssignSkill("@sleep", 1));

    General *bgm_zhangfei = new General(this, "bgm_zhangfei", "shu");
    bgm_zhangfei->addSkill(new Jie);
    bgm_zhangfei->addSkill(new Dahe);
    bgm_zhangfei->addSkill(new DahePindian);
    related_skills.insertMulti("dahe", "#dahe");

    General *bgm_lvmeng = new General(this, "bgm_lvmeng", "wu", 3);
    bgm_lvmeng->addSkill(new Tanhu);
    bgm_lvmeng->addSkill(new MouduanStart);
    bgm_lvmeng->addSkill(new Mouduan);
    bgm_lvmeng->addSkill(new MouduanClear);
    related_skills.insertMulti("mouduan", "#mouduan");
    related_skills.insertMulti("mouduan", "#mouduan-clear");

    General *bgm_liubei = new General(this, "bgm_liubei$", "shu");
    bgm_liubei->addSkill(new Zhaolie);
    bgm_liubei->addSkill(new ZhaolieAct);
    bgm_liubei->addSkill(new Shichou);
    related_skills.insertMulti("zhaolie", "#zhaolie");

    General *bgm_daqiao = new General(this, "bgm_daqiao", "wu", 3, false);
    bgm_daqiao->addSkill(new Yanxiao);
    bgm_daqiao->addSkill(new Anxian);

    General *bgm_ganning = new General(this, "bgm_ganning", "qun");
    bgm_ganning->addSkill(new Yinling);
    bgm_ganning->addSkill(new YinlingClear);
    bgm_ganning->addSkill(new Junwei);
    bgm_ganning->addSkill(new JunweiGot);
    patterns.insert(".junwei", new JunweiPattern);
    related_skills.insertMulti("yinling", "#yinling-clear");
    related_skills.insertMulti("junwei", "#junwei-got");

    General *bgm_xiahoudun = new General(this, "bgm_xiahoudun", "wei");
    bgm_xiahoudun->addSkill(new Fenyong);
    bgm_xiahoudun->addSkill(new FenyongClear);
    bgm_xiahoudun->addSkill(new Xuehen);
    bgm_xiahoudun->addSkill(new XuehenAvoidTriggeringCardsMove);
    related_skills.insertMulti("fenyong", "#fenyong-clear");
    related_skills.insertMulti("xuehen", "#xuehen-avoid-triggering-cards-move");

    addMetaObject<LihunCard>();
    addMetaObject<DaheCard>();
    addMetaObject<TanhuCard>();
    addMetaObject<YanxiaoCard>();
    addMetaObject<YinlingCard>();
}

ADD_PACKAGE(BGM)
