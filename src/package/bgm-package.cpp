#include "bgm-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class ChongZhen: public TriggerSkill{
public:
    ChongZhen(): TriggerSkill("chongzhen"){
        events << CardResponsed << SlashEffect << CardEffect << CardEffected << CardFinished;
    }

    virtual int getPriority() const{
        return 3;
    }

    void doChongZhen(ServerPlayer *player, const Card *card) const{
        if(card->getSkillName() != "longdan")
            return;

        Room *room = player->getRoom();

        ServerPlayer *target = player->tag["ChongZhenTarget"].value<PlayerStar>();
        if(!target || target->isKongcheng() || !room->askForSkillInvoke(player, objectName()))
            return;

        int card_id = room->askForCardChosen(player, target, "h", objectName());
        room->obtainCard(player, card_id, false);
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == CardFinished){
            player->tag["ChongZhenTarget"] = QVariant::fromValue(NULL);
        }
        else if(event == CardResponsed){
            CardStar card = data.value<CardStar>();
            doChongZhen(player, card);
        }
        else if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            player->tag["ChongZhenTarget"] = QVariant::fromValue(effect.to);
            doChongZhen(player, effect.slash);
        }
        else{
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Duel")
                    || effect.card->inherits("ArcheryAttack")
                    || effect.card->inherits("SavageAssault")
                    || effect.card->inherits("Slash"))
                player->tag["ChongZhenTarget"] = QVariant::fromValue(event == CardEffect ? effect.to : effect.from);
        }

        return false;
    }
};

LihunCard::LihunCard(){
    owner_discarded = true;
}

bool LihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!to_select->getGeneral()->isMale())
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
        CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, effect.from->objectName(),
            effect.to->objectName(), "lihun", QString());
        room->moveCardTo(dummy_card, effect.from, Player::Hand, reason, false);
    }
    effect.to->setFlags("LihunTarget");
}

