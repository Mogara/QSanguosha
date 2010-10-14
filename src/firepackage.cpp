#include "firepackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"

QuhuCard::QuhuCard(){

}

bool QuhuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->getHp() <= Self->getHp())
        return false;

    if(to_select->isKongcheng())
        return false;

    return true;
}

void QuhuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *xunyu = effect.from;
    ServerPlayer *tiger = effect.to;
    Room *room = xunyu->getRoom();

    bool success = room->pindian(xunyu, tiger);
    if(success){
        QList<ServerPlayer *> players = room->getOtherPlayers(tiger), wolves;
        foreach(ServerPlayer *player, players){
            if(tiger->inMyAttackRange(player))
                wolves << player;
        }

        if(wolves.isEmpty())
            return;

        ServerPlayer *wolf = room->askForPlayerChosen(xunyu, wolves);

        DamageStruct damage;
        damage.from = tiger;
        damage.to = wolf;

        room->damage(damage);

    }else{
        DamageStruct damage;
        damage.card = NULL;
        damage.from = tiger;
        damage.to = xunyu;

        room->damage(damage);
    }
}

void QuhuCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("quhu_used", true);
}

JiemingCard::JiemingCard(){

}

bool JiemingCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    int upper = qMin(5, to_select->getMaxHP());
    return to_select->getHandcardNum() < upper;
}

void JiemingCard::onEffect(const CardEffectStruct &effect) const{
    int upper = qMin(5, effect.to->getMaxHP());
    int x = upper - effect.to->getHandcardNum();
    if(x <= 0)
        return;

    effect.to->drawCards(x);
}

class JiemingViewAsSkill: public ZeroCardViewAsSkill{
public:
    JiemingViewAsSkill():ZeroCardViewAsSkill("jieming"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@jieming";
    }

    virtual const Card *viewAs() const{
        return new JiemingCard;
    }
};

class Jieming: public MasochismSkill{
public:
    Jieming():MasochismSkill("jieming"){
        view_as_skill = new JiemingViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *xunyu, const DamageStruct &damage) const{
        Room *room = xunyu->getRoom();
        int x = damage.damage, i;
        for(i=0; i<x; i++){
            if(!room->askForUseCard(xunyu, "@@jieming", "@jieming"))
                break;
        }
    }
};

class Quhu: public ZeroCardViewAsSkill{
public:
    Quhu():ZeroCardViewAsSkill("quhu"){

    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("quhu_used", false).toBool() && !Self->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new QuhuCard;
    }
};

QiangxiCard::QiangxiCard(){

}

bool QiangxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(!subcards.isEmpty() && Self->getWeapon() == Sanguosha->getCard(subcards.first()))
        return Self->distanceTo(to_select) <= 1;

    return Self->inMyAttackRange(to_select);
}

void QiangxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(subcards.isEmpty())
        room->loseHp(effect.from);

    DamageStruct damage;
    damage.card = NULL;
    damage.from = effect.from;
    damage.to = effect.to;

    room->damage(damage);
}

void QiangxiCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("qiangxi_used", true);
}

class Qiangxi: public ViewAsSkill{
public:
    Qiangxi():ViewAsSkill("qiangxi"){

    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("qiangxi_used", false).toBool();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return new QiangxiCard;
        else if(cards.length() == 1){
            QiangxiCard *card = new QiangxiCard;
            card->addSubcards(cards);

            return card;
        }else
            return NULL;
    }
};

class Luanji:public ViewAsSkill{
public:
    Luanji():ViewAsSkill("luanji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getCard();
            return !to_select->isEquipped() && to_select->getCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            ArcheryAttack *aa = new ArcheryAttack(first->getSuit(), 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        }else
            return NULL;
    }
};

class ShuangxiongViewAsSkill: public OneCardViewAsSkill{
public:
    ShuangxiongViewAsSkill():OneCardViewAsSkill("shuangxiong"){
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("shuangxiong") != 0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int value = Self->getMark("shuangxiong");
        if(value == 1)
            return to_select->getCard()->isBlack();
        else if(value == 2)
            return to_select->getCard()->isRed();

        return false;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Duel *duel = new Duel(card->getSuit(), card->getNumber());
        duel->addSubcard(card->getId());
        duel->setSkillName(objectName());
        return duel;
    }
};

