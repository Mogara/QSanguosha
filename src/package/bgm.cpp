#include "bgm.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "settings.h"

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
        room->playSkillEffect("chongzhen", card->inherits("Jink") ? 1: 2);
    }

    virtual bool trigger(TriggerEvent event, Room*, ServerPlayer *player, QVariant &data) const{
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
        room->moveCardTo(dummy_card, effect.from, Player::Hand, false);
    room->setTag("LihunTarget", QVariant::fromValue(effect.to));
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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *diaochan, QVariant &) const{
        if(event == PhaseChange && diaochan->getPhase() == Player::Discard){
            ServerPlayer *target = room->getTag("LihunTarget").value<PlayerStar>();
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
        events << CardGot << CardDrawing;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return 2;
    }

    void doManjuan(ServerPlayer *sp_pangtong, int card_id) const{
        Room *room = sp_pangtong->getRoom();
        sp_pangtong->setFlags("ManjuanInvoke");
        QList<int> discardedPile = room->getDiscardPile(), toGainList;
        const Card *card = Sanguosha->getCard(card_id);
        foreach(int id, discardedPile){
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

        room->playSkillEffect(objectName());
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

    void doZuixiang(ServerPlayer *player) const{
        Room *room = player->getRoom();

        room->playSkillEffect("zuixiang");
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
        room->playSkillEffect(objectName());
        damage.damage ++;
        data = QVariant::fromValue(damage);

        return false;
    }
};

DaheCard::DaheCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool DaheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void DaheCard::use(Room *room, ServerPlayer *zhangfei, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect(skill_name, 1);
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason != "dahe" || !pindian->from->hasSkill(objectName()))
            return false;

        if(pindian->isSuccess()){
            room->playSkillEffect("dahe", 2);
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
            room->playSkillEffect("dahe", 3);
            if(!pindian->from->isKongcheng()){
                room->showAllCards(pindian->from);
                room->askForDiscard(pindian->from, "dahe", 1, false, false);
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
                room->playSkillEffect(objectName());
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
                    room->playSkillEffect(objectName());
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
                choice = room->askForChoice(victim, "zhaolie", choicelist.join("+"));
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
        events << PhaseChange << Predamaged << Dying;
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
                room->playSkillEffect(objectName());
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
        else if(event == Predamaged && player->hasLordSkill(objectName())){
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
    will_throw = false;
}

bool YanxiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->property("yanxiao").toInt() < 1;
}

void YanxiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.value(0, source);
    room->moveCardTo(this, target, Player::Judging);
    room->setPlayerProperty(target, "yanxiao", QVariant::fromValue(getEffectiveId()));
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

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Judge)
            return false;
        int yanxiao_id = player->property("yanxiao").toInt();
        if(yanxiao_id < 1)
            return false;
        const Card *yanxiao = Sanguosha->getCard(yanxiao_id);
        QList<const Card *> judgis = player->getJudgingArea();
        if(judgis.contains(yanxiao)){
            while(!judgis.isEmpty() && player->isAlive()){
                const Card *trick = judgis.takeLast();
                player->obtainCard(trick);
            }
        }
        room->setPlayerProperty(player, "yanxiao", QVariant::fromValue(-1));
        return false;
    }
};