class LihunSelect: public OneCardViewAsSkill{
public:
    LihunSelect():OneCardViewAsSkill("lihun"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LihunCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new LihunCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Lihun: public TriggerSkill{
public:
    Lihun():TriggerSkill("lihun"){
        events << PhaseChange;
        view_as_skill = new LihunSelect;
    }

    virtual int getPriority() const{
        return 4;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasUsed("LihunCard");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *diaochan, QVariant &data) const{
        PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();

        if(phase_change.from == Player::Play){
            ServerPlayer *target = NULL;
            foreach(ServerPlayer *other, room->getOtherPlayers(diaochan)){
                if(other->hasFlag("LihunTarget")){
                    other->setFlags("-LihunTarget");
                    target = other;
                    break;
                }
            }

            if(!target || target->isDead())
                return false;

            int hp = target->isAlive() ? target->getHp() : 0;
            if(diaochan->getCards("he").length() <= hp){
                foreach(const Card *card, diaochan->getCards("he"))
                    room->obtainCard(target, card, room->getCardPlace(card->getEffectiveId()) != Player::Hand);
            }
            else{
                int i;
                for(i = 0; i < hp; i++){
                    if(diaochan->isNude())
                        return false;

                    int card_id = room->askForCardChosen(diaochan, diaochan, "he", objectName());
                    room->obtainCard(target, card_id, room->getCardPlace(card_id) != Player::Hand);
                }
            }
            room->removeTag("LihunTarget");
        }

        return false;
    }
};

class Kuiwei: public TriggerSkill{
public:
    Kuiwei(): TriggerSkill("kuiwei"){
        events << PhaseChange;
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
        if(caoren->getPhase() == Player::Finish){
            if(!caoren->askForSkillInvoke(objectName()))
                return false;

            int n = getWeaponCount(caoren);
            caoren->drawCards(n+2);
            caoren->turnOver();

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

                if(caoren->getCards("he").length() <= n){
                    caoren->throwAllEquips();
                    caoren->throwAllHandCards();
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

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName(objectName());

        return ncard;
    }
};

class Manjuan: public TriggerSkill{
public:
    Manjuan(): TriggerSkill("manjuan"){
        events << CardGotOnePiece << CardDrawing;
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
            room->moveCardTo(Sanguosha->getCard(id), sp_pangtong, Player::Hand, true);

        sp_pangtong->invoke("clearAG");
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *sp_pangtong, QVariant &data) const{
        if(sp_pangtong->hasFlag("ManjuanInvoke")){
            sp_pangtong->setFlags("-ManjuanInvoke");
            return false;
        }

        int card_id = -1;
        CardMoveReason reason(CardMoveReason::S_REASON_PUT, sp_pangtong->objectName(), "manjuan", QString());
        if(event == CardGotOnePiece){
            CardMoveStar move = data.value<CardMoveStar>();
            card_id = move->card_id;
            if(move->to_place == Player::Hand){
                const Card* card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, NULL, Player::DiscardPile, reason);
            }else
                return false;
        }
        else if(event == CardDrawing){
            if(room->getTag("FirstRound").toBool())
                return false;
            card_id = data.toInt();
            const Card* card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, NULL, Player::DiscardPile, reason);
        }

        LogMessage log;
        log.type = "$ManjuanGot";
        log.from = sp_pangtong;
        log.card_str = Sanguosha->getCard(card_id)->toString();
        room->sendLog(log);

        if(sp_pangtong->getPhase() == Player::NotActive || !sp_pangtong->askForSkillInvoke(objectName(), data))
            return event == CardGotOnePiece ? false : true;

        doManjuan(sp_pangtong, card_id);
        return event == CardGotOnePiece ? false : true;
    }
};

class Zuixiang: public TriggerSkill{
public:
    Zuixiang(): TriggerSkill("zuixiang"){
        events << PhaseChange << CardEffected ;
        frequency = Limited;

        type[Card::Basic] = "BasicCard";
        type[Card::Trick] = "TrickCard";
        type[Card::Equip] = "EquipCard";
    }

    void doZuixiang(ServerPlayer *player) const{
        Room *room = player->getRoom();

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
            CardsMoveStruct move(zuixiang, player, Player::Hand, reason);
            room->moveCards(move, true);
        }
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *sp_pangtong, QVariant &data) const{
        QList<int> zuixiang = sp_pangtong->getPile("dream");

        if(event == PhaseChange && sp_pangtong->getMark("zuixiangHasTrigger") == 0){
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
        else if(event == CardEffected){
            if(zuixiang.isEmpty())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(sp_pangtong->hasCardLock(type[effect.card->getTypeId()])){
                LogMessage log;
                log.type = effect.from ? "#ZuiXiang1" : "#Zuixiang2";
                log.from = effect.to;
                if(effect.from)
                    log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();

                room->sendLog(log);

                room->playSkillEffect(objectName());
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
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->inherits("Slash") || !damage.card->isRed())
            return false;

        LogMessage log;
        log.type = "#Jie";
        log.from = player;
        log.to << damage.to;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(damage.damage + 1);
        room->sendLog(log);
        damage.damage ++;
        data = QVariant::fromValue(damage);

        return false;
    }
};

DaheCard::DaheCard(){
    once = true;
    will_throw = false;
}

bool DaheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self && targets.isEmpty() && !to_select->isKongcheng();
}

void DaheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QString reason = "dahe";
    ServerPlayer *target = targets.first();
    LogMessage log;
    log.type = "#Pindian";
    log.from = source;
    log.to << target;
    room->sendLog(log);

    const Card *card1 = room->askForPindian(source, source, target, reason);
    const Card *card2 = room->askForPindian(target, source, target, reason);

    PindianStruct pindian_struct;
    pindian_struct.from = source;
    pindian_struct.to = target;
    pindian_struct.from_card = card1;
    pindian_struct.to_card = card2;
    pindian_struct.reason = reason;

    PindianStar pindian_star = &pindian_struct;
    QVariant data = QVariant::fromValue(pindian_star);
    room->getThread()->trigger(Pindian, room, source, data);

    bool success = pindian_star->from_card->getNumber() > pindian_star->to_card->getNumber();
    log.type = success ? "#PindianSuccess" : "#PindianFailure";
    log.from = source;
    log.to << target;
    room->sendLog(log);

    if(success){
        room->setEmotion(source, "success");
        room->setPlayerFlag(target, reason);
        QList<ServerPlayer *> to_givelist = room->getAlivePlayers();
        foreach(ServerPlayer *p, targets){
            if(p->getHp() > source->getHp())
                to_givelist.removeOne(p);
        }
        QString choice = room->askForChoice(source, reason, "yes+no");
        if(!to_givelist.isEmpty() && choice == "yes"){
            ServerPlayer *to_give = room->askForPlayerChosen(source, to_givelist, reason);
            to_give->obtainCard(card2);
        }
    }else{
        room->setEmotion(source, "no-success");
        if(!source->isKongcheng()){
            room->showAllCards(source);
            room->askForDiscard(source, reason, 1, 1, false, false);
        }
    }
}

class DaheViewAsSkill: public ZeroCardViewAsSkill{
public:
    DaheViewAsSkill():ZeroCardViewAsSkill("dahe"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaheCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new DaheCard;
    }

};

class Dahe: public TriggerSkill{
public:
    Dahe():TriggerSkill("dahe"){
        events << SlashProceed << PhaseChange;
        view_as_skill = new DaheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *bgm_zhangfei = room->findPlayerBySkillName(objectName());
        if(!bgm_zhangfei || bgm_zhangfei->loseTriggerSkills())
            return false;
        if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!effect.to->hasFlag(objectName()))
                return false;
            const Card *jink = room->askForCard(effect.to, "jink",
                                                QString("@dahe-jink:%1:%2:%3")
                                                .arg(effect.from->objectName())
                                                .arg(bgm_zhangfei->objectName())
                                                .arg(objectName()),
                                                data);
            if(jink && jink->getSuit() != Card::Heart){
                LogMessage log;
                log.type = "#DaheEffect";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = jink->getSuitString();
                log.arg2 = objectName();
                room->sendLog(log);

                room->slashResult(effect, NULL);
            }
            room->slashResult(effect, jink);

