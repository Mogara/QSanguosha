#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"

class Xingshang: public GameStartSkill{
public:
    Xingshang():GameStartSkill("xingshang"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setLegatee(player);
    }
};

ShenfenCard::ShenfenCard(){
    target_fixed = true;
}

void ShenfenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->turnOver();
    room->broadcastProperty(source, "face_up");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);

    foreach(ServerPlayer *player, players){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = source;
        damage.to = player;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }

    foreach(ServerPlayer *player, players){
        player->throwAllEquips();
    }

    foreach(ServerPlayer *player, players){
        int discard_num = qMin(player->getHandcardNum(), 4);
        room->askForDiscard(player, discard_num);
    }
}

FangzhuCard::FangzhuCard(){

}

class FangzhuViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuViewAsSkill():ZeroCardViewAsSkill("fangzhu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@fangzhu";
    }

    virtual const Card *viewAs() const{
        return new FangzhuCard;
    }
};

class Fangzhu: public MasochismSkill{
public:
    Fangzhu():MasochismSkill("fangzhu"){
        view_as_skill = new FangzhuViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &damage) const{
        // exile somebody        
        Room *room = caopi->getRoom();
        if(room->askForSkillInvoke(caopi, objectName())){
            QList<ServerPlayer *> targets;
            room->askForCardWithTargets(caopi, "@@fangzhu", "@fangzhu", targets);

            ServerPlayer *to_exile = targets.first();
            to_exile->drawCards(caopi->getLostHp());

            bool face_up = !to_exile->faceUp();
            room->setPlayerProperty(to_exile, "face_up", face_up);
        }

    }
};

class Songwei: public TriggerSkill{
public:
    Songwei():TriggerSkill("songwei$"){
        events << JudgeOnEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

class Duanliang: public ViewAsSkill{
public:
    Duanliang():ViewAsSkill("duanliang"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        const Card *card = to_select->getCard();
        return card->isBlack() && !card->inherits("TrickCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *first = cards.first()->getCard();

        SupplyShortage *card = new SupplyShortage(first->getSuit(), first->getNumber());
        card->addSubcard(first->getId());

        return card;
    }
};

class Huoshou: public GameStartSkill{
public:
    Huoshou():GameStartSkill("huoshou"){

    }

    void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setMenghuo(player);
    }
};

YinghunCard::YinghunCard(){

}

void YinghunCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();
    Room *room = effect.from->getRoom();

    if(x == 1){
        effect.to->drawCards(1);
        room->askForDiscard(effect.to, 1);
    }else{
        QString choice = room->askForChoice(effect.from, "yinghun", "d1tx+dxt1");
        if(choice == "d1tx"){
            effect.from->drawCards(1);
            room->askForDiscard(effect.to, x); // FIXME:
        }else{
            effect.from->drawCards(x);
            room->askForDiscard(effect.to, 1);
        }
    }
}

class YinghunViewAsSkill: public ZeroCardViewAsSkill{
public:
    YinghunViewAsSkill():ZeroCardViewAsSkill("yinghun"){
    }

    virtual const Card *viewAs() const{
        return new YinghunCard;        
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@yinghun";
    }
};

class Yinghun: public PhaseChangeSkill{
public:
    Yinghun():PhaseChangeSkill("yinghun"){
        view_as_skill = new YinghunViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *sunjian) const{
        if(sunjian->getPhase() == Player::Start && sunjian->isWounded()){
            Room *room = sunjian->getRoom();
            QList<ServerPlayer *> targets;
            const Card *card = room->askForCardWithTargets(sunjian, "@@yinghun", "@yinghun-card:" + sunjian->getGeneralName(), targets);
            if(card){
                CardEffectStruct effect;
                effect.from = sunjian;
                effect.to = targets.first();
                effect.card = card;

                room->cardEffect(effect);
            }
        }

        return false;
    }
};

class Haoshi: public PhaseChangeSkill{
public:
    Haoshi():PhaseChangeSkill("haoshi"){
    }

