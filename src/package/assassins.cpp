#include "assassins.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

FengyinCard::FengyinCard(){
    target_fixed = true;
    will_throw = false;
}

void FengyinCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &) const{
    PlayerStar target = room->getCurrent();
    target->obtainCard(this, false);
}

class FengyinViewAsSkill:public OneCardViewAsSkill{
public:
    FengyinViewAsSkill():OneCardViewAsSkill("fengyin"){
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fengyin";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isKindOf("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FengyinCard *card = new FengyinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Fengyin:public TriggerSkill{
public:
    Fengyin():TriggerSkill("fengyin"){
        events << PhaseChange;
        view_as_skill = new FengyinViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(player != splayer && player->getPhase() == Player::Start){
            if(player->getHp() >= splayer->getHp()){
                if(room->askForUseCard(splayer, "@@fengyin", "@fengyin:" + player->objectName())){
                    player->skip(Player::Play);
                    player->skip(Player::Discard);
                }
            }
        }
        return false;
    }
};

class ChizhongKeep: public MaxCardsSkill{
public:
    ChizhongKeep():MaxCardsSkill("chizhong"){
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return target->getLostHp();
        else
            return 0;
    }
};

class Chizhong: public TriggerSkill{
public:
    Chizhong():TriggerSkill("#chizhong"){
        events << Death << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(event == PhaseChange && splayer == player && player->getPhase() == Player::Discard) {
            if(player->getHandcardNum() > player->getHp()){
                LogMessage log;
                log.type = "#Chizhong";
                log.from = splayer;
                log.arg = "chizhong";
                room->sendLog(log);
                room->playSkillEffect("chizhong", 1);
            }
            return false;
        }

        if(event != Death || player == splayer)
            return false;

        room->setPlayerProperty(splayer, "maxhp", splayer->getMaxHp()+1);
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = splayer;
        log.arg = "chizhong";
        room->sendLog(log);
        room->playSkillEffect("chizhong", 2);

        return false;
    }
};

MixinCard::MixinCard(){
    will_throw = false;
    mute = true;
}

bool MixinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void MixinCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *source = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = source->getRoom();
    room->playSkillEffect("mixin", 1);
    target->obtainCard(this, false);
    QList<ServerPlayer *> others;
    foreach(ServerPlayer *p, room->getOtherPlayers(target))
        if(target->canSlash(p, false))
            others << p;

    if(others.isEmpty())
        return;

    ServerPlayer *target2 = room->askForPlayerChosen(source, others, "mixin");
    LogMessage log;
    log.type = "#CollateralSlash";
    log.from = source;
    log.to << target2;
    room->sendLog(log);
    room->setPlayerFlag(target, "jiefanUsed");
    const Card *card = room->askForCard(target, "slash", "@mixin:" + target2->objectName(), QVariant());
    if(card){
        room->setPlayerFlag(target, "-jiefanUsed");
        room->playSkillEffect("mixin", 2);
        CardUseStruct card_use;
        card_use.from = target;
        card_use.to << target2;
        card_use.card = card;
        room->useCard(card_use, false);
    }
    else{
        room->setPlayerFlag(target, "-jiefanUsed");
        room->playSkillEffect("mixin", 3);
        QList<int> card_ids = target->handCards();
        room->fillAG(card_ids, target2);
        int cdid = room->askForAG(target2, card_ids, false, objectName());
        room->obtainCard(target2, cdid, false);
        target2->invoke("clearAG");
    }
}

class Mixin:public OneCardViewAsSkill{
public:
    Mixin():OneCardViewAsSkill("mixin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MixinCard");
    }

    virtual bool viewFilter(const CardItem *card) const{
        return !card->isEquipped();
    }

    virtual const Card *viewAs(CardItem *originalCard) const{
        MixinCard *card = new MixinCard;
        card->addSubcard(originalCard->getFilteredCard());
        return card;
    }
};

