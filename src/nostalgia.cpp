#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "nostalgia.h"

class Hongyan: public FilterSkill{
public:
    Hongyan():FilterSkill("hongyan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *new_card = Card::Clone(card);
        if(new_card) {
            new_card->setSuit(Card::Heart);
            new_card->setSkillName(objectName());
            return new_card;
        }else
            return card;
    }
};

class HongyanRetrial: public TriggerSkill{
public:
    HongyanRetrial():TriggerSkill("#hongyan-retrial"){
        frequency = Compulsory;

        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->getSuit() == Card::Spade){
            LogMessage log;
            log.type = "#HongyanJudge";
            log.from = player;

            Card *new_card = Card::Clone(judge->card);
            new_card->setSuit(Card::Heart);
            new_card->setSkillName("hongyan");
            judge->card = new_card;

            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

TianxiangCard::TianxiangCard()
{
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = effect.from->tag["TianxiangDamage"].value<DamageStruct>();
    damage.to = effect.to;
    damage.chain = true;
    room->damage(damage);

    if(damage.to->isAlive())
        damage.to->drawCards(damage.to->getLostHp());
}

class TianxiangViewAsSkill: public OneCardViewAsSkill{
public:
    TianxiangViewAsSkill():OneCardViewAsSkill("tianxiang"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@tianxiang";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TianxiangCard *card = new TianxiangCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Tianxiang: public TriggerSkill{
public:
    Tianxiang():TriggerSkill("tianxiang"){
        events << Predamaged;

        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(!xiaoqiao->isKongcheng()){
            DamageStruct damage = data.value<DamageStruct>();
            Room *room = xiaoqiao->getRoom();

            xiaoqiao->tag["TianxiangDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(xiaoqiao, "@tianxiang", "@@tianxiang-card"))
                return true;
        }

        return false;
    }
};

class MoonSpearSkill: public WeaponSkill{
public:
    MoonSpearSkill():WeaponSkill("moon_spear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;


        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card == NULL || !card->isBlack())
            return false;

        Room *room = player->getRoom();
        room->askForUseCard(player, "slash", "@moon-spear-slash");

        return false;
    }
};

class MoonSpear: public Weapon{
public:
    MoonSpear(Suit suit = Card::Diamond, int number = 12)
        :Weapon(suit, number, 3){
        setObjectName("moon_spear");
        skill = new MoonSpearSkill;
    }
};

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){

                    player->setFlags("-jilei");
                    player->setFlags("-jileiB");
                    player->setFlags("-jileiE");
                    player->setFlags("-jileiT");

                    player->invoke("jilei");
                }
            }
        }

        return false;
    }
};


class Jilei: public TriggerSkill{
public:
    Jilei():TriggerSkill("jilei"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        Room *room = yangxiu->getRoom();
        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->playSkillEffect(objectName());

            QString jilei_flag = choice[0].toUpper();
            damage.from->invoke("jilei", jilei_flag);

            damage.from->setFlags("jilei");
            damage.from->setFlags("jilei" + jilei_flag);

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
        }

        return false;
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.multiple && effect.card->inherits("TrickCard")){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), data)){
                room->playSkillEffect(objectName());

                LogMessage log;

                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                player->drawCards(1);
                return true;
            }
        }

        return false;
    }
};

GuhuoCard::GuhuoCard(){
    mute = true;
}

bool GuhuoCard::guhuo(ServerPlayer *yuji) const{
    Room *room = yuji->getRoom();
    room->setTag("Guohuoing", true);

    room->moveCardTo(this, yuji, Player::Special, false);

    QList<ServerPlayer *> players = room->getOtherPlayers(yuji);
    QSet<ServerPlayer *> questioned;

    foreach(ServerPlayer *player, players){
        QString choice = room->askForChoice(player, "guhuo", "question+noquestion");
        if(choice == "question"){
            room->setEmotion(player, Room::Question);
            questioned << player;
        }else
            room->setEmotion(player, Room::NoQuestion);

        LogMessage log;
        log.type = "#GuhuoQuery";
        log.from = player;
        log.arg = choice;

        room->sendLog(log);
    }

    bool success = false;
    if(questioned.isEmpty()){
        success = true;

        foreach(ServerPlayer *player, players)
            room->setEmotion(player, Room::NoEmotion);

    }else{
        const Card *card = Sanguosha->getCard(subcards.first());
        bool real = card->objectName() == user_string;
        success = real && card->getSuit() == Card::Heart;

        foreach(ServerPlayer *player, players){
            room->setEmotion(player, Room::NoEmotion);

            if(questioned.contains(player)){
                if(real)
                    room->loseHp(player);
                else
                    player->drawCards(1);
            }
        }
    }

    LogMessage log;
    log.type = "$GuhuoResult";
    log.from = yuji;
    log.card_str = QString::number(subcards.first());
    room->sendLog(log);

    room->setTag("Guohuoing", false);

    if(!success)
        room->throwCard(this);

    return success;
}