class Anxian: public TriggerSkill{
public:
    Anxian():TriggerSkill("anxian"){
        events << Predamage << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *daqiao, QVariant &data) const{
        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();

            if(damage.card && damage.card->inherits("Slash") &&
               !damage.chain && !damage.to->isKongcheng()
                && daqiao->askForSkillInvoke(objectName(), data)){

                room->playSkillEffect(objectName(), 1);
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
        else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(!effect.card->inherits("Slash"))
                return false;
            if(daqiao->isKongcheng())
                return false;
            if(room->askForCard(daqiao, ".", "@anxian-discard", QVariant(), CardDiscarded)){
                room->playSkillEffect(objectName(), 2);
                effect.from->drawCards(1);
                LogMessage log;
                log.type = "#AnxianAvoid";
                log.from = effect.from;
                log.to << daqiao;
                log.arg = objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

YinlingCard::YinlingCard(){
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

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("brocade").length() < 4;
    }

    virtual const Card *viewAs(CardItem *originalcard) const{
        YinlingCard *card = new YinlingCard;
        card->addSubcard(originalcard->getFilteredCard());
        return card;
    }
};

class YinlingClear: public TriggerSkill{
public:
    YinlingClear():TriggerSkill("#yinling-clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room*, ServerPlayer *player, QVariant &data) const{
        if (data.toString() != "yinling")
            return false;
        player->clearPile("brocade");
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
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *ganning, QVariant &) const{
        if(ganning->getPhase() == Player::Finish && ganning->getPile("brocade").length() >= 3 && ganning->askForSkillInvoke(objectName())) {
            QList<int> brocade = ganning->getPile("brocade");
            room->playSkillEffect(objectName());

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
                room->throwCard(Sanguosha->getCard(card_id));
            }

            Config.AIDelay = ai_delay;

            ServerPlayer *target = room->askForPlayerChosen(ganning, room->getAllPlayers(), objectName());
            QVariant ai_data = QVariant::fromValue((PlayerStar)ganning);
            const Card *card = room->askForCard(target, ".junwei", "@junwei-show", ai_data, NonTrigger);
            if(card){
                room->showCard(target, card->getEffectiveId());
                ServerPlayer *receiver = room->askForPlayerChosen(ganning, room->getAllPlayers(), "junweigive");
                if(receiver != target)
                    receiver->obtainCard(card);
            }else{
                room->loseHp(target, 1);
                if(!target->isAlive()) return false;
                if(target->hasEquip()) {
                    int card_id = room->askForCardChosen(ganning, target, "e", objectName());
                    target->addToPile("junwei_equip", card_id);
                }
            }
        }
        return false;
    }
};

class JunweiGot: public TriggerSkill {
public:
    JunweiGot(): TriggerSkill("#junwei-got") {
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
        if(player->getPhase() != Player::NotActive || player->getPile("junwei_equip").length() == 0)
            return false;
        foreach(int card_id, player->getPile("junwei_equip")) {
            const Card *card = Sanguosha->getCard(card_id);

            //int equip_index = -1;
            //const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            //equip_index = static_cast<int>(equip->location());
            room->moveCardTo(card, player, Player::Equip);

            LogMessage log;
            log.from = player;
            log.type = "$JunweiGot";
            log.card_str = QString::number(card_id);
            room->sendLog(log);
        }
        return false;
    }
};

class Fenyong: public TriggerSkill{
public:
    Fenyong(): TriggerSkill("fenyong") {
        events << Damaged << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &) const {
        if(event == Damaged) {
            if (player->getMark("@fenyong") == 0 && room->askForSkillInvoke(player, objectName())) {
                player->gainMark("@fenyong");
                room->playSkillEffect(objectName(), 1);
            }
        } else if (event == Predamaged) {
            if (player->getMark("@fenyong") > 0) {
                room->playSkillEffect(objectName(), 2);
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
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("fenyong") && target->getMark("@fenyong") > 0;
    }

    virtual bool trigger(TriggerEvent, Room*, ServerPlayer *player, QVariant &) const{
        player->loseAllMarks("@fenyong");
        return false;
    }
};

class Xuehen: public TriggerSkill{
public:
    Xuehen(): TriggerSkill("xuehen") {
        events << PhaseChange;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const {
        ServerPlayer *xiahou = room->findPlayerBySkillName(objectName());
        if(!xiahou)
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
                if (xiahou->canSlash(p, false))
                    targets << p;
            targets << xiahou;
            QString choice;
            if (targets.isEmpty())
                choice = "discard";
            else
                choice = room->askForChoice(xiahou, objectName(), "discard+slash");
            if (choice == "slash") {
                room->playSkillEffect(objectName(), 2);

                ServerPlayer *victim = room->askForPlayerChosen(xiahou, targets, objectName());

                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                CardUseStruct card_use;
                card_use.from = xiahou;
                card_use.to << victim;
                card_use.card = slash;
                room->useCard(card_use, false);
            } else {
                room->playSkillEffect(objectName(), 1);
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
                for (int i = 0; i < dummy->subcardsLength(); i++)
                    room->moveCardTo(Sanguosha->getCard(card_ids[i]), player, original_places[i], false);
                room->setPlayerFlag(player, "-XuehenTarget_InTempMoving");
                //if (dummy->subcardsLength() > 0)
                    room->throwCard(dummy);
                dummy->deleteLater();
            }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const {
        return -2;
    }
};

class XuehenAvoidTriggeringCardsMove: public TriggerSkill{
public:
    XuehenAvoidTriggeringCardsMove():TriggerSkill("#xuehen-avoid-triggering-cards-move"){
        events << CardGot;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        foreach(ServerPlayer *p, room->getAllPlayers())
            if(p->hasFlag("XuehenTarget_InTempMoving"))
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
    related_skills.insertMulti("mouduan", "#mouduan");

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

//diy paster
class Shude:public PhaseChangeSkill{
public:
    Shude():PhaseChangeSkill("shude"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *wyj) const{
        Room *room = wyj->getRoom();
        if(wyj->getPhase() == Player::Finish){
            int x = wyj->getMaxHp() - wyj->getHandcardNum();
            if(x > 0 && wyj->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                wyj->drawCards(x);
            }
        }
        return false;
    }
};

FuluanCard::FuluanCard(){
}

bool FuluanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->inMyAttackRange(to_select);
}

void FuluanCard::use(Room *room, ServerPlayer *shen, const QList<ServerPlayer *> &targets) const{
    targets.first()->turnOver();
    room->setPlayerFlag(shen, "fuluan");
    room->acquireSkill(shen, "#fuluan_slash");
}

class Fuluan: public ViewAsSkill{
public:
    Fuluan(): ViewAsSkill("fuluan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FuluanCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;

        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getSuit() != item->getFilteredCard()->getSuit())
                return false;
        }
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 3)
            return NULL;
        FuluanCard *card = new FuluanCard;
        card->addSubcards(cards);
        return card;
    }
};

class FuluanSlash: public SlashSkill{
public:
    FuluanSlash():SlashSkill("#fuluan_slash"){
        frequency = NotFrequent;
    }

    virtual int getSlashResidue(const Player *t) const{
        if(t->hasSkill("fuluan") && t->hasFlag("fuluan"))
            return -998;
        else
            return 0;
    }
};

class Zhaoxin:public PhaseChangeSkill{
public:
    Zhaoxin():PhaseChangeSkill("zhaoxin"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Play)
            return false;

        Room *room = player->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(player->canSlash(tmp))
                targets << tmp;
        }
        if(targets.isEmpty() || !player->askForSkillInvoke(objectName()))
            return false;
        room->showAllCards(player);

        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        CardUseStruct card_use;
        card_use.from = player;
        card_use.to << target;
        card_use.card = slash;
        room->useCard(card_use, false);
        return false;
    }
};

class Langgu: public TriggerSkill{
public:
    Langgu():TriggerSkill("langgu"){
        events << Damaged << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(!damage.from || damage.from->isKongcheng())
                return false;

            int x = damage.damage, i;
            for(i=0; i<x; i++){
                if(player->askForSkillInvoke(objectName(),data)){
                    room->playSkillEffect(objectName());
                    JudgeStruct judge;
                    judge.reason = objectName();
                    judge.who = player;

                    room->judge(judge);
                    room->fillAG(damage.from->handCards(), player);
                    QString mark = judge.card->getSuitString();
                    room->setPlayerFlag(player, mark);
                    while(!damage.from->isKongcheng()){
                        int card_id = room->askForAG(player, damage.from->handCards(), true, "langgu");
                        if(card_id < 0)
                            break;
                        const Card *cc = Sanguosha->getCard(card_id);
                        if(mark == cc->getSuitString()){
                            room->throwCard(card_id);
                            room->takeAG(NULL, card_id);
                        }
                    }
                    room->broadcastInvoke("clearAG");
                    room->setPlayerFlag(player, "-"+mark);
                }
            }
        }
        else{
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason != objectName())
                return false;
            const Card *card = room->askForCard(player, ".", "@langgu", data, AskForRetrial);
            if(card){
                room->throwCard(judge->card);
                judge->card = Sanguosha->getCard(card->getEffectiveId());
                room->moveCardTo(judge->card, NULL, Player::Special);

                LogMessage log;
                log.type = "$ChangedJudge";
                log.from = player;
                log.to << judge->who;
                log.card_str = card->getEffectIdString();
                room->sendLog(log);

                room->sendJudgeResult(judge);
            }
        }
        return false;
    }
};