class Cangni: public TriggerSkill{
public:
    Cangni():TriggerSkill("cangni"){
        events << PhaseChange << CardLost << CardLostDone << CardGot << CardGotDone;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange){
            if(player->getPhase() != Player::Discard || !player->askForSkillInvoke(objectName()))
                return false;
            QStringList choices;
            choices << "draw";
            if(player->isWounded())
                choices << "recover";

            QString choice;
            if(choices.size() == 1)
                choice = choices.first();
            else
                choice = room->askForChoice(player, objectName(), choices.join("+"));

            if(choice == "recover") {
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
            else
                player->drawCards(2);

            room->playSkillEffect(objectName(), 1);
            player->turnOver();
            return false;
        }
        else if(event == CardGot)
            player->tag["GotCangni"] = true;
        else if(event == CardLost)
            player->tag["LostCangni"] = true;
        else{
            if(player->getPhase() != Player::NotActive || player->faceUp())
                return false;

            ServerPlayer *target = room->getCurrent();
            if(target->isDead())
                return false;
            if(event == CardLostDone && !target->isNude() && player->tag.value("LostCangni", false).toBool()
                && player->askForSkillInvoke(objectName(), "cangni_lost")){
                player->tag.remove("LostCangni");
                room->playSkillEffect(objectName(), 3);
                room->askForDiscard(target, objectName(), 1, 1, false, true);
                return false;
            }

            if(event == CardGotDone && !target->hasFlag("cangni_used") && player->tag.value("GotCangni", false).toBool()
                && player->askForSkillInvoke(objectName(), "cangni_got")){
                player->tag.remove("GotCangni");
                room->setPlayerFlag(target, "cangni_used");
                room->playSkillEffect(objectName(), 2);
                target->drawCards(1);
            }
        }
        return false;
    }
};                    

DuyiCard::DuyiCard(){
    target_fixed = true;
}

void DuyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> card_ids = room->getNCards(1);
    int id = card_ids.first();
    room->fillAG(card_ids, NULL);
    room->getThread()->delay();
    ServerPlayer *target = room->askForPlayerChosen(source, room->getAlivePlayers(), "duyi");
    const Card *card = Sanguosha->getCard(id);
    target->obtainCard(card);
    if(card->isBlack()) {
        room->setPlayerCardLock(target, ".|.|.|hand");
        //target->jilei(".|.|.|hand");
        //target->invoke("jilei", ".|.|.|hand");
        room->setPlayerFlag(target, "duyi_target");
        LogMessage log;
        log.type = "#duyi_eff";
        log.from = source;
        log.to << target;
        log.arg = "duyi";
        room->sendLog(log);
    }

    room->getThread()->delay();
    foreach(ServerPlayer *p, room->getPlayers())
        p->invoke("clearAG");
}

class DuyiViewAsSkill:public ZeroCardViewAsSkill{
public:
    DuyiViewAsSkill():ZeroCardViewAsSkill("duyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuyiCard");
    }

    virtual const Card *viewAs() const{
        return new DuyiCard;
    }
};

class Duyi:public TriggerSkill{
public:
    Duyi():TriggerSkill("duyi"){
        view_as_skill = new DuyiViewAsSkill;
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(splayer->getPhase() == Player::Discard)
            if(splayer->hasFlag("duyi_target")){
                room->setPlayerCardLock(splayer, "-.|.|.|hand");
                //splayer->jilei(".");
                //splayer->invoke("jilei", ".");
                room->setPlayerFlag(splayer, "-duyi_target");
            }

        if(splayer->getPhase() == Player::NotActive)
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if(p->hasFlag("duyi_target")) {
                    room->setPlayerCardLock(p, "-.|.|.|hand");
                    //p->jilei(".");
                    //p->invoke("jilei", ".");
                    room->setPlayerFlag(p, "-duyi_target");
                    LogMessage log;
                    log.type = "#duyi_clear";
                    log.from = p;
                    log.arg = objectName();
                    room->sendLog(log);
                }

        return false;
    }
};