bool GuhuoCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    CardStar card = Self->tag.value("Guhuo").value<CardStar>();
    return card && card->targetFilter(targets, to_select);
}

bool GuhuoCard::targetFixed() const{
    if(ClientInstance->getStatus() == Client::Responsing)
        return true;

    CardStar card = Self->tag.value("Guhuo").value<CardStar>();
    return card && card->targetFixed();
}

bool GuhuoCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    CardStar card = Self->tag.value("Guhuo").value<CardStar>();
    return card && card->targetsFeasible(targets);
}

const Card *GuhuoCard::validate(const CardUseStruct *card_use) const{
    Room *room = card_use->from->getRoom();

    LogMessage log;
    log.type = card_use->to.isEmpty() ? "#GuhuoNoTarget" : "#Guhuo";
    log.from = card_use->from;
    log.to = card_use->to;
    log.arg = user_string;

    room->sendLog(log);

    if(guhuo(card_use->from)){
        const Card *card = Sanguosha->getCard(subcards.first());
        Card *use_card = Sanguosha->cloneCard(user_string, card->getSuit(), card->getNumber());
        use_card->addSubcard(this);

        return use_card;
    }else
        return NULL;
}

const Card *GuhuoCard::validateInResposing(ServerPlayer *user, bool *continuable) const{
    *continuable = true;

    Room *room = user->getRoom();

    LogMessage log;
    log.type = "#GuhuoNoTarget";
    log.from = user;
    log.arg = user_string;
    room->sendLog(log);

    if(guhuo(user)){
        const Card *card = Sanguosha->getCard(subcards.first());
        Card *use_card = Sanguosha->cloneCard(user_string, card->getSuit(), card->getNumber());
        use_card->addSubcard(this);

        return use_card;
    }else
        return NULL;
}

class GuhuoViewAsSkill: public OneCardViewAsSkill{
public:
    GuhuoViewAsSkill():OneCardViewAsSkill("guhuo"){
    }

    virtual bool isEnabledAtResponse() const{
        return !Self->isKongcheng()
                && !ClientInstance->card_pattern.startsWith("@")
                && !ClientInstance->card_pattern.contains("+");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(ClientInstance->getStatus() == Client::Responsing){
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(ClientInstance->card_pattern);
            card->addSubcard(card_item->getFilteredCard());
            return card;
        }

        CardStar c = Self->tag.value("Guhuo").value<CardStar>();
        if(c){
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(c->objectName());
            card->addSubcard(card_item->getFilteredCard());

            return card;
        }else
            return NULL;
    }
};

class Guhuo: public TriggerSkill{
public:
    Guhuo():TriggerSkill("guhuo"){
        default_choice = "noquestion";
        view_as_skill = new GuhuoViewAsSkill;

        events << AskForPeaches;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *, QVariant &) const{
        return false;
    }
};

NostalgiaPackage::NostalgiaPackage()
    :Package("nostalgia")
{
    General *xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false);
    xiaoqiao->addSkill(new Hongyan);
    xiaoqiao->addSkill(new HongyanRetrial);
    xiaoqiao->addSkill(new Tianxiang);

    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);

    General *yuji = new General(this, "yuji", "qun", 3);
    yuji->addSkill(new Guhuo);

    Card *moon_spear = new MoonSpear;
    moon_spear->setParent(this);

    addMetaObject<TianxiangCard>();
    addMetaObject<GuhuoCard>();
}

ADD_PACKAGE(Nostalgia);
