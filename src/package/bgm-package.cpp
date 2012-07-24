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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == CardResponsed){
            CardStar card = data.value<CardStar>();
            if(card->getSkillName() == "longdan"){
                ServerPlayer *target = NULL;
                foreach(ServerPlayer* p, room->getOtherPlayers(player)){
                    if(p->hasFlag("Chongzhen")){
                        room->setPlayerFlag(p, "-Chongzhen");
                        if(!p->isKongcheng()){
                            target = p;
                            break;
                        }
                    }
                }
                QVariant data = QVariant::fromValue(target);
                if(target && player->askForSkillInvoke(objectName(), data)){
                    int card_id = room->askForCardChosen(player, target, "h", objectName());
        			room->obtainCard(player, card_id, false);
                    if(card->inherits("Jink"))
                        room->playSkillEffect("chongzhen", 1);
                    else
                        room->playSkillEffect("chongzhen", 2);
                }
            }
        }
        else{
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.from->objectName() == player->objectName() && use.card->getSkillName() == "longdan"){
                foreach(ServerPlayer *p, use.to){
                    QVariant data = QVariant::fromValue(p);
                    if(p->isKongcheng()) continue;
                    if(player->askForSkillInvoke(objectName(), data)){
                        if(use.card->inherits("Jink"))
                            room->playSkillEffect("chongzhen", 1);
                        else
                            room->playSkillEffect("chongzhen", 2);
                        int card_id = room->askForCardChosen(player, p, "h", objectName());
                                    room->obtainCard(player, card_id, false);
                    }
                }
            }
            else if(use.to.contains(player) && !use.card->inherits("Collateral")
                    && use.card->getSkillName() != "luanwu"
                    && use.card->getSkillName() != "tiaoxin"){
                room->setPlayerFlag(use.from, "Chongzhen");
            }
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
        if(effect.to->getGeneralName().contains("lvbu"))
            room->playSkillEffect("lihun", 2);
        else
            room->playSkillEffect("lihun", 1);
        room->moveCardTo(dummy_card, effect.from, Player::Hand, false);
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
        return target != NULL && target->hasUsed("LihunCard");
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
                foreach(const Card *card, diaochan->getCards("he")){
                    room->obtainCard(target, card, room->getCardPlace(card->getEffectiveId()) != Player::Hand);
                }
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
			room->playSkillEffect("kuiwei", 1);
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
				room->playSkillEffect("kuiwei", 2);
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
        events << CardGot << CardDrawing;
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
        if(event == CardGot){
            CardMoveStar move = data.value<CardMoveStar>();
            card_id = move->card_id;
            if(move->to_place == Player::Hand){
                room->throwCard(card_id);
            }else
                return false;
        }
        else if(event == CardDrawing){
            if(room->getTag("FirstRound").toBool())
                return false;

            card_id = data.toInt();
            room->throwCard(card_id);
        }

        LogMessage log;
        log.type = "$ManjuanGot";
        log.from = sp_pangtong;
        log.card_str = Sanguosha->getCard(card_id)->toString();
        room->sendLog(log);

        if(sp_pangtong->getPhase() == Player::NotActive || !sp_pangtong->askForSkillInvoke(objectName(), data))
            return event == CardGot ? false : true;

        doManjuan(sp_pangtong, card_id);
        return event == CardGot ? false : true;
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

    void doZuixiang(Room *room, ServerPlayer *player) const{
        QList<int> ids = room->getNCards(3);
        foreach(int id, ids){
            const Card *cd = Sanguosha->getCard(id);
            room->moveCardTo(cd, NULL, Player::Special, true);
            room->getThread()->delay();
            player->addToPile("dream", id, true);
            room->setPlayerCardLock(player, type[cd->getTypeId()]);
        }

        QList<int> zuixiang = player->getPile("dream");
        QSet<int> numbers;
        foreach(int id, zuixiang){
            const Card *card = Sanguosha->getCard(id);
            if(numbers.contains(card->getNumber())){
                foreach(int id, zuixiang){
                    const Card *card = Sanguosha->getCard(id);
                    room->moveCardTo(card, player, Player::Hand, true);
                    player->addMark("zuixiangHasTrigger");
                    room->setPlayerCardLock(player, ".");
                }
                return;
            }

            numbers.insert(card->getNumber());
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
                    doZuixiang(room, sp_pangtong);
                }else
                    doZuixiang(room, sp_pangtong);
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
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void DaheCard::use(Room *room, ServerPlayer *zhangfei, const QList<ServerPlayer *> &targets) const{
    zhangfei->pindian(targets.first(), "dahe", this);
}

class DaheViewAsSkill: public OneCardViewAsSkill{
public:
    DaheViewAsSkill():OneCardViewAsSkill("dahe"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaheCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DaheCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Dahe: public TriggerSkill{
public:
    Dahe():TriggerSkill("dahe"){
        events << SlashProceed << PhaseChange;
        view_as_skill = new DaheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *bgm_zhangfei = room->findPlayerBySkillName(objectName());
        if(!bgm_zhangfei)
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
                                                data, CardUsed);
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
        events << TurnStart << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *lvmeng = room->findPlayerBySkillName(objectName());

        if(event == CardLost){
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

class Zhaolie:public DrawCardsSkill{
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
            room->playSkillEffect(objectName());
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
            for(int i = 0; i < 3; i++){
                int card_id = room->drawCard();
                room->moveCardTo(Sanguosha->getCard(card_id), liubei, Player::Special, true);
                room->getThread()->delay();

                const Card *card = Sanguosha->getCard(card_id);
                if(!card->inherits("BasicCard") || card->inherits("Peach")){
                    if(!card->inherits("BasicCard")){
                        no_basic++;
                    }
                    room->throwCard(card_id);
                }else{
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
                        if(victim->isAlive())
                            room->obtainCard(victim, c->getEffectiveId(), true);
                        else
                            room->throwCard(c->getEffectiveId());
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
                        room->obtainCard(liubei, c->getEffectiveId(), true);
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
        events << PhaseChange << DamageInflicted << Dying;
        frequency = Limited;

    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange && player->getMark("hate") < 1 && player->hasLordSkill(objectName())
            && player->getPhase() == Player::Start && player->getCards("he").length() > 1){
            QList<ServerPlayer *> targets = room->getOtherPlayers(player);
            QList<ServerPlayer *> victims;

            foreach(ServerPlayer *p, targets){
                if(p->getKingdom() == "shu" && !p->hasLordSkill(objectName())){
                    victims << p;
                }
            }
            if(victims.empty())
                return false;
            if(player->askForSkillInvoke(objectName())){
                player->addMark("hate");
                ServerPlayer *victim = room->askForPlayerChosen(player, victims, objectName());
                victim->addMark("hate"+player->objectName());
                victim->gainMark("@hate");
                int first_id = room->askForCardChosen(player, player, "he", objectName());
                DummyCard *dummy = new DummyCard;
                dummy->addSubcard(first_id);
                player->addToPile("#shichou", dummy, room->getCardPlace(first_id) == Player::Equip);
                int second_id = room->askForCardChosen(player, player, "he", objectName());
                dummy->addSubcard(second_id);
                room->moveCardTo(dummy, victim, Player::Hand, false);
                delete dummy;
            }
        }
        else if(event == DamageInflicted && player->hasLordSkill(objectName())){
            ServerPlayer *target = NULL;
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if(p->getMark("hate"+player->objectName()) > 0){
                    target = p;
                    break;
                }
            }
            if(target == NULL)
                return false;
            LogMessage log;
            log.type = "#ShichouProtect";
            log.arg = objectName();
            log.from = player;
            log.to << target;
            room->sendLog(log);
            DamageStruct damage = data.value<DamageStruct>();
            DamageStruct newdamage;
            newdamage.card = damage.card;
            newdamage.from = damage.from;
            newdamage.to = target;
            newdamage.damage = damage.damage;
            newdamage.transfer = true;

            room->damage(newdamage);
            if(target->isAlive() && target->getMark("hate"+player->objectName()) > 0)
                target->drawCards(damage.damage);
            return true;
        }
        else if(event == Dying){
            foreach(ServerPlayer *p, room->getAllPlayers()){
                if(p->hasLordSkill(objectName()) && player->getMark("hate"+p->objectName()) > 0){
                    player->setMark("hate"+p->objectName(), 0);
                    player->loseMark("@hate");
                }
            }
        }
        return false;
    }
};

YanxiaoCard::YanxiaoCard(){
}

bool YanxiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getPile("smile").empty();
}

void YanxiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.value(0, source);
    target->addToPile("smile", this->getEffectiveId());
}

class YanxiaoViewAsSkill: public OneCardViewAsSkill{
public:
    YanxiaoViewAsSkill():OneCardViewAsSkill("yanxiao"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new YanxiaoCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Yanxiao: public TriggerSkill{
public:
    Yanxiao():TriggerSkill("yanxiao"){
        events << PhaseChange;
        view_as_skill = new YanxiaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Judge;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        QList<const DelayedTrick *> tricks = player->delayedTricks();
        if(player->getPile("smile").length() > 0){
            while(!tricks.isEmpty() && player->isAlive()){
                const DelayedTrick *trick = tricks.takeLast();
                player->obtainCard(trick);
            }
            QList<int> yanxiao(player->getPile("smile"));
            foreach(int card_id, yanxiao){
                player->obtainCard(Sanguosha->getCard(card_id));
            }
            yanxiao.clear();
        }
        return false;
    }
};


class Anxian: public TriggerSkill{
public:
    Anxian():TriggerSkill("anxian"){
        events << DamageCaused << TargetConfirmed << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *daqiao, QVariant &data) const{
        if(event == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();

            if(damage.card && damage.card->inherits("Slash") &&
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
        else if(event == TargetConfirmed){

            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.to.contains(daqiao) || !daqiao->hasSkill(objectName()) || daqiao->isKongcheng())
                return false;
            if(use.card && use.card->inherits("Slash")){
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
    related_skills.insertMulti("mouduan", "#mouduan");

    General *bgm_liubei = new General(this, "bgm_liubei$", "shu");
    bgm_liubei->addSkill(new Zhaolie);
    bgm_liubei->addSkill(new ZhaolieAct);
    bgm_liubei->addSkill(new Shichou);
    related_skills.insertMulti("zhaolie", "#zhaolie");

    General *bgm_daqiao = new General(this, "bgm_daqiao", "wu", 3, false);
    bgm_daqiao->addSkill(new Yanxiao);
    bgm_daqiao->addSkill(new Anxian);

    addMetaObject<LihunCard>();
    addMetaObject<DaheCard>();
    addMetaObject<TanhuCard>();
    addMetaObject<YanxiaoCard>();
}

ADD_PACKAGE(BGM)
