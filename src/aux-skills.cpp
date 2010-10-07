#include "aux-skills.h"

#include "carditem.h"
#include "standard.h"
#include "clientplayer.h"

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
        card->addSubcards(cards);
        return card;
    }else
        return NULL;
}

// -------------------------------------------

YijiViewAsSkill::YijiViewAsSkill()
    :ViewAsSkill("yiji")
{
    card = new RendeCard;
}

void YijiViewAsSkill::setCards(const QString &card_str){
    ids.clear();

    QStringList cards = card_str.split("+");
    foreach(QString card, cards)
        ids << card.toInt();
}

bool YijiViewAsSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    return ids.contains(to_select->getCard()->getId());
}

const Card *YijiViewAsSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.isEmpty())
        return NULL;

    card->clearSubcards();
    card->addSubcards(cards);

    return card;
}

// ------------------------------------------------

class ChoosePlayerCard: public DummyCard{
public:
    ChoosePlayerCard(){
        target_fixed = false;
    }

    void setPlayerNames(const QStringList &names){
        set = names.toSet();
    }

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
        return targets.isEmpty() && set.contains(to_select->objectName());
    }

private:
    QSet<QString> set;
};

ChoosePlayerSkill::ChoosePlayerSkill()
    :ZeroCardViewAsSkill("choose_player")
{
    card = new ChoosePlayerCard;
    card->setParent(this);
}

void ChoosePlayerSkill::setPlayerNames(const QStringList &names){
    card->setPlayerNames(names);
}

const Card *ChoosePlayerSkill::viewAs() const{
    return card;
}
