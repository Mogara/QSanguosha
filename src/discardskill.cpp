#include "discardskill.h"
#include "carditem.h"

class DiscardCard: public Card{
public:
    DiscardCard():Card(NoSuit, 0, true){
        target_fixed = true;
    }

    virtual QString getType() const{
        return "discard_card";
    }

    virtual QString getSubtype() const{
        return "discard_card";
    }

    virtual int getTypeId() const{
        return -1;
    }
};

DiscardSkill::DiscardSkill()
    :ViewAsSkill("discard"), card(new DiscardCard), num(0)
{
    card->setParent(this);
}

void DiscardSkill::setNum(int num){
    this->num = num;
}

void DiscardSkill::setIncludeEquip(bool include_equip){
    this->include_equip = include_equip;
}

bool DiscardSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    if(selected.length() >= num)
        return false;

    if(!include_equip && to_select->isEquipped())
        return false;

    return true;
}

const Card *DiscardSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.length() == num){
        card->clearSubcards();
        foreach(CardItem *card_item, cards)
            card->addSubcard(card_item->getCard()->getId());
        return card;
    }else
        return NULL;
}
