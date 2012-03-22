#include "yjcm2012-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "god.h"
#include "maneuvering.h"

ZhenlieCard::ZhenlieCard(){
    target_fixed = true;
    will_throw = false;
}

void ZhenlieCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->showCard(source, getSubcards().first());
}

class ZhenlieViewAsSkill:public OneCardViewAsSkill{
public:
    ZhenlieViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@zhenlie";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("zhenliecard");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() != card->getSuit();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhenlieCard *card = new ZhenlieCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Zhenlie: public TriggerSkill{
public:
    Zhenlie():TriggerSkill("zhenlie"){
        events << AskForRetrial;
        view_as_skill = new ZhenlieViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;
        return !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@zhenlie-card" << judge->who->objectName()
                << ":" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        room->setPlayerMark(player, "zhenliecard", judge->card->getEffectiveId());
        const Card *card = room->askForCard(player, "@zhenlie", prompt, data);

        if(card){
            int card_id = room->drawCard();
            room->getThread()->delay();

            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }
        return false;
    }
};

class Miji: public PhaseChangeSkill{
public:
    Miji():PhaseChangeSkill("miji"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *wangyi) const{
        if(!wangyi->isWounded())
            return false;
        if(wangyi->getPhase() == Player::Start || wangyi->getPhase() == Player::Finish){
            if(!wangyi->askForSkillInvoke(objectName()))
                return false;
            Room *room = wangyi->getRoom();
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wangyi;

            room->judge(judge);

            if(judge.isGood()){
                room->playSkillEffect(objectName());
                int x = wangyi->getLostHp();
                wangyi->drawCards(x);
                ServerPlayer *target = room->askForPlayerChosen(wangyi, room->getAllPlayers(), objectName());

                QList<const Card *> miji_cards = wangyi->getHandcards().mid(wangyi->getHandcardNum() - x);
                foreach(const Card *card, miji_cards)
                    room->moveCardTo(card, target, Player::Hand, false);
            }
        }
        return false;
    }
};

class Zhiyu: public MasochismSkill{
public:
    Zhiyu():MasochismSkill("zhiyu"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if(target->askForSkillInvoke(objectName(), QVariant::fromValue(damage))){
            target->drawCards(1);

            Room *room = target->getRoom();
            room->showAllCards(target);

            QList<const Card *> cards = target->getHandcards();
            Card::Color color = cards.first()->getColor();
            bool same_color = true;
            foreach(const Card *card, cards){
                if(card->getColor() != color){
                    same_color = false;
                    break;
                }
            }

            if(same_color && damage.from){
                DamageStruct zhiyu_damage;
                zhiyu_damage.from = target;
                zhiyu_damage.to = damage.from;

                room->damage(zhiyu_damage);
            }
        }
    }
};

class Fuji: public TriggerSkill{
public:
    Fuji():TriggerSkill("fuji"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

        if(player->distanceTo(damage.to) == 1 && damage.card && damage.card->inherits("Slash")){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->loseMaxHp(damage.to, damage.damage);
            return true;
        }
        return false;
    }
};

class Dangxian: public TriggerSkill{
public:
    Dangxian():TriggerSkill("dangxian"){
        events << TurnStart;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        QList<Player::Phase> phases;
        phases << Player::Play;
        player->play(phases);
        return false;
    }
};

class Fuli: public TriggerSkill{
public:
    Fuli():TriggerSkill("fuli"){
        events << Dying;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@laoji") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liaohua, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != liaohua)
            return false;

        Room *room = liaohua->getRoom();
        if(liaohua->askForSkillInvoke(objectName(), data)){
            //room->broadcastInvoke("animate", "lightbox:$fuli");
            room->playSkillEffect(objectName());

            liaohua->loseMark("@laoji");
            int x = qMin(5, room->getAlivePlayers().count());
            RecoverStruct rev;
            rev.recover = x;
            room->recover(liaohua, rev);
            liaohua->turnOver();
        }
        return false;
    }
};

class Fuhun: public PhaseChangeSkill{
public:
    Fuhun():PhaseChangeSkill("fuhun"){

    }

