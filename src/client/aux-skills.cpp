#include "aux-skills.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"
#include "clientplayer.h"
#include "standard-skillcards.h"
#include "engine.h"

DiscardSkill::DiscardSkill()
    :ViewAsSkill("discard"), card(new DummyCard),
    num(0), include_equip(false)
{
    card->setParent(this);
}

void DiscardSkill::setNum(int num){
    this->num = num;
	this->minnum = num;
}

void DiscardSkill::setMinNum(int minnum){
    this->minnum = minnum;
}

void DiscardSkill::setIncludeEquip(bool include_equip){
    this->include_equip = include_equip;
}

bool DiscardSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    if(selected.length() >= num)
        return false;

    if(!include_equip && to_select->isEquipped())
        return false;

    if(Self->isJilei(to_select->getFilteredCard()))
        return false;

    return true;
}

const Card *DiscardSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.length() >= minnum){
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
    this->pattern = Sanguosha->getPattern(pattern);
}

bool ResponseSkill::matchPattern(const Player *player, const Card *card) const{
    if(player->isJilei(card))
        return false;

    return pattern && pattern->match(player, card);
}

bool ResponseSkill::viewFilter(const CardItem *to_select) const{
    const Card *card = to_select->getFilteredCard();
    return matchPattern(Self, card);
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

bool FreeDiscardSkill::viewFilter(const QList<CardItem *> &, const CardItem *) const{
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

bool YijiViewAsSkill::viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
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

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
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
