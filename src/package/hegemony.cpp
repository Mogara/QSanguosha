#include "hegemony.h"
#include "standard.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"

class Xiaoguo: public PhaseChangeSkill{
public:
    Xiaoguo():PhaseChangeSkill("xiaoguo"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Finish)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer || player == splayer || splayer->isKongcheng())
            return false;
        if(room->askForCard(splayer, "BasicCard", "@xiaoguo", QVariant(), CardDiscarded)){
            room->playSkillEffect(objectName());
            if(!room->askForCard(player, "EquipCard,TrickCard", "@xiaoguoresponse:" + splayer->objectName(), QVariant(), CardDiscarded)){
                DamageStruct damage;
                damage.from = splayer;
                damage.to = player;
                room->damage(damage);
            }
        }
        return false;
    }
};

class Shushen: public TriggerSkill{
public:
    Shushen():TriggerSkill("shushen"){
        events << HpRecover;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        RecoverStruct recover = data.value<RecoverStruct>();
        int x = recover.recover, i;
        for(i=0; i<x; i++){
            if(player->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), "shushen1");
                target->drawCards(1);
                target = room->askForPlayerChosen(player, room->getOtherPlayers(player), "shushen2");
                target->drawCards(1);
            }
        }
        return false;
    }
};

class Shenzhi:public PhaseChangeSkill{
public:
    Shenzhi():PhaseChangeSkill("shenzhi"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start || player->isKongcheng())
            return false;
        Room *room = player->getRoom();
        if(player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            int x = player->getHandcardNum();
            player->throwAllHandCards();
            if(x >= player->getHp()){
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover, true);
            }
        }
        return false;
    }
};

DuoshiCard::DuoshiCard(){
}

bool DuoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void DuoshiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->drawCards(2);
    targets[1]->drawCards(2);
    room->askForDiscard(source, skill_name, 2,2, false, true);
    room->askForDiscard(targets[1], skill_name, 2,2, false, true);
}

class Duoshi: public OneCardViewAsSkill{
public:
    Duoshi():OneCardViewAsSkill("duoshi"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DuoshiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Duanbing: public TriggerSkill{
public:
    Duanbing():TriggerSkill("duanbing"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isKindOf("Slash"))
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(player->distanceTo(p) == 1 && !use.to.contains(p) && player->canSlash(p, false))
                targets << p;
        }
        if(targets.isEmpty())
            return false;
        if(player->askForSkillInvoke(objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            use.to.append(target);
            data = QVariant::fromValue(use);
        }
        return false;
    }
};

FenxunCard::FenxunCard(){
    once = true;
    will_throw = false;
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FenxunCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->setPlayerMark(targets[1], "fenxuntarget", 1);
    room->setFixedDistance(source, targets[1], 1);
}

class FenxunViewAsSkill: public OneCardViewAsSkill{
public:
    FenxunViewAsSkill():OneCardViewAsSkill("fenxun"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FenxunCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new FenxunCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Fenxun: public PhaseChangeSkill{
public:
    Fenxun():PhaseChangeSkill("fenxun"){
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAlivePlayers();
            foreach(ServerPlayer *player, players){
                if(player->getMark("fenxuntarget") > 0){
                    room->setPlayerMark(player, "fenxuntarget", 0);
                    room->setFixedDistance(target, player, -1);
                }
            }
        }
        return false;
    }
};

class Mingshi: public TriggerSkill{
public:
    Mingshi():TriggerSkill("mingshi"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from->isKongcheng()){
            Room *room = player->getRoom();
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            QString choice = room->askForChoice(damage.from, objectName(), "mingshishow+mingshicancel");
            if(choice == "mingshishow"){
                room->playSkillEffect(objectName(), 1);
                room->showAllCards(damage.from);
            }
            else{
                room->playSkillEffect(objectName(), 2);
                damage.damage --;
                damage.damage = qMax(damage.damage, 0);
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Lirang: public TriggerSkill{
public:
    Lirang():TriggerSkill("lirang"){
        events << CardDiscarded;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardStar card = data.value<CardStar>();
        Room *room = player->getRoom();
        if(player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            ServerPlayer *t = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            t->obtainCard(card);
        }
        return false;
    }
};

HegemonyPackage::HegemonyPackage()
    :Package("hegemony")
{
    General *yuejin = new General(this, "yuejin", "wei");
    yuejin->addSkill(new Xiaoguo);

    General *ganfuren = new General(this, "ganfuren", "shu", 3, false);
    ganfuren->addSkill(new Shushen);
    ganfuren->addSkill(new Shenzhi);

    General *lushun = new General(this, "lushun", "wu", 3);
    lushun->addSkill("qianxun");
    lushun->addSkill(new Duoshi);

    General *dingfeng = new General(this, "dingfeng", "wu");
    dingfeng->addSkill(new Duanbing);
    dingfeng->addSkill(new Fenxun);

    General *kongrong = new General(this, "kongrong", "qun", 3);
    kongrong->addSkill(new Mingshi);
    kongrong->addSkill(new Lirang);
/*
    General *tianfeng = new General(this, "tianfeng", "qun", 3);
    tianfeng->addSkill(sijian);
    tianfeng->addSkill(suishi);

    General *jiling = new General(this, "jiling", "qun");
    jiling->addSkill(shuangren);

    General *zoushi = new General(this, "zoushi", "qun", 3, false);
    zoushi->addSkill(huoshui);
    zoushi->addSkill(qingcheng);

    General *mateng = new General(this, "mateng", "qun");
    mateng->addSkill("mashu");
    mateng->addSkill(xiongyi);

    General *panfeng = new General(this, "panfeng", "qun");
    panfeng->addSkill(kuangfu);
*/
    addMetaObject<DuoshiCard>();
    addMetaObject<FenxunCard>();
}

ADD_PACKAGE(Hegemony)