class Huangen: public TriggerSkill{
public:
    Huangen(): TriggerSkill("huangen"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *liuxie = room->findPlayerBySkillName(objectName());
        if(!liuxie)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isKindOf("TrickCard") || use.to.length() < 2)
            return false;
        if(liuxie->askForSkillInvoke(objectName())){
            ServerPlayer *target = room->askForPlayerChosen(liuxie, use.to, objectName());
            use.to.removeOne(target);
            target->drawCards(1);
            data = QVariant::fromValue(use);
        }
        return false;
    }
};

class Hantong: public TriggerSkill{
public:
    Hantong():TriggerSkill("hantong"){
        events << CardDiscarded << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange){
            if(player->getPhase() == Player::Start){
                if(!player->getPile("zhao").isEmpty() && player->askForSkillInvoke(objectName())){
                    room->playSkillEffect(objectName());
                    room->throwCard(player->getPile("zhao").first());
                    QString skill = room->askForChoice(player, objectName(), "hujia+jijiang+jiuyuan+xueyi");
                    player->tag["Zhao"] = skill;
                    room->acquireSkill(player, skill);
                }
            }
            else if(player->getPhase() == Player::NotActive){
                QString skill = player->tag.value("Zhao").toString();
                room->detachSkillFromPlayer(player, skill);
            }
            return false;
        }
        if(player->getPhase() == Player::Discard){
            CardStar card = data.value<CardStar>();
            if(card->getSubcards().isEmpty() || !player->askForSkillInvoke(objectName()))
                return false;
            room->playSkillEffect(objectName());
            foreach(int card_id, card->getSubcards())
                player->addToPile("zhao", card_id);
        }

