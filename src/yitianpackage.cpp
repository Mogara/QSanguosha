#include "yitianpackage.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = move.from;
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && move.to_place == Player::DiscardedPile){
        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;
        damage.damage = 1;
        damage.nature = DamageStruct::Normal;

        from->getRoom()->damage(damage);
    }
}

class YitianSwordSkill : public WeaponSkill{
public:
    YitianSwordSkill():WeaponSkill("yitian_sword"){
        events << CardGot;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardMoveStruct move = data.value<CardMoveStruct>();
        const Card *card = Sanguosha->getCard(move.card_id);
        Room *room = player->getRoom();
        if(room->getCurrent() != player && card->inherits("Slash") && move.to_place == Player::Hand){
            QString pattern = QString("@@yitian-%1").arg(move.card_id);
            room->askForUseCard(player, pattern, "@yitian-sword");
        }

        return false;
    }
};

class YitianSwordViewAsSkill: public ViewAsSkill{
public:
    YitianSwordViewAsSkill():ViewAsSkill("yitian_sword"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@yitian-");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        QString pattern = ClientInstance->card_pattern;
        pattern.remove("@@yitian-");
        int card_id = pattern.toInt();

        return to_select->getCard()->getId() == card_id;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        return cards.first()->getCard();
    }
};

class YitianSword:public Weapon{
public:
    YitianSword(Suit suit = Spade, int number = 6):Weapon(suit, number, 2){
        setObjectName("yitian_sword");
        skill = new YitianSwordSkill;
        attach_skill = true;
    }

    virtual void onMove(const CardMoveStruct &move) const{
        if(move.from_place == Player::Equip){
            Room *room = move.from->getRoom();

            bool invoke = room->askForSkillInvoke(move.from, objectName());
            if(!invoke)
                return;

            ServerPlayer *target = room->askForPlayerChosen(move.from, room->getAllPlayers());
            DamageStruct damage;
            damage.from = move.from;
            damage.to = target;
            damage.card = this;

            room->damage(damage);
        }
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

ChengxiangCard::ChengxiangCard()
{

}

bool ChengxiangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *) const{
    return targets.isEmpty();
}

void ChengxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->recover(effect.to);
    effect.to->drawCards(subcardsLength());
    effect.to->turnOver();
    room->broadcastProperty(effect.to, "faceup");
}

class ChengxiangViewAsSkill: public ViewAsSkill{
public:
    ChengxiangViewAsSkill():ViewAsSkill("chengxiang"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@chengxiang";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int sum = 0;
        foreach(CardItem *item, selected){
            sum += item->getCard()->getNumber();
        }

        sum += to_select->getCard()->getNumber();

        return sum <= Self->getMark("chengxiang");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int sum = 0;
        foreach(CardItem *item, cards){
            sum += item->getCard()->getNumber();
        }

        if(sum == Self->getMark("chengxiang")){
            ChengxiangCard *card = new ChengxiangCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Chengxiang: public MasochismSkill{
public:
    Chengxiang():MasochismSkill("chengxiang"){
        view_as_skill = new ChengxiangViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caochong, const DamageStruct &damage) const{
        const Card *card = damage.card;
        if(card == NULL)
            return;

        int point = card->getNumber();
        if(point == 0)
            return;

        Room *room = caochong->getRoom();
        room->setPlayerMark(caochong, objectName(), point);

        QString prompt = QString("@chengxiang-card:::%1").arg(point);
        room->askForUseCard(caochong, "@@chengxiang", prompt);
    }
};

class Conghui: public PhaseChangeSkill{
public:
    Conghui():PhaseChangeSkill("conghui"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *caochong) const{
        if(caochong->getPhase() == Player::Discard)
            return true;
        else
            return false;
    }
};

class Zaoyao: public PhaseChangeSkill{
public:
    Zaoyao():PhaseChangeSkill("zaoyao"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *caochong) const{
        if(caochong->getHandcardNum() > 13){
            caochong->throwAllCards();
            caochong->getRoom()->loseHp(caochong);
        }

        return false;
    }
};

YitianPackage::YitianPackage()
    :Package("yitian")
{
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 1)
            << new Shit(Card::Diamond, 1)
            << new YitianSword
            << new MoonSpear;

    foreach(Card *card, cards)
        card->setParent(this);

    t["yitian"] = tr("yitian");
    t["shit"] = tr("shit");
    t["yitian_sword"] = tr("yitian_sword");
    t["moon_spear"] = tr("moon_spear");

    // generals
    General *caochong = new General(this, "caochong", "wei", 3);
    caochong->addSkill(new Chengxiang);
    caochong->addSkill(new Conghui);
    caochong->addSkill(new Zaoyao);

    t["caochong"] = tr("caochong");

    t["chengxiang"] = tr("chengxiang");
    t["conghui"] = tr("conghui");
    t["zaoyao"] = tr("zaoyao");

    t[":chengxiang"] = tr(":chengxiang");
    t[":conghui"] = tr(":conghui");
    t[":zaoyao"] = tr(":zaoyao");

    t[":moon_spear"] = tr(":moon_spear");
    t[":yitian_sword"] = tr(":yitian_sword");
    t[":shit"] = tr(":shit");
    t["yitian_sword:yes"] = tr("yitian_sword:yes");

    t["@chengxiang-card"] = tr("@chengxiang-card");
    t["@moon-spear-slash"] = tr("@moon-spear-slash");

    skills << new YitianSwordViewAsSkill;

    addMetaObject<ChengxiangCard>();
}

ADD_PACKAGE(Yitian);
