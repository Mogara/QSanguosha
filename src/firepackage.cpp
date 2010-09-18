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

    // FIXME: if nobody in his attack range, also return false

    return true;
}

void QuhuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *xunyu = effect.from;
    Room *room = xunyu->getRoom();

    bool success = room->pindian(xunyu, effect.to);
    if(success){

    }else{
        DamageStruct damage;
        damage.card = this;
        damage.from = effect.to;
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
    return targets.isEmpty();
}

void JiemingCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.to->getMaxHP() - effect.to->getHandcardNum();
    if(x <= 0)
        return;

    x = qMin(5, x);
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
            room->askForUseCard(xunyu, "@@jieming", "@jieming");
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
        return new QuhuCard();
    }
};

QiangxiCard::QiangxiCard(){

}

bool QiangxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && Self->inMyAttackRange(to_select);
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
            Card::Suit suit = first->isRed() ? Card::Heart : Card::Spade;
            ArcheryAttack *aa = new ArcheryAttack(suit, 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        }else
            return NULL;
    }
};

class ShuangxiongViewAsSkill: public ViewAsSkill{
public:
    ShuangxiongViewAsSkill():ViewAsSkill("shuangxiong"){        
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("shuangxiong") != 0;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        int value = Self->getMark("shuangxiong");
        if(value == 1)
            return to_select->getCard()->isBlack();
        else if(value == 2)
            return to_select->getCard()->isRed();

        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        const Card *card = cards.first()->getCard();
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
                int card_id = room->getJudgeCard(target);
                const Card *card = Sanguosha->getCard(card_id);
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

class Lianhuan: public ViewAsSkill{
public:
    Lianhuan():ViewAsSkill("lianhuan"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *card = cards.first()->getCard();
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
        chain->addSubcards(cards);
        chain->setSkillName(objectName());
        return chain;
    }
};

class Huoji: public ViewAsSkill{
public:
    Huoji():ViewAsSkill("huoji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *card = cards.first()->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcards(cards);
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Qinyin: public TriggerSkill{
public:
    Qinyin():TriggerSkill("qinyin"){
        events << CardDiscarded;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        int discard_num = data.toInt();

        if(player->getPhase() == Player::Discard && discard_num >= 2){
            Room *room = player->getRoom();
            QString result = room->askForChoice(player, objectName(), "up+down+no");
            if(result == "no"){
                return false;
            }

            QList<ServerPlayer *> all_players = room->getAllPlayers();
            if(result == "up"){
                foreach(ServerPlayer *player, all_players){
                    room->recover(player);
                }
            }else if(result == "down"){
                foreach(ServerPlayer *player, all_players){
                    room->loseHp(player);
                }
            }
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

FirePackage::FirePackage()
    :Package("fire")
{
    General *xunyu, *dianwei, *wolong, *pangtong, *taishici, *yuanshao, *shuangxiong, *shenzhouyu, *shenzhugeliang;

    xunyu = new General(this, "xunyu", "wei", 3);
    xunyu->addSkill(new Quhu);
    xunyu->addSkill(new Jieming);

    dianwei = new General(this, "dianwei", "wei");
    dianwei->addSkill(new Qiangxi);

    wolong = new General(this, "wolong", "shu", 3);
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Skill("kanpo"));

    pangtong = new General(this, "pangtong", "shu", 3);
    pangtong->addSkill(new Lianhuan);

    taishici = new General(this, "taishici", "wu");
    taishici->addSkill(new Tianyi);

    yuanshao = new General(this, "yuanshao$", "qun");
    yuanshao->addSkill(new Luanji);    

    shuangxiong = new General(this, "shuangxiong", "qun");
    shuangxiong->addSkill(new Shuangxiong);

    shenzhouyu = new General(this, "shenzhouyu", "wu");
    shenzhouyu->addSkill(new Qinyin);

    shenzhugeliang = new General(this, "shenzhugeliang", "shu", 3);

    t["fire"] = tr("fire");
    t["xunyu"] = tr("xunyu");
    t["dianwei"] = tr("dianwei");
    t["wolong"] = tr("wolong");
    t["pangtong"] = tr("pangtong");
    t["taishici"] = tr("taishici");
    t["yuanshao"] = tr("yuanshao");
    t["shuangxiong"] = tr("shuangxiong");
    t["shenzhouyu"] = tr("shenzhouyu");
    t["shenzhugeliang"] = tr("shenzhugeliang");

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

    t[":qinyin"] = tr(":qinyin");
    t[":yeyan"] = tr(":yeyan");
    t[":qixing"] = tr(":qixing");
    t[":kuangfeng"] = tr(":kuangfeng");
    t[":dawu"] = tr(":dawu");

    // descriptive texts
    t["jieming:yes"] = tr("jieming:yes");

    t[":qinyin:"] = tr(":qinyin:");
    t["qinyin:up"] = tr("qinyin:up");
    t["qinyin:down"] = tr("qinyin:down");
    t["qinyin:no"] = tr("qinyin:no");

    addMetaObject<QuhuCard>();
    addMetaObject<JiemingCard>();
    addMetaObject<QiangxiCard>();
    addMetaObject<TianyiCard>();
}

ADD_PACKAGE(Fire);