class Shuangxiong: public PhaseChangeSkill{
public:
    Shuangxiong():PhaseChangeSkill("shuangxiong"){
        view_as_skill = new ShuangxiongViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if(target->getPhase() == Player::Start){
            room->setPlayerMark(target, "shuangxiong", 0);
        }else if(target->getPhase() == Player::Draw){
            if(room->askForSkillInvoke(target, objectName())){
                const Card *card = room->getJudgeCard(target);
                target->obtainCard(card);
                if(card->isRed())
                    room->setPlayerMark(target, "shuangxiong", 1);
                else
                    room->setPlayerMark(target, "shuangxiong", 2);

                return true;
            }
        }

        return false;
    }
};

class Mengjin: public TriggerSkill{
public:
    Mengjin():TriggerSkill("mengjin"){
        events << SlashResult;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *pangde, QVariant &data) const{
        SlashResultStruct result = data.value<SlashResultStruct>();
        if(!result.success && !result.to->isNude()){
            Room *room = pangde->getRoom();
            room->askForSkillInvoke(pangde, objectName());
            int to_throw = room->askForCardChosen(pangde, result.to, "he", objectName());
            room->throwCard(to_throw);
        }

        return false;
    }
};

class Lianhuan: public OneCardViewAsSkill{
public:
    Lianhuan():OneCardViewAsSkill("lianhuan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
        chain->addSubcard(card->getId());
        chain->setSkillName(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill{
public:
    Niepan():TriggerSkill("niepan"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->hasFlag("niepan_used");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(room->askForSkillInvoke(player, objectName())){
            player->throwAllCards();
            room->setPlayerProperty(player, "hp", 3);
            player->drawCards(3);

            room->setPlayerFlag(player, "niepan_used");

            return true;
        }else
            return false;
    }
};

class Huoji: public OneCardViewAsSkill{
public:
    Huoji():OneCardViewAsSkill("huoji"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Bazhen: public TriggerSkill{
public:
    Bazhen():TriggerSkill("bazhen"){
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasFlag("armor_nullified") && target->getArmor() == NULL
                && target->hasSkill(objectName()) && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *wolong, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        Room *room = wolong->getRoom();
        if(room->askForSkillInvoke(wolong, objectName())){
            const Card *card = room->getJudgeCard(wolong);
            if(card->isRed()){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(wolong, Room::Good);
                return true;
            }else
                room->setEmotion(wolong, Room::Bad);
        }

        return false;
    }

};

TianyiCard::TianyiCard(){

}

bool TianyiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TianyiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *taishici = effect.from;
    Room *room = taishici->getRoom();

    bool success = room->pindian(taishici, effect.to);
    if(success){
        room->setPlayerFlag(taishici, "tianyi_success");
    }else{
        room->setPlayerFlag(taishici, "tianyi_failed");
    }
}

void TianyiCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("tianyi_used", true);
}

class TianyiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TianyiViewAsSkill():ZeroCardViewAsSkill("tianyi"){

    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("tianyi_used", false).toBool() && !Self->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new TianyiCard;
    }
};

class Tianyi: public PhaseChangeSkill{
public:
    Tianyi():PhaseChangeSkill("tianyi"){
        view_as_skill = new TianyiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(target->hasFlag("tianyi_failed"))
                room->setPlayerFlag(target, "-tianyi_failed");
            if(target->hasFlag("tianyi_success"))
                room->setPlayerFlag(target, "-tianyi_success");
        }

        return false;
    }
};

class Xueyi: public TriggerSkill{
public:
    Xueyi():TriggerSkill("xueyi$"){
        frequency = Compulsory;
        events << GameStart << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->getKingdom() == "qun";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *yuanshao = room->getLord();

        if(event == GameStart){
            if(player->isLord()){

                QList<ServerPlayer *> lieges = room->getLieges(yuanshao);
                room->setPlayerProperty(yuanshao, "xueyi", lieges.length() * 2);
            }
        }else if(event == Death){
            if(!player->isLord()){
                int xueyi = yuanshao->getXueyi();
                room->setPlayerProperty(yuanshao, "xueyi", xueyi - 2);
            }
        }

        return false;
    }
};

FirePackage::FirePackage()
    :Package("fire")
{
    General *xunyu, *dianwei, *wolong, *pangtong, *taishici, *yuanshao, *shuangxiong, *pangde;

    xunyu = new General(this, "xunyu", "wei", 3);
    xunyu->addSkill(new Quhu);
    xunyu->addSkill(new Jieming);

    dianwei = new General(this, "dianwei", "wei");
    dianwei->addSkill(new Qiangxi);

    wolong = new General(this, "wolong", "shu", 3);
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Skill("kanpo"));
    wolong->addSkill(new Bazhen);