    virtual int getPriority(ServerPlayer *) const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Draw){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(2);

                if(target->getHandcardNum() > 5){
                    // find player(s) who has the least handcards

                }
            }
        }

        return false;
    }
};

DimengCard::DimengCard(){

}

bool DimengCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() <= 1)
        return to_select != Self;
    else
        return false;
}

bool DimengCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

class Dimeng: public ZeroCardViewAsSkill{
public:
    Dimeng():ZeroCardViewAsSkill("dimeng"){

    }

    virtual const Card *viewAs() const{
        return new DimengCard;
    }
};

class Luanwu: public ZeroCardViewAsSkill{
public:
    Luanwu():ZeroCardViewAsSkill("luanwu"){

    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->tag.value("luanwu_used", false).toBool();
    }
};

LuanwuCard::LuanwuCard(){
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        CardEffectStruct effect;
        effect.from = source;
        effect.to = player;
        effect.card = this;
    }
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{

}

void LuanwuCard::use(const QList<const ClientPlayer *> &) const{
    ClientInstance->tag.insert("luanwu_used", true);
}

class Jiuchi: public ViewAsSkill{
public:
    Jiuchi():ViewAsSkill("jiuchi"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *first = cards.first()->getCard();
        Analeptic *analeptic = new Analeptic(first->getSuit(), first->getNumber());
        analeptic->addSubcard(first->getId());

        return analeptic;
    }
};

class Benghuai: public PhaseChangeSkill{
public:
    Benghuai():PhaseChangeSkill("benghuai"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const{
        bool trigger_this = false;
        Room *room = dongzhuo->getRoom();

        if(dongzhuo->getPhase() == Player::Finish){            
            QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
            foreach(ServerPlayer *player, players){
                if(dongzhuo->getHp() > player->getHp()){
                    trigger_this = true;
                    break;
                }
            }
        }

        if(trigger_this){
            QString result = room->askForChoice(dongzhuo, "benghuai", "hp+max_hp");

            if(result == "hp"){
                room->loseHp(dongzhuo);
            }else{
                dongzhuo->setMaxHP(dongzhuo->getMaxHP() - 1);
                if(dongzhuo->getMaxHP() == 0){
                    room->getThread()->trigger(Death, dongzhuo);
                }

                room->broadcastProperty(dongzhuo, "max_hp");
                room->broadcastProperty(dongzhuo, "hp");
            }
        }

        return false;
    }
};

class Guixin: public MasochismSkill{
public:
    Guixin():MasochismSkill("guixin"){

    }

    virtual void onDamaged(ServerPlayer *shencc, const DamageStruct &damage) const{
        Room *room = shencc->getRoom();
        int i, x = damage.damage;
        for(i=0; i<x; i++){
            if(room->askForSkillInvoke(shencc, objectName())){
                QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
                foreach(ServerPlayer *player, players){
                    if(!player->isAllNude()){
                        int card_id = room->askForCardChosen(shencc, player, "hej", objectName());
                        room->obtainCard(shencc, card_id);
                    }
                }

                shencc->turnOver();
                room->broadcastProperty(shencc, "face_up");
            }else
                break;
        }
    }
};

class Feiying: public GameStartSkill{
public:
    Feiying():GameStartSkill("feiying"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setPlayerCorrect(player, "skill_dest", +1);
    }
};

class Baonu: public TriggerSkill{
public:
    Baonu():TriggerSkill("baonu"){
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        // increase his baonu marks

        return false;
    }
};

class Wumo: public TriggerSkill{
public:
    Wumo():TriggerSkill("wumo"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("TrickCard") && !use.card->inherits("DelayedTrick")){
            Room *room = player->getRoom();
            room->loseHp(player);
        }

        return false;
    }
};

class Shenfen:public ZeroCardViewAsSkill{
public:
    Shenfen():ZeroCardViewAsSkill("shenfen"){

    }

