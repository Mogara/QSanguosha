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
        if(room->askForCard(splayer, "BasicCard", "@xiaoguo:" + player->objectName(), QVariant::fromValue((PlayerStar)player), CardDiscarded)){
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isKindOf("Slash"))
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(player->distanceTo(p) == 1 && !use.to.contains(p) && player->canSlash(p, use.card, false))
                targets << p;
        }
        if(!targets.isEmpty() && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from->isKongcheng() && damage.damage > 0){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            QString choice = room->askForChoice(damage.from, objectName(), "mingshishow+mingshicancel");
            if(choice == "mingshishow")
                room->showAllCards(damage.from);
            else{
                damage.damage --;
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardStar card = data.value<CardStar>();
        if(player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            ServerPlayer *t = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            t->obtainCard(card);
        }
        return false;
    }
};

class Kuangfu: public TriggerSkill{
public:
    Kuangfu():TriggerSkill("kuangfu"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->isKindOf("Slash"))
            return false;
        if(damage.to->hasEquip() && player->askForSkillInvoke(objectName(),data)){
            room->playSkillEffect(objectName());
            int card_id = room->askForCardChosen(player, damage.to, "e", objectName());
            const Card *card = Sanguosha->getCard(card_id);
            if(room->askForChoice(player, objectName(), "kuangfuget+kuangfudis") == "kuangfuget")
                player->obtainCard(card);
            else
                room->throwCard(card);
        }
        return false;
    }
};

XiongyiCard::XiongyiCard(){
    once = true;
    will_throw = false;
}

bool XiongyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = Self->aliveCount();
    x = qMax(2, x%2+x/2);
    return targets.length() < x-1;
}

void XiongyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->loseMark("@xiongyi");
    bool onlyme = false;
    if(targets.contains(source) && targets.length() == 1)
        onlyme = true;
    foreach(ServerPlayer *tmp, targets)
        tmp->drawCards(3);
    if(onlyme && source->isWounded()){
        RecoverStruct r;
        room->recover(source, r, true);
    }
}

class Xiongyi: public ZeroCardViewAsSkill{
public:
    Xiongyi():ZeroCardViewAsSkill("xiongyi"){
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new XiongyiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@xiongyi") > 0;
    }
};

class Huoshui: public PhaseChangeSkill{
public:
    Huoshui():PhaseChangeSkill("huoshui"){
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(target->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                target->turnOver();
                ServerPlayer *t = room->askForPlayerChosen(target, room->getOtherPlayers(target), objectName());
                t->turnOver();
            }
        }
        return false;
    }
};

QingchengCard::QingchengCard(){
    once = true;
}

bool QingchengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void QingchengCard::onEffect(const CardEffectStruct &effect) const{
    QStringList skillist;
    Room *room = effect.from->getRoom();
    foreach(const SkillClass *skill, effect.to->getVisibleSkillList()){
        if(skill->getLocation() == Skill::Right &&
           skill->getFrequency() != Skill::Limited &&
           skill->getFrequency() != Skill::Wake &&
           !skill->isLordSkill()){
            skillist << skill->objectName();
        }
    }
    if(!skillist.isEmpty()){
        QString ski = room->askForChoice(effect.from, skill_name, skillist.join("+"));
        room->acquireSkill(effect.from, ski);
        effect.from->tag["Qingcheng"] = QVariant::fromValue(ski);
    }
}

class QingchengViewAsSkill: public OneCardViewAsSkill{
public:
    QingchengViewAsSkill():OneCardViewAsSkill("qingcheng"){
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@qingcheng";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new QingchengCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Qingcheng: public PhaseChangeSkill{
public:
    Qingcheng():PhaseChangeSkill("qingcheng"){
        view_as_skill = new QingchengViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::RoundStart){
            QString lzskill = player->tag["Qingcheng"].toString();
            player->getRoom()->detachSkillFromPlayer(player, lzskill);
            player->getRoom()->askForUseCard(player, "@@qingcheng", "@qingcheng");
        }
        return false;
    }
};

