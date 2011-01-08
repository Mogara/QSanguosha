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

TianxiangCard::TianxiangCard()
{
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = room->getTag("TianxiangDamage").value<DamageStruct>();
    damage.to = effect.to;
    room->damage(damage);

    if(damage.to->isAlive() && damage.to->isWounded())
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

            room->setTag("TianxiangDamage", QVariant::fromValue(damage));
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

NostalgiaPackage::NostalgiaPackage()
    :Package("nostalgia")
{
    General *xiaoqiao;
    xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false);
    xiaoqiao->addSkill(new Hongyan);
    xiaoqiao->addSkill(new Tianxiang);

    Card *moon_spear = new MoonSpear;
    moon_spear->setParent(this);

    addMetaObject<TianxiangCard>();
}

ADD_PACKAGE(Nostalgia);