        return false;
    }
};

Yic0ngCard::Yic0ngCard(){
    target_fixed = true;
    will_throw = false;
}

void Yic0ngCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    foreach(int x, getSubcards())
        source->addToPile("hoo", x);
    source->acquireSkill("#yic0ng-distance");
}

class Yic0ngViewAsSkill:public ViewAsSkill{
public:
    Yic0ngViewAsSkill():ViewAsSkill("yic0ng"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@yic0ng";
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        Yic0ngCard *card = new Yic0ngCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
};

class Yic0ng:public PhaseChangeSkill{
public:
    Yic0ng():PhaseChangeSkill("yic0ng"){
        view_as_skill = new Yic0ngViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Finish)
            player->getRoom()->askForUseCard(player, "@@yic0ng", "@yic0ng");
        return false;
    }
};

class Yic0ngDistance: public DistanceSkill{
public:
    Yic0ngDistance():DistanceSkill("#yic0ng-distance"){
    }

    virtual int getCorrect(const Player *, const Player *to) const{
        if(to->hasSkill(objectName()))
            return to->getPile("hoo").length();
        else
            return 0;
    }
};

class TuqiEffect:public PhaseChangeSkill{
public:
    TuqiEffect():PhaseChangeSkill("#tuqi-effect"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start){
            int x = player->getPile("hoo").length();
            if(x > 0)
                room->playSkillEffect("tuqi");
            room->setPlayerMark(player, "@tuqi", x);
            player->clearPile("hoo");
            if(x <= 2)
                player->drawCards(1);
        }
        else if(player->getPhase() == Player::NotActive)
            room->setPlayerMark(player, "@tuqi", 0);
        return false;
    }
};

class Tuqi: public DistanceSkill{
public:
    Tuqi():DistanceSkill("tuqi"){
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if(from->hasSkill(objectName()))
            return - from->getMark("@tuqi");
        else
            return 0;
    }
};

