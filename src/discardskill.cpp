#include "discardskill.h"

class DiscardCard: public Card{
public:
    DiscardCard():Card(NoSuit, 0, true){}

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
    :ViewAsSkill("discard", true), card(new DiscardCard), num(0)
{
    card->setParent(this);
}

void DiscardSkill::setNum(int num){
    this->num = num;
}

bool DiscardSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    return true;
}

const Card *DiscardSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.length() == num)
        return card;
    else
        return NULL;
}