ShuangrenCard::ShuangrenCard(){
    will_throw = false;
    mute = true;
}

bool ShuangrenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return !to_select->isKongcheng();
}

void ShuangrenCard::use(Room *room, ServerPlayer *aoko, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    bool success = aoko->pindian(target, skill_name, this);
    if(success){
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(skill_name);
        ServerPlayer *victim = room->askForPlayerChosen(aoko, room->getOtherPlayers(aoko), skill_name);
        CardUseStruct card_use;
        card_use.from = aoko;
        card_use.to << victim;
        card_use.card = slash;
        room->useCard(card_use, false);
    }else{
        room->playSkillEffect(skill_name, 2);
        aoko->setFlags("shuangren");
    }
}

class ShuangrenViewAsSkill: public OneCardViewAsSkill{
public:
    ShuangrenViewAsSkill():OneCardViewAsSkill("shuangren"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@shuangren";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ShuangrenCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Shuangren: public PhaseChangeSkill{
public:
    Shuangren():PhaseChangeSkill("shuangren"){
        view_as_skill = new ShuangrenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Play && !player->isKongcheng()){
            if(player->getRoom()->askForUseCard(player, "@@shuangren", "@shuangren"))
                if(player->hasFlag("shuangren"))
                    return true;
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class Sijian: public TriggerSkill{
public:
    Sijian():TriggerSkill("sijian"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(!player->askForSkillInvoke(objectName()))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
        if(room->askForChoice(player, objectName(), "qi+mo") == "qi"){
            room->playSkillEffect(objectName(), 1);
            room->askForDiscard(target, objectName(), qMin(target->getHp(), target->getCardCount(true)), false, true);
        }else{
            room->playSkillEffect(objectName(), 2);
            target->drawCards(target->getLostHp());
        }

        return false;
    }
};

class Suishi: public TriggerSkill{
public:
    Suishi():TriggerSkill("suishi"){
        frequency = Compulsory;
        events << DamageComplete << Death << HpLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *tianfeng = room->findPlayerBySkillName(objectName());
        if(!tianfeng)
            return false;
        if(event == Death){
            if(player != tianfeng){
                room->playSkillEffect(objectName(), 2);
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = tianfeng;
                log.arg = objectName();
                room->sendLog(log);
                room->loseHp(tianfeng);
            }
            return false;
        }
        if(player == tianfeng)
            return false;
        int x = 0;
        if(event == DamageComplete){
            DamageStruct damage = data.value<DamageStruct>();
            x = damage.damage;
        }
        else
            x = data.toInt();

        if(x > 0){
            room->playSkillEffect(objectName(), 1);
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = tianfeng;
            log.arg = objectName();
            room->sendLog(log);
        }
        for(int i=0; i<x; i++){
            if(player->getHp() >= tianfeng->getHandcardNum())
                tianfeng->drawCards(1);
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

    General *tianfeng = new General(this, "tianfeng", "qun", 3);
    tianfeng->addSkill(new Sijian);
    tianfeng->addSkill(new Suishi);

    General *jiling = new General(this, "jiling", "qun");
    jiling->addSkill(new Shuangren);

    General *zoushi = new General(this, "zoushi", "qun", 3, false);
    zoushi->addSkill(new Huoshui);
    zoushi->addSkill(new Qingcheng);

    General *mateng = new General(this, "mateng", "qun");
    mateng->addSkill("mashu");
    mateng->addSkill(new Xiongyi);
    mateng->addSkill(new MarkAssignSkill("@xiongyi", 1));

    General *panfeng = new General(this, "panfeng", "qun");
    panfeng->addSkill(new Kuangfu);

    addMetaObject<DuoshiCard>();
    addMetaObject<FenxunCard>();
    addMetaObject<XiongyiCard>();
    addMetaObject<QingchengCard>();
    addMetaObject<ShuangrenCard>();
}

ADD_PACKAGE(Hegemony)
