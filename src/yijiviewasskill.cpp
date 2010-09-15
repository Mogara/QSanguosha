#include "yijiviewasskill.h"
#include "carditem.h"
#include "standard.h"


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
