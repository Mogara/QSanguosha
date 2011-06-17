#include "aux-skills.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"
#include "clientplayer.h"
#include "standard-skillcards.h"

DiscardSkill::DiscardSkill()
    :ViewAsSkill("discard"), card(new DummyCard),
    num(0), include_equip(false)
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

    if(ClientInstance->isJilei(to_select->getFilteredCard()))
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

ResponseSkill::ResponseSkill()
    :OneCardViewAsSkill("response-skill")
{

}

void ResponseSkill::setPattern(const QString &pattern){
    nameset.clear();
    suit = Card::NoSuit;

    if(pattern == ".")
        return;

    if(pattern == "slash")
        nameset << "slash" << "thunder_slash" << "fire_slash";
    else if(pattern == "peach+analeptic")
        nameset << "peach" << "analeptic";
    else if(pattern.startsWith(".")){
        char suit_char = pattern.toAscii().at(1);
        switch(suit_char){
        case 'S': suit = Card::Spade; break;
        case 'C': suit = Card::Club; break;
        case 'H': suit = Card::Heart; break;
        case 'D': suit = Card::Diamond; break;
        default:
            break;
        }
    }else
        nameset << pattern;
}

bool ResponseSkill::matchPattern(const Card *card) const{
    if(suit != Card::NoSuit && card->getSuit() != suit)
        return false;

    return nameset.isEmpty() || nameset.contains(card->objectName());
}

bool ResponseSkill::viewFilter(const CardItem *to_select) const{
    if(to_select->isEquipped())
        return false;

    const Card *card = to_select->getFilteredCard();
    return matchPattern(card);
}

const Card *ResponseSkill::viewAs(CardItem *card_item) const{
    return card_item->getFilteredCard();
}

// -------------------------------------------

FreeDiscardSkill::FreeDiscardSkill(QObject *parent)
    :ViewAsSkill("free-discard")
{
    setParent(parent);
    card = new DummyCard;
}

bool FreeDiscardSkill::isEnabledAtPlay(const Player *) const{
    return true;
}

bool FreeDiscardSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    return true;
}

const Card *FreeDiscardSkill::viewAs(const QList<CardItem *> &cards) const{
    if(!cards.isEmpty()){

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
    QStringList cards = card_str.split("+");
    ids = Card::StringsToIds(cards);
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
