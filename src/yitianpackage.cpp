#include "yitianpackage.h"
#include "skill.h"
#include "engine.h"
#include "client.h"

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
            QList<ServerPlayer *> targets;
            if(room->askForCardWithTargets(player, "@@yitian", "@yitian-sword", targets)){
                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << targets;

                QVariant use_data = QVariant::fromValue(use);
                room->getThread()->trigger(CardUsed, player, use_data);
            }
        }

        return false;
    }
};

class YitianSwordViewAsSkill: public ZeroCardViewAsSkill{
public:
    YitianSwordViewAsSkill():ZeroCardViewAsSkill("yitian_sword"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@yitian";
    }

    virtual const Card *viewAs() const{
        return new Slash(Card::NoSuit, 0);
    }
};

class YitianSword:public Weapon{
public:
    YitianSword(Suit suit = Spade, int number = 6):Weapon(suit, number, 2){
        setObjectName("yitian_sword");
        skill = new YitianSwordSkill;
        attach_skill = true;
    }
};

YitianPackage::YitianPackage()
    :Package("yitian")
{
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 1)
            << new Shit(Card::Diamond, 1)
            << new YitianSword;

    foreach(Card *card, cards)
        card->setParent(this);

    t["yitian"] = tr("yitian");
    t["shit"] = tr("shit");
    t["yitian_sword"] = tr("yitian_sword");

    skills << new YitianSwordViewAsSkill;
}

ADD_PACKAGE(Yitian);