            return true;
        }
        else if(event == PhaseChange && bgm_zhangfei->getPhase() == Player::NotActive){
            foreach(ServerPlayer *other, room->getOtherPlayers(player))
                if(other->hasFlag(objectName()))
                    room->setPlayerFlag(other, "-" + objectName());
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

void TanhuCard::use(Room *room, ServerPlayer *lvmeng, const QList<ServerPlayer *> &targets) const{
    bool success = lvmeng->pindian(targets.first(), "tanhu", this);
    if(success){
        room->setPlayerFlag(targets.first(), "TanhuTarget");
        room->setFixedDistance(lvmeng, targets.first(), 1);
    }
}

class TanhuViewAsSkill: public OneCardViewAsSkill{
public:
    TanhuViewAsSkill():OneCardViewAsSkill("tanhu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TanhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new TanhuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
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
        events << TurnStart << CardLostOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *lvmeng = room->findPlayerBySkillName(objectName());
        if(lvmeng && lvmeng->loseTriggerSkills())
            return false;
        if(event == CardLostOneTime){
            if((player->getMark("@wu") > 0) && player->getHandcardNum() <= 2){
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

    General *bgm_lvmeng = new General(this, "bgm_lvmeng", "wu", 3);
    bgm_lvmeng->addSkill(new Tanhu);
    bgm_lvmeng->addSkill(new MouduanStart);
    bgm_lvmeng->addSkill(new Mouduan);
    related_skills.insertMulti("mouduan", "#mouduan");

    addMetaObject<LihunCard>();
    addMetaObject<DaheCard>();
    addMetaObject<TanhuCard>();
}

ADD_PACKAGE(BGM)