    pangtong = new General(this, "pangtong", "shu", 3);
    pangtong->addSkill(new Lianhuan);
    pangtong->addSkill(new Niepan);

    taishici = new General(this, "taishici", "wu");
    taishici->addSkill(new Tianyi);

    yuanshao = new General(this, "yuanshao$", "qun");
    yuanshao->addSkill(new Luanji);
    yuanshao->addSkill(new Xueyi);

    shuangxiong = new General(this, "shuangxiong", "qun");
    shuangxiong->addSkill(new Shuangxiong);

    pangde = new General(this, "pangde", "qun");
    pangde->addSkill(new Mashu);
    pangde->addSkill(new Mengjin);

    t["fire"] = tr("fire");
    t["xunyu"] = tr("xunyu");
    t["dianwei"] = tr("dianwei");
    t["wolong"] = tr("wolong");
    t["pangtong"] = tr("pangtong");
    t["taishici"] = tr("taishici");
    t["yuanshao"] = tr("yuanshao");
    t["shuangxiong"] = tr("shuangxiong");
    t["pangde"] = tr("pangde");

    t["quhu"] = tr("quhu");
    t["jieming"] = tr("jieming");
    t["qiangxi"] = tr("qiangxi");
    t["lianhuan"] = tr("lianhuan");
    t["niepan"] = tr("niepan");
    t["bazhen"] = tr("bazhen");
    t["huoji"] = tr("huoji");
    t["kanpo"] = tr("kanpo");
    t["tianyi"] = tr("tianyi");
    t["mengjin"] = tr("mengjin");
    t["luanji"] = tr("luanji");
    t["xueyi"] = tr("xueyi");

    t["qinyin"] = tr("qinyin");
    t["yeyan"] = tr("yeyan");
    t["qixing"] = tr("qixing");
    t["kuangfeng"] = tr("kuangfeng");
    t["dawu"] = tr("dawu");

    t[":quhu"] = tr(":quhu");
    t[":jieming"] = tr(":jieming");
    t[":qiangxi"] = tr(":qiangxi");
    t[":lianhuan"] = tr(":lianhuan");
    t[":niepan"] = tr(":niepan");
    t[":bazhen"] = tr(":bazhen");
    t[":huoji"] = tr(":huoji");
    t[":kanpo"] = tr(":kanpo");
    t[":tianyi"] = tr(":tianyi");
    t[":mengjin"] = tr(":mengjin");
    t[":luanji"] = tr(":luanji");    
    t[":xueyi"] = tr(":xueyi");
    t[":shuangxiong"] = tr(":shuangxiong");

    // descriptive texts
    t["jieming:yes"] = tr("jieming:yes");
    t["shuangxiong:yes"] = tr("shuangxiong:yes");
    t["mengjin:yes"] = tr("mengjin:yes");
    t["niepan:yes"] = tr("niepan:yes");
    t["bazhen:yes"] = tr("bazhen:yes");

    t["@jieming"] = tr("@jieming");

    addMetaObject<QuhuCard>();
    addMetaObject<JiemingCard>();
    addMetaObject<QiangxiCard>();
    addMetaObject<TianyiCard>();
}

ADD_PACKAGE(Fire);