    virtual const Card *viewAs() const{
        return new ShenfenCard;
    }
};

ThicketPackage::ThicketPackage()
    :Package("thicket")
{
    General *caopi, *xuhuang, *menghuo, *zhurong, *sunjian, *lusu, *jiaxu, *dongzhuo;

    caopi = new General(this, "caopi$", "wei", 3);
    caopi->addSkill(new Xingshang);
    caopi->addSkill(new Fangzhu);
    caopi->addSkill(new Songwei);

    xuhuang = new General(this, "xuhuang", "wei");
    xuhuang->addSkill(new Duanliang);

    menghuo = new General(this, "menghuo", "shu");
    menghuo->addSkill(new Huoshou);

    zhurong = new General(this, "zhurong", "shu", 4, false);

    sunjian = new General(this, "sunjian", "wu");
    sunjian->addSkill(new Yinghun);

    lusu = new General(this, "lusu", "wu", 3);
    lusu->addSkill(new Haoshi);
    lusu->addSkill(new Dimeng);

    jiaxu = new General(this, "jiaxu", "qun", 3);
    jiaxu->addSkill(new Skill("wansha"));
    jiaxu->addSkill(new Skill("weimu"));
    jiaxu->addSkill(new Luanwu);

    dongzhuo = new General(this, "dongzhuo$", "qun", 8);
    dongzhuo->addSkill(new Jiuchi);
    dongzhuo->addSkill(new Benghuai);

    // two gods !!
    General *shencaocao, *shenlubu;

    shencaocao = new General(this, "shencaocao$", "wei", 3);
    shencaocao->addSkill(new Guixin);
    shencaocao->addSkill(new Feiying);

    shenlubu = new General(this, "shenlubu", "qun", 5);
    shenlubu->addSkill(new Baonu);
    shenlubu->addSkill(new Shenfen);

    t["thicket"] = tr("thicket");

    t["caopi"] = tr("caopi");
    t["xuhuang"] = tr("xuhuang");
    t["menghuo"] = tr("menghuo");
    t["zhurong"] = tr("zhurong");
    t["sunjian"] = tr("sunjian");
    t["lusu"] = tr("lusu");
    t["jiaxu"] = tr("jiaxu");
    t["dongzhuo"] = tr("dongzhuo");

    t["shencaocao"] = tr("shencaocao");
    t["shenlubu"] = tr("shenlubu");

    // skills
    t["xingshang"] = tr("xingshang");
    t["fangzhu"] = tr("fangzhu");
    t["songwei"] = tr("songwei");
    t["duanliang"] = tr("duanliang");
    t["huoshuo"] = tr("huoshuo");
    t["zaiqi"] = tr("zaiqi");
    t["juxiang"] = tr("juxiang");
    t["lieren"] = tr("lieren");
    t["yinghun"] = tr("yinghun");
    t["haoshi"] = tr("haoshi");
    t["dimeng"] = tr("dimeng");
    t["wansha"] = tr("wansha");
    t["weimu"] = tr("weimu");
    t["luanwu"] = tr("luanwu");
    t["jiuchi"] = tr("jiuchi");
    t["roulin"] = tr("roulin");
    t["benghuai"] = tr("benghuai");
    t["baonue"] = tr("baonue");

    t["guixin"] = tr("guixin");
    t["feiying"] = tr("feiying");
    t["baonu"] = tr("baonu");
    t["wumou"] = tr("wumou");
    t["wuqian"] = tr("wuqian");
    t["shenfen"] = tr("shenfen");

    // skill descriptive texts
    t[":benghuai:"] = tr(":benghuai:");
    t["benghuai:hp"] = tr("benghuai:hp");
    t["benghuai:max_hp"] = tr("benghuai:max_hp");

    metaobjects << &DimengCard::staticMetaObject
            << &LuanwuCard::staticMetaObject
            << &YinghunCard::staticMetaObject
            << &FangzhuCard::staticMetaObject;
}

ADD_PACKAGE(Thicket)
