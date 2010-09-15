#include "yijiviewasskill.h"
#include "carditem.h"

class YijiCard: public Card{
public:
    YijiCard():Card(Card::NoSuit, 0){
    }

    virtual QString getType() const{
        return "yiji_card";
    }

    virtual QString getSubtype() const{
        return "yiji_card";
    }

    virtual int getTypeId() const{
        return 0;
    }
};


YijiViewAsSkill::YijiViewAsSkill()
    :ViewAsSkill("yiji")
{
    card = new YijiCard;
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