    virtual bool onPhaseChange(ServerPlayer *shuangying) const{
        Room *room = shuangying->getRoom();
        if(shuangying->getPhase() == Player::Start){
            if(shuangying->hasSkill("wusheng"))
                shuangying->setFlags("has_wusheng");
            if(shuangying->hasSkill("paoxiao"))
                shuangying->setFlags("has_paoxiao");
        }
        else if(shuangying->getPhase() == Player::Draw){
            if(!shuangying->askForSkillInvoke(objectName()))
                return false;

            room->playSkillEffect(objectName());

            const Card *first = NULL;
            foreach(int card_id, room->getNCards(2)){
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                room->getThread()->delay();
                const Card *card = Sanguosha->getCard(card_id);
                room->obtainCard(shuangying, card);
                if(first == NULL)
                    first = card;
                else{
                    if(first->isRed() != card->isRed()){
                        room->acquireSkill(shuangying, "wusheng");
                        room->acquireSkill(shuangying, "paoxiao");
                        shuangying->setFlags("fuhun");
                    }
                }
            }

            return true;
        }
        else if(shuangying->getPhase() == Player::NotActive && shuangying->hasFlag("fuhun")){
            if(!shuangying->hasFlag("has_wusheng"))
                room->detachSkillFromPlayer(shuangying, "wusheng");
            if(!shuangying->hasFlag("has_paoxiao"))
                room->detachSkillFromPlayer(shuangying, "paoxiao");
        }
        return false;
    }
};

class Zishou:public DrawCardsSkill{
public:
    Zishou():DrawCardsSkill("zishou"){
    }

    virtual int getDrawNum(ServerPlayer *liubiao, int n) const{
        Room *room = liubiao->getRoom();
        if(room->askForSkillInvoke(liubiao, objectName())){
            room->playSkillEffect(objectName());
            liubiao->skip(Player::Play);
            return n + liubiao->getLostHp();
        }else
            return n;
    }
};

class Shiyong: public TriggerSkill{
public:
    Shiyong():TriggerSkill("shiyong"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.card->isRed()){
            Room *room = player->getRoom();
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->loseMaxHp(player);
        }
        return false;
    }
};

class Gongqi : public OneCardViewAsSkill{
public:
    Gongqi():OneCardViewAsSkill("gongqi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getTypeId() == Card::Equip;
    }

    const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        WushenSlash *slash = new WushenSlash(card->getSuit(), card->getNumber());
        slash->addSubcard(card);
        return slash;
    }
};

class Jiefan : public TriggerSkill{
public:
    Jiefan():TriggerSkill("jiefan"){
        events << Dying << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        ServerPlayer *handang = room->findPlayerBySkillName(objectName());

        if(event == Dying){
            if(!handang || !room->askForSkillInvoke(handang, objectName()))
                return false;

            DyingStruct dying = data.value<DyingStruct>();
            const Card *slash = room->askForCard(handang, "slash", "jiefan-slash:" + dying.damage->from->objectName(), data);
            room->setTag("JiefanTarget", data);
            if(slash){
                CardUseStruct use;
                use.card = slash;
                use.from = handang;
                use.to << dying.damage->from;
                room->useCard(use);
            }
        }
        else{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!player->hasSkill(objectName())
               || room->getTag("JiefanTarget").isNull()
               || !room->askForSkillInvoke(handang, objectName()))
                return false;

            DyingStruct dying = room->getTag("JiefanTarget").value<DyingStruct>();
            ServerPlayer *target = dying.who;
            room->removeTag("JiefanTarget");
            Peach *peach = new Peach(effect.slash->getSuit(), effect.slash->getNumber());
            peach->setSkillName(objectName());
            CardUseStruct use;
            use.card = peach;
            use.from = handang;
            use.to << target;
            room->useCard(use);

            return true;
        }

        return false;
    }
};

AnxuCard::AnxuCard(){
    once = true;
}

bool AnxuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    else if(targets.length() == 1)
        return to_select->getHandcardNum() != targets.first()->getHandcardNum();
    else
        return false;
}

bool AnxuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void AnxuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> selecteds = targets;
    ServerPlayer *from = selecteds.first()->getHandcardNum() < selecteds.last()->getHandcardNum() ? selecteds.takeFirst() : selecteds.takeLast();
    ServerPlayer *to = selecteds.takeFirst();
    int id = room->askForCardChosen(from, to, "h", "anxu");
    const Card *cd = Sanguosha->getCard(id);
    room->moveCardTo(cd, from, Player::Hand, true);
    room->showCard(from, id);
    if(cd->getSuit() != Card::Spade){
        if(!source->isWounded())
            source->drawCards(1);
        else{
            if(room->askForChoice(source, "anxu", "draw+recover") == "draw")
                source->drawCards(1);
            else{
                RecoverStruct recover;
                recover.card = this;
                recover.who = source;
                recover.recover = 1;
                room->recover(source, recover, true);
            }
        }
    }
}

class Anxu: public ZeroCardViewAsSkill{
public:
    Anxu():ZeroCardViewAsSkill("anxu"){
    }