class Duanzhi: public TriggerSkill{
public:
    Duanzhi(): TriggerSkill("duanzhi") {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from == effect.to)
            return false;
        if(effect.card->getTypeId() == Card::Skill)
            return false;

        if(player->askForSkillInvoke(objectName(), data)) {
            room->playSkillEffect(objectName());
            DummyCard *dummy = new DummyCard;
            QList<int> card_ids;
            for(int i = 0; i < 2; i++) {
                if(!effect.from || effect.from->getCardCount(true) - i == 0)
                    break;
                if(room->askForChoice(player, objectName(), "discard+cancel") == "cancel")
                    break;
                int card_id = room->askForCardChosen(player, effect.from, "he", objectName());
                while(card_ids.contains(card_id))
                    card_id = effect.from->getRandomCardId("he");
                card_ids << card_id;
                dummy->addSubcard(card_id);
            }
            room->throwCard(dummy, effect.from, player);
            delete dummy;
            room->loseHp(player);
        }
        return false;
    }
};

AssassinsPackage::AssassinsPackage()
    :Package("assassins")
{
    General *mushun = new General(this, "mushun", "qun", 3);
    mushun->addSkill(new Fengyin);
    mushun->addSkill(new ChizhongKeep);
    mushun->addSkill(new Chizhong);
    related_skills.insertMulti("chizhong", "#chizhong");
    addMetaObject<FengyinCard>();

    General *fushi = new General(this, "fushi", "qun", 3, false);
    fushi->addSkill(new Mixin);
    fushi->addSkill(new Cangni);
    addMetaObject<MixinCard>();

    General *jiben = new General(this, "jiben", "qun", 3);
    jiben->addSkill(new Duyi);
    jiben->addSkill(new Duanzhi);
    addMetaObject<DuyiCard>();
}

//Olympics
JisuCard::JisuCard(){
}

bool JisuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void JisuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName(skill_name);
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use);
}

class JisuViewAsSkill: public ZeroCardViewAsSkill{
public:
    JisuViewAsSkill():ZeroCardViewAsSkill("jisu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jisu";
    }

    virtual const Card *viewAs() const{
        return new JisuCard;
    }
};

class Jisu: public PhaseChangeSkill{
public:
    Jisu():PhaseChangeSkill("jisu"){
        view_as_skill = new JisuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *poem) const{
        Room *room = poem->getRoom();
        if(poem->getPhase() == Player::Judge){
            if(room->askForUseCard(poem, "@@jisu", "@jisu")){
                poem->skip(Player::Draw);
                return true;
            }
        }
        return false;
    }
};

class Shuiyong: public TriggerSkill{
public:
    Shuiyong():TriggerSkill("shuiyong"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#ShuiyongProtect";
            log.from = player;
            log.arg2 = QString::number(damage.damage);
            log.arg = objectName();
            room->sendLog(log);

            return true;
        }else
            return false;
    }
};

class Shuijian:public DrawCardsSkill{
public:
    Shuijian():DrawCardsSkill("shuijian"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *sunyang, int n) const{
        Room *room = sunyang->getRoom();
        if(room->askForSkillInvoke(sunyang, objectName())){
            int x = sunyang->getEquips().count();
            if(x > 0)
                room->playSkillEffect(objectName());
            return n + x/2 + 1;
        }else
            return n;
    }
};

OlympicsPackage::OlympicsPackage():Package("olympics"){
    General *yeshiwen = new General(this, "yeshiwen", "wu", 3, false);
    yeshiwen->addSkill(new Jisu);
    yeshiwen->addSkill(new Shuiyong);

    General *sunyang = new General(this, "sunyang", "wu");
    sunyang->addSkill(new Shuijian);

    addMetaObject<JisuCard>();
}

ADD_PACKAGE(Assassins)
ADD_PACKAGE(Olympics)
