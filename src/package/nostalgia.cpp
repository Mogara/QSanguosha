#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "nostalgia.h"
#include "yjcm-package.h"
#include "settings.h"

class MoonSpearSkill: public WeaponSkill{
public:
    MoonSpearSkill():WeaponSkill("MoonSpear"){
        events << CardFinished << CardResponded;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(triggerEvent == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;

            if(card == player->tag["MoonSpearSlash"].value<CardStar>()){
                card = NULL;
            }
        }else if(triggerEvent == CardResponded){
            card = data.value<ResponsedStruct>().m_card;
            player->tag["MoonSpearSlash"] = data;
        }

        if(card == NULL || !card->isBlack())
            return false;

        //@todo: askForUseCard combines asking and using the card.
        //animating weapon effect should happen in-between.
        //we should come back after the askFor methods are restructured.
        room->askForUseCard(player, "slash", "@moon-spear-slash");

        return false;
    }
};

MoonSpear::MoonSpear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("MoonSpear");
    skill = new MoonSpearSkill;
}

NostalgiaPackage::NostalgiaPackage()
    :Package("nostalgia")
{
    type = CardPack;

    Card *moon_spear = new MoonSpear;
    moon_spear->setParent(this);
}

// old yjcm's generals

class NosWuyan: public TriggerSkill{
public:
    NosWuyan():TriggerSkill("noswuyan"){
        events << CardEffect << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.to == effect.from)
            return false;

        if(effect.card->getTypeId() == Card::TypeTrick){

            if((effect.from && effect.from->hasSkill(objectName()))){
                LogMessage log;
                log.type = "#WuyanBaD";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();

                room->sendLog(log);

                room->broadcastSkillInvoke("wuyan");

                return true;
            }

            if(effect.to->hasSkill(objectName()) && effect.from){
                LogMessage log;
                log.type = "#WuyanGooD";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();

                room->sendLog(log);

                room->broadcastSkillInvoke("wuyan");

                return true;
            }
        }

        return false;
    }
};

NosJujianCard::NosJujianCard(){
    mute = true;
}

void NosJujianCard::onEffect(const CardEffectStruct &effect) const{
    int n = subcardsLength();
    effect.to->drawCards(n);
    Room *room = effect.from->getRoom();
    room->broadcastSkillInvoke("jujian");

    if(n == 3){
        QSet<Card::CardType> types;

        foreach(int card_id, effect.card->getSubcards()){
            const Card *card = Sanguosha->getCard(card_id);
            types << card->getTypeId();
        }

        if(types.size() == 1){

            LogMessage log;
            log.type = "#JujianRecover";
            log.from = effect.from;
            const Card *card = Sanguosha->getCard(subcards.first());
            log.arg = card->getType();
            room->sendLog(log);

            RecoverStruct recover;
            recover.card = this;
            recover.who = effect.from;
            room->recover(effect.from, recover);
        }
    }
}

class NosJujian: public ViewAsSkill{
public:
    NosJujian():ViewAsSkill("nosjujian"){

    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("NosJujianCard");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        NosJujianCard *card = new NosJujianCard;
        card->addSubcards(cards);
        return card;
    }
};

class NosEnyuan: public TriggerSkill{
public:
    NosEnyuan():TriggerSkill("nosenyuan"){
        events << HpRecover << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who != player){
                recover.who->drawCards(recover.recover);

                LogMessage log;
                log.type = "#EnyuanRecover";
                log.from = player;
                log.to << recover.who;
                log.arg = QString::number(recover.recover);
                log.arg2 = objectName();

                room->sendLog(log);

                room->broadcastSkillInvoke("enyuan", qrand() % 2 + 1);

            }
        }else if(triggerEvent == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *source = damage.from;
            if(source && source != player){
                room->broadcastSkillInvoke("enyuan", qrand() % 2 + 3);

                const Card *card = room->askForCard(source, ".|heart|.|hand", "@enyuanheart", QVariant(), Card::MethodNone);
                if(card){
                    player->obtainCard(card);
                }else{
                    room->loseHp(source);
                }
            }
        }

        return false;
    }
};

NosXuanhuoCard::NosXuanhuoCard(){
    will_throw = false;
    mute = true;
}

void NosXuanhuoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.from->getRoom();
    room->broadcastSkillInvoke("xuanhuo");
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "nosxuanhuo");
    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
    room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);

    QList<ServerPlayer *> targets = room->getOtherPlayers(effect.to);
    ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "nosxuanhuo");
    if(target != effect.from)
        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.from->objectName());
        reason.m_playerId = target->objectName();
        room->obtainCard(target, Sanguosha->getCard(card_id), reason, false);
}

