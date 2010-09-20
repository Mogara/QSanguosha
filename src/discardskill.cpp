#include "discardskill.h"
#include "carditem.h"

DiscardSkill::DiscardSkill()
    :ViewAsSkill("discard"), card(new DummyCard),
    num(0), include_equip(false), suit(Card::NoSuit)
{
    card->setParent(this);
}

void DiscardSkill::setNum(int num){
    this->num = num;
}

void DiscardSkill::setIncludeEquip(bool include_equip){
    this->include_equip = include_equip;
}

void DiscardSkill::setSuit(Card::Suit suit){
    this->suit = suit;
}

bool DiscardSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    if(selected.length() >= num)
        return false;

    if(!include_equip && to_select->isEquipped())
        return false;

    if(suit != Card::NoSuit && to_select->getCard()->getSuit() != suit)
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