MingjianCard::MingjianCard(){
    will_throw = false;
    target_fixed = true;
}

void MingjianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &) const{
    ServerPlayer *target = room->getCurrent();
    foreach(int id, getSubcards())
        target->addToPile("jian", id);
}

class MingjianViewAsSkill: public OneCardViewAsSkill{
public:
    MingjianViewAsSkill():OneCardViewAsSkill("mingjian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@mingjian";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new MingjianCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Mingjian:public PhaseChangeSkill{
public:
    Mingjian():PhaseChangeSkill("mingjian"){
        view_as_skill = new MingjianViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::NotActive){
            player->clearPile("jian");
            return false;
        }
        ServerPlayer *xin = room->findPlayerBySkillName(objectName());
        if(!xin || player->getPhase() != Player::Start || xin->isNude())
            return false;
        QString choice = room->askForChoice(xin, objectName(), !player->getJudgingArea().isEmpty() ? "ming+jian+cancel" : "jian+cancel");
        if(choice == "cancel")
            return false;
        if(choice == "ming"){
            if(room->askForCard(xin, "..", "@mingjian1:" + player->objectName(), QVariant::fromValue((PlayerStar)player), CardDiscarded))
                player->skip(Player::Judge);
        }
        else
            room->askForUseCard(xin, "@@mingjian", "@mingjian2:" + player->objectName());

        return false;
    }
};

class MingjianStop: public TriggerSkill{
public:
    MingjianStop():TriggerSkill("#mingjian-stop"){
        events << AskForRetrial;
    }

    virtual int getPriority() const{
        return 5;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room*, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        return !judge->who->getPile("jian").isEmpty();
    }
};

class Yinzhi:public MasochismSkill{
public:
    Yinzhi():MasochismSkill("yinzhi"){
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        if(!damage.from || !room->askForSkillInvoke(guojia, objectName()))
            return;

        room->playSkillEffect(objectName());

        for(int i=0; i< damage.damage; i++){
            QList<int> card_ids = room->getNCards(2);
            room->fillAG(card_ids);
            foreach(int id, card_ids){
                CardStar card = Sanguosha->getCard(id);
                if(card->getSuit() == Card::Spade){
                    if(!damage.from->isKongcheng()){
                        ServerPlayer *target = room->askForPlayerChosen(guojia, room->getOtherPlayers(damage.from), objectName());
                        room->obtainCard(target, room->askForCardChosen(target, damage.from, "h", objectName()), false);
                    }
                    room->throwCard(id);
                }
                else
                    room->obtainCard(guojia, id);
            }
            room->broadcastInvoke("clearAG");
        }

    }
};

PasterPackage::PasterPackage()
    :Package("paster")
{
    General *xinxianying = new General(this, "xinxianying", "wei", 3, false);
    xinxianying->addSkill(new Mingjian);
    xinxianying->addSkill(new MingjianStop);
    xinxianying->addSkill(new Yinzhi);
    related_skills.insertMulti("mingjian", "#mingjian-stop");

    General *wangyuanji = new General(this, "wangyuanji", "wei", 3, false);
    wangyuanji->addSkill(new Fuluan);
    wangyuanji->addSkill(new Shude);
    skills << new FuluanSlash;

    General *simazhao = new General(this, "simazhao", "wei", 3);
    simazhao->addSkill(new Zhaoxin);
    simazhao->addSkill(new Langgu);

    General *liuxie = new General(this, "liuxie", "qun");
    liuxie->addSkill(new Huangen);
    liuxie->addSkill(new Hantong);

    General *gongshunzan = new General(this, "gongshunzan", "qun");
    gongshunzan->addSkill(new Yic0ng);
    gongshunzan->addSkill(new Tuqi);
    gongshunzan->addSkill(new TuqiEffect);
    related_skills.insertMulti("tuqi", "#tuqi-effect");

    skills << new Yic0ngDistance;
    addMetaObject<FuluanCard>();
    addMetaObject<Yic0ngCard>();
    addMetaObject<MingjianCard>();
}

ADD_PACKAGE(BGM)
ADD_PACKAGE(Paster)