class NosXuanhuo: public OneCardViewAsSkill{
public:
    NosXuanhuo():OneCardViewAsSkill("nosxuanhuo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("NosXuanhuoCard");
    }

    virtual bool viewFilter(const Card* to_select) const{
        return ! to_select->isEquipped() && to_select->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        NosXuanhuoCard *xuanhuoCard = new NosXuanhuoCard;
        xuanhuoCard->addSubcard(originalCard);
        return xuanhuoCard;
    }
};

class NosXuanfeng: public TriggerSkill{
public:
    NosXuanfeng():TriggerSkill("nosxuanfeng"){
        events << CardsMoveOneTime;
    }

    virtual QString getDefaultChoice(ServerPlayer *) const{
        return "nothing";
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *lingtong, QVariant &data) const{
        if(triggerEvent == CardsMoveOneTime){
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->from == lingtong && move->from_places.contains(Player::PlaceEquip))
            {

                QStringList choicelist;
                choicelist << "nothing";
                QList<ServerPlayer *> targets1;
                foreach(ServerPlayer *target, room->getAlivePlayers()){
                    if(lingtong->canSlash(target, NULL, false))
                        targets1 << target;
                }
                if (!targets1.isEmpty()) choicelist << "slash";
                QList<ServerPlayer *> targets2;
                foreach(ServerPlayer *p, room->getOtherPlayers(lingtong)){
                    if(lingtong->distanceTo(p) <= 1)
                        targets2 << p;
                }
                if (!targets2.isEmpty()) choicelist << "damage";

                QString choice;
                if (choicelist.length() == 1)
                    return false;
                else
                    choice = room->askForChoice(lingtong, objectName(), choicelist.join("+"));


                if(choice == "slash"){
                    ServerPlayer *target = room->askForPlayerChosen(lingtong, targets1, "xuanfeng-slash");

                    Slash *slash = new Slash(Card::NoSuit, 0);
                    slash->setSkillName("nosxuanfeng");

                    CardUseStruct card_use;
                    card_use.card = slash;
                    card_use.from = lingtong;
                    card_use.to << target;
                    room->useCard(card_use, false);
                }else if(choice == "damage"){
                    room->broadcastSkillInvoke("xuanfeng");

                    ServerPlayer *target = room->askForPlayerChosen(lingtong, targets2, "xuanfeng-damage");

                    DamageStruct damage;
                    damage.from = lingtong;
                    damage.to = target;
                    room->damage(damage);
                }
            }
        }

        return false;
    }
};

class NosShangshi: public Shangshi{
public:
    NosShangshi():Shangshi("nosshangshi", 998)
    {
        frequency = Frequent;
        events << PostHpReduced << HpLost << HpRecover << MaxHpChanged << EventPhaseChanging << EventAcquireSkill;
    }

    virtual int getPriority() const{
        return -1;
    }
};

class NosShangshiCardMove: public Shangshi{
public:
    NosShangshiCardMove():Shangshi("#nosshangshi", 998)
    {
        events << CardsMoveOneTime << CardDrawnDone;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual QString getEffectName() const{
        return "nosshangshi";
    }
};

class NosGongqi : public OneCardViewAsSkill{
public:
    NosGongqi():OneCardViewAsSkill("nosgongqi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getTypeId() == Card::TypeEquip;
    }

    const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard);
        slash->setSkillName(objectName());
        return slash;
    }
};

class NosGongqiTargetMod: public TargetModSkill {
public:
    NosGongqiTargetMod(): TargetModSkill("#nosgongqi-target") {
        frequency = NotFrequent;
    }

    virtual int getDistanceLimit(const Player *, const Card *card) const{
        if (card->getSkillName() == "nosgongqi")
            return 1000;
        else
            return 0;
    }
};