    virtual const Card *viewAs() const{
        return new AnxuCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("AnxuCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class Zhuiyi: public TriggerSkill{
public:
    Zhuiyi():TriggerSkill("zhuiyi"){
        events << Death ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        int n = 0;
        foreach(const Card *cd, player->getCards("he")){
            if(cd->getSuit() != Card::Spade)
                n++;
        }
        if(n == 0)
            return false;
        DamageStar damage = data.value<DamageStar>();
        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(damage->from), objectName());

        LogMessage log;
        log.type = "#ZhuiyiLog";
        log.from = player;
        log.arg = QString::number(n);
        room->sendLog(log);
        target->drawCards(n);
        return false;
    }
 };

class LihuoViewAsSkill:public OneCardViewAsSkill{
public:
    LihuoViewAsSkill():OneCardViewAsSkill("lihuo"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *acard = new FireSlash(card->getSuit(), card->getNumber());
        acard->addSubcard(card->getId());
        acard->setSkillName(objectName());
        return acard;
    }
};

class Lihuo: public TriggerSkill{
public:
    Lihuo():TriggerSkill("lihuo"){
        events << CardUsed ;
        view_as_skill = new LihuoViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getSkillName() == objectName())
            room->loseHp(player, 1);
        return false;
    }
 };

ChunlaoCard::ChunlaoCard(){
    will_throw = false;
    target_fixed = true;
}

void ChunlaoCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->addToPile("ChunlaoPile", this->subcards.first(), true);
}

class ChunlaoViewAsSkill:public OneCardViewAsSkill{
public:
    ChunlaoViewAsSkill():OneCardViewAsSkill("chunlao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@chunlao-slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *acard = new ChunlaoCard;
        acard->addSubcard(card->getId());
        //acard->setSkillName(objectName());
        return acard;
    }
};

class Chunlao: public TriggerSkill{
public:
    Chunlao():TriggerSkill("chunlao"){
        events << PhaseChange << Dying ;
        view_as_skill = new ChunlaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *chengpu = room->findPlayerBySkillName(objectName());
        if(event == PhaseChange && chengpu->getPhase() == Player::Finish && !chengpu->isKongcheng()){
            //if(chengpu->askForSkillInvoke(objectName(), data))
                room->askForUseCard(chengpu, "@@chunlao-slash", "@chunlao");
        }else if(event == Dying && !chengpu->getPile("ChunlaoPile").isEmpty()){
            DyingStruct dying = data.value<DyingStruct>();
            if(chengpu->askForSkillInvoke(objectName(), data)){
                room->throwCard(chengpu->getPile("ChunlaoPile").first());
                Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
                analeptic->setSkillName(objectName());
                CardUseStruct use;
                use.card = analeptic;
                use.from = dying.who;
                use.to << dying.who;
                room->useCard(use);
            }
        }
        return false;
    }
 };

YJCM2012Package::YJCM2012Package():Package("YJCM2012"){

    General *wangyi = new General(this, "wangyi", "wei", 3, false);
    wangyi->addSkill(new Zhenlie);
    wangyi->addSkill(new Miji);

    General *xunyou = new General(this, "xunyou", "wei", 3);
    xunyou->addSkill(new Zhiyu);

    General *caozhang = new General(this, "caozhang", "wei");

    General *madai = new General(this, "madai", "shu");
    madai->addSkill(new Fuji);

    General *liaohua = new General(this, "liaohua", "shu");
    liaohua->addSkill(new Dangxian);
    liaohua->addSkill(new MarkAssignSkill("@laoji", 1));
    liaohua->addSkill(new Fuli);

    General *guanxingzhangbao = new General(this, "guanxingzhangbao", "shu");
    guanxingzhangbao->addSkill(new Fuhun);

    General *chengpu = new General(this, "chengpu", "wu");
    chengpu->addSkill(new Lihuo);
    chengpu->addSkill(new Chunlao);

    General *bulianshi = new General(this, "bulianshi", "wu", 3, false);
    bulianshi->addSkill(new Anxu);
    bulianshi->addSkill(new Zhuiyi);

    General *handang = new General(this, "handang", "qun");
    handang->addSkill(new Gongqi);
    handang->addSkill(new Jiefan);

    General *liubiao = new General(this, "liubiao", "qun", 4);
    liubiao->addSkill(new Zishou);
    liubiao->addSkill(new Skill("zongshi", Skill::Compulsory));
	
    General *huaxiong = new General(this, "huaxiong", "qun", 6);
    huaxiong->addSkill(new Shiyong);

    addMetaObject<ZhenlieCard>();
    addMetaObject<ChunlaoCard>();
    addMetaObject<AnxuCard>();
}

ADD_PACKAGE(YJCM2012)
