#include "firepackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"

QuhuCard::QuhuCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool QuhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->getHp() <= Self->getHp())
        return false;

    if(to_select->isKongcheng())
        return false;

    return true;
}

void QuhuCard::use(Room *room, ServerPlayer *xunyu, QList<ServerPlayer *> &targets) const{
    ServerPlayer *tiger = targets.first();

    room->broadcastSkillInvoke("quhu");

    bool success = xunyu->pindian(tiger, "quhu", this);
    if(success){
        room->broadcastSkillInvoke("quhu");

        QList<ServerPlayer *> players = room->getOtherPlayers(tiger), wolves;
        foreach(ServerPlayer *player, players){
            if(tiger->inMyAttackRange(player))
                wolves << player;
        }

        if(wolves.isEmpty()){
            LogMessage log;
            log.type = "#QuhuNoWolf";
            log.from = xunyu;
            log.to << tiger;
            room->sendLog(log);

            return;
        }

        room->broadcastSkillInvoke("#tunlang");
        ServerPlayer *wolf = room->askForPlayerChosen(xunyu, wolves, "quhu");

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

JiemingCard::JiemingCard(){

}

bool JiemingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    int upper = qMin(5, to_select->getMaxHp());
    return to_select->getHandcardNum() < upper;
}

void JiemingCard::onEffect(const CardEffectStruct &effect) const{
    int upper = qMin(5, effect.to->getMaxHp());
    int x = upper - effect.to->getHandcardNum();
    if(x <= 0)
        return;

    effect.to->drawCards(x);
}

class JiemingViewAsSkill: public ZeroCardViewAsSkill{
public:
    JiemingViewAsSkill():ZeroCardViewAsSkill("jieming"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@jieming";
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

class Quhu: public OneCardViewAsSkill{
public:
    Quhu():OneCardViewAsSkill("quhu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QuhuCard *card = new QuhuCard;
        card->addSubcard(card);
        return card;
    }
};

QiangxiCard::QiangxiCard(){
    once = true;
}

bool QiangxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
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

class Qiangxi: public ViewAsSkill{
public:
    Qiangxi():ViewAsSkill("qiangxi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QiangxiCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty() && to_select->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
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

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first();
            return !to_select->isEquipped() && to_select->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first();
            ArcheryAttack *aa = new ArcheryAttack(first->getSuit(), 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        }else
            return NULL;
    }
};

class Xueyi: public MaxCardsSkill{
public:
    Xueyi():MaxCardsSkill("xueyi$"){
    }
    virtual int getExtra(const Player *target) const{
        int extra = 0;
        const Player *lord = NULL;
        if(target->isLord())
            lord = target;
        QList<const Player *> players = target->getSiblings();
        foreach(const Player *player, players){
            if(player->isAlive() && player->getKingdom() == "qun")
                extra += 2;
            if(player->isLord())
                lord = player;
        }
        if(target->hasLordSkill(objectName()))
            return extra;
        else
            return 0;
    }
};

class ShuangxiongViewAsSkill: public OneCardViewAsSkill{
public:
    ShuangxiongViewAsSkill():OneCardViewAsSkill("shuangxiong"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("shuangxiong") != 0;
    }

    virtual bool viewFilter(const Card *card) const{
        if(card->isEquipped())
            return false;

        int value = Self->getMark("shuangxiong");
        if(value == 1)
            return card->isBlack();
        else if(value == 2)
            return card->isRed();

        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Duel *duel = new Duel(originalCard->getSuit(), originalCard->getNumber());
        duel->addSubcard(originalCard);
        duel->setSkillName(objectName());
        return duel;
    }
};

class Shuangxiong: public TriggerSkill{
public:
    Shuangxiong():TriggerSkill("shuangxiong"){
        view_as_skill = new ShuangxiongViewAsSkill;

        events << EventPhaseStart << FinishJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *shuangxiong, QVariant &data) const{
        if(triggerEvent == EventPhaseStart){
            if(shuangxiong->getPhase() == Player::Start){
                room->setPlayerMark(shuangxiong, "shuangxiong", 0);
            }else if(shuangxiong->getPhase() == Player::Draw){
                if(shuangxiong->askForSkillInvoke(objectName())){
                    shuangxiong->setFlags("shuangxiong");

                    room->broadcastSkillInvoke("shuangxiong");
                    JudgeStruct judge;
                    judge.pattern = QRegExp("(.*)");
                    judge.good = true;
                    judge.reason = objectName();
                    judge.who = shuangxiong;

                    room->judge(judge);

                    room->setPlayerMark(shuangxiong, "shuangxiong", judge.card->isRed() ? 1 : 2);
                    shuangxiong->setFlags("-shuangxiong");

                    return true;
                }
            }
        }else if(triggerEvent == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "shuangxiong"){
                shuangxiong->obtainCard(judge->card);
                return true;
            }
        }

        return false;
    }
};

class Mengjin: public TriggerSkill{
public:
    Mengjin():TriggerSkill("mengjin"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *pangde, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude()){
            Room *room = pangde->getRoom();
            if(pangde->askForSkillInvoke(objectName(), data)){
                room->broadcastSkillInvoke(objectName());
                int to_throw = room->askForCardChosen(pangde, effect.to, "he", objectName());
                CardMoveReason reason(CardMoveReason::S_REASON_DISMANTLE, effect.to->objectName());
                reason.m_playerId = pangde->objectName();
                reason.m_targetId = effect.to->objectName();
                room->moveCardTo(Sanguosha->getCard(to_throw), NULL, NULL, Player::DiscardPile, reason);
            }
        }

        return false;
    }
};

class Lianhuan: public OneCardViewAsSkill{
public:
    Lianhuan():OneCardViewAsSkill("lianhuan"){
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped() && to_select->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        IronChain *chain = new IronChain(originalCard->getSuit(), originalCard->getNumber());
        chain->addSubcard(originalCard);
        chain->setSkillName(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill{
public:
    Niepan():TriggerSkill("niepan"){
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *pangtong, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != pangtong)
            return false;

        if(pangtong->askForSkillInvoke(objectName(), data)){
            room->broadcastInvoke("animate", "lightbox:$niepan");
            room->broadcastSkillInvoke(objectName());

            pangtong->loseMark("@nirvana");

            pangtong->throwAllCards();
            room->setPlayerProperty(pangtong, "hp", qMin(3, pangtong->getMaxHp()));
            pangtong->drawCards(3);

            if(pangtong->isChained()){
                if(dying_data.damage == NULL || dying_data.damage->nature == DamageStruct::Normal)
                    room->setPlayerProperty(pangtong, "chained", false);
            }
            if(!pangtong->faceUp())
                pangtong->turnOver();
        }

        return false;
    }
};

class Huoji: public OneCardViewAsSkill{
public:
    Huoji():OneCardViewAsSkill("huoji"){
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped() && to_select->isRed();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        
        FireAttack *fire_attack = new FireAttack(originalCard->getSuit(), originalCard->getNumber());
        fire_attack->addSubcard(originalCard->getId());
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
        return target != NULL && TriggerSkill::triggerable(target) && !target->getArmor()
                && !target->hasFlag("wuqian") && target->getMark("qinggang") == 0;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *wolong, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        if(wolong->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wolong;
            judge.play_animation = true;

            room->setEmotion(wolong, "armor/eight_diagram");
            room->judge(judge);

            if(judge.isGood()){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class Kanpo: public OneCardViewAsSkill{
public:
    Kanpo():OneCardViewAsSkill("kanpo"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isBlack() && !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "nullification";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        const Card *first = originalCard;
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("kanpo");

        return ncard;
    }
};

TianyiCard::TianyiCard(){
    once = true;
    will_throw = false;
}

bool TianyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TianyiCard::use(Room *room, ServerPlayer *taishici, QList<ServerPlayer *> &targets) const{
    bool success = taishici->pindian(targets.first(), "tianyi", this);
    if(success){
        room->setPlayerFlag(taishici, "tianyi_success");
    }else{
        room->setPlayerFlag(taishici, "tianyi_failed");
    }
}

class TianyiViewAsSkill: public OneCardViewAsSkill{
public:
    TianyiViewAsSkill():OneCardViewAsSkill("tianyi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TianyiCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new TianyiCard;
        card->addSubcard(card);
        return card;
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
            if(target->hasFlag("tianyi_success")){
                room->setPlayerFlag(target, "-tianyi_success");
            }
        }

        return false;
    }
};

FirePackage::FirePackage()
    :Package("fire")
{
    General *dianwei, *xunyu, *pangtong, *wolong, *taishici, *yuanshao, *yanliangwenchou, *pangde;

    dianwei = new General(this, "dianwei", "wei");
    dianwei->addSkill(new Qiangxi);

    xunyu = new General(this, "xunyu", "wei", 3);
    xunyu->addSkill(new Quhu);
    xunyu->addSkill(new Jieming);

    pangtong = new General(this, "pangtong", "shu", 3);
    pangtong->addSkill(new Lianhuan);
    pangtong->addSkill(new MarkAssignSkill("@nirvana", 1));
    pangtong->addSkill(new Niepan);
    related_skills.insertMulti("niepan", "#@nirvana-1");

    wolong = new General(this, "wolong", "shu", 3);
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Kanpo);
    wolong->addSkill(new Bazhen);

    taishici = new General(this, "taishici", "wu");
    taishici->addSkill(new Tianyi);

    yuanshao = new General(this, "yuanshao$", "qun");
    yuanshao->addSkill(new Luanji);
    yuanshao->addSkill(new Xueyi);

    yanliangwenchou = new General(this, "yanliangwenchou", "qun");
    yanliangwenchou->addSkill(new Shuangxiong);

    pangde = new General(this, "pangde", "qun");
    pangde->addSkill(new Mengjin);
    pangde->addSkill("mashu");
    pangde->addSkill(new SPConvertSkill("pangde_guiwei", "pangde", "sp_pangde"));

    addMetaObject<QuhuCard>();
    addMetaObject<JiemingCard>();
    addMetaObject<QiangxiCard>();
    addMetaObject<TianyiCard>();
}

ADD_PACKAGE(Fire);