class NosJiefan : public TriggerSkill{
public:
    NosJiefan():TriggerSkill("nosjiefan"){
        events << AskForPeaches << DamageCaused << CardFinished << CardUsed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *handang, QVariant &data) const{
        if (handang == NULL) return false;

        ServerPlayer *current = room->getCurrent();
        if(!current || current->isDead())
            return false;
        if(triggerEvent == CardUsed)
        {
            if(!handang->hasFlag("nosjiefanUsed"))
                return false;

            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->isKindOf("Slash"))
            {
                room->broadcastSkillInvoke(objectName());
                room->setPlayerFlag(handang, "-nosjiefanUsed");
                room->setCardFlag(use.card, "nosjiefan-slash");
            }
        }else if(triggerEvent == AskForPeaches  && handang->getPhase() == Player::NotActive && handang->canSlash(current, NULL, false)
                && handang->askForSkillInvoke(objectName(), data)){
            DyingStruct dying = data.value<DyingStruct>();

            forever{
                if(handang->hasFlag("nosjiefan_success"))
                    room->setPlayerFlag(handang, "-nosjiefan_success");


                if(handang->hasFlag("nosjiefan_failed")){
                    room->setPlayerFlag(handang, "-nosjiefan_failed");
                    break;
                }

                if(dying.who->getHp() > 0 || handang->isNude() || current->isDead() || !handang->canSlash(current, NULL, false))
                    break;

                room->setPlayerFlag(handang, "nosjiefanUsed");
                room->setTag("JiefanTarget", data);
                if(!room->askForUseSlashTo(handang, current, "jiefan-slash:" + dying.who->objectName()))
                {
                    room->setPlayerFlag(handang, "-nosjiefanUsed");
                    room->removeTag("JiefanTarget");
                    break;
                }
            }
        }
        else if(triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->isKindOf("Slash") && damage.card->hasFlag("nosjiefan-slash")){

                DyingStruct dying = room->getTag("NosJiefanTarget").value<DyingStruct>();

                ServerPlayer *target = dying.who;
                if(target && target->getHp() > 0){
                    LogMessage log;
                    log.type = "#NosJiefanNull1";
                    log.from = dying.who;
                    room->sendLog(log);
                }
                else if(target && target->isDead()){
                    LogMessage log;
                    log.type = "#NosJiefanNull2";
                    log.from = dying.who;
                    log.to << handang;
                    room->sendLog(log);
                }
                else if(current->hasSkill("wansha") && current->isAlive()  && target->objectName() != handang->objectName()){
                    LogMessage log;
                    log.type = "#NosJiefanNull3";
                    log.from = current;
                    room->sendLog(log);
                }
                else
                {
                    Peach *peach = new Peach(damage.card->getSuit(), damage.card->getNumber());
                    peach->setSkillName(objectName());
                    CardUseStruct use;
                    use.card = peach;
                    use.from = handang;
                    use.to << target;

                    room->setPlayerFlag(handang, "nosjiefan_success");
                    if ((target->getGeneralName().contains("sunquan")
                        || target->getGeneralName().contains("sunce")
                        || target->getGeneralName().contains("sunjian"))
                        && target->isLord())
                        room->setPlayerFlag(handang, "NosJiefanToLord");
                    room->useCard(use);
                    room->setPlayerFlag(handang, "-NosJiefanToLord");
                }
                return true;
            }
            return false;
        }
        else if(triggerEvent == CardFinished && !room->getTag("NosJiefanTarget").isNull()){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->isKindOf("Slash")){
                if(!handang->hasFlag("nosjiefan_success"))
                    room->setPlayerFlag(handang, "nosjiefan_failed");
                room->removeTag("NosJiefanTarget");
            }
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const {
        if (player->hasFlag("NosJiefanToLord"))
            return 2;
        else
            return 1;
    }
};

NostalGeneralPackage::NostalGeneralPackage()
    :Package("nostal_general")
{
    General *nos_fazheng = new General(this, "nos_fazheng", "shu", 3);
    nos_fazheng->addSkill(new NosEnyuan);
    nos_fazheng->addSkill(new NosXuanhuo);

    General *nos_lingtong = new General(this, "nos_lingtong", "wu");
    nos_lingtong->addSkill(new NosXuanfeng);

    General *nos_xushu = new General(this, "nos_xushu", "shu", 3);
    nos_xushu->addSkill(new NosWuyan);
    nos_xushu->addSkill(new NosJujian);

    General *nos_zhangchunhua = new General(this, "nos_zhangchunhua", "wei", 3, false);
    nos_zhangchunhua->addSkill("jueqing");
    nos_zhangchunhua->addSkill(new NosShangshi);
    nos_zhangchunhua->addSkill(new NosShangshiCardMove);
    related_skills.insertMulti("nosshangshi", "#nosshangshi");

    General *nos_handang = new General(this, "nos_handang", "wu");
    nos_handang->addSkill(new NosGongqi);
    nos_handang->addSkill(new NosGongqiTargetMod);
    related_skills.insertMulti("nosgongqi", "#nosgongqi-target");
    nos_handang->addSkill(new NosJiefan);

    addMetaObject<NosXuanhuoCard>();
    addMetaObject<NosJujianCard>();
}

ADD_PACKAGE(Nostalgia)
ADD_PACKAGE(NostalGeneral)

