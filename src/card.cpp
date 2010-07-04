#include "card.h"
#include "settings.h"

Card::Card(CardClass *card_class, enum Suit suit, int number, int id)
    :card_class(card_class), suit(suit), number(number), id(id)
{
    Q_ASSERT(card_class != NULL);
    if(number < 1 || number > 13)
        number = 0;

    setObjectName(card_class->objectName());
}

QString Card::getSuitString() const{
    switch(suit){
    case Spade: return "spade";
    case Heart: return "heart";
    case Club: return "club";
    case Diamond: return "diamond";
    default: return "no_suit";
    }
}

bool Card::isRed() const{
    return suit == Heart || suit == Diamond;
}

bool Card::isBlack() const{
    return suit == Spade || suit == Club;
}

int Card::getNumber() const{
    return number;
}

QString Card::getNumberString() const{
    if(number == 10)
        return "10";
    else{
        static const char *number_string = "-A23456789-JQK";
        return QString(number_string[number]);
    }
}

QString Card::getType() const{
    return card_class->type;
}

Card::Suit Card::getSuit() const{
    return suit;
}

QString Card::getPixmapPath() const{
    return card_class->getPixmapPath();
}

bool Card::CompareBySuitNumber(const Card *a, const Card *b){
    if(a->suit != b->suit)
        return a->suit < b->suit;
    else
        return a->number < b->number;
}

bool Card::CompareByType(const Card *a, const Card *b){
    static QMap<QString,int> typemap;
    if(typemap.isEmpty()){
        typemap["basic"] = 0;
        typemap["equip"] = 1;
        typemap["trick"] = 2;
    }

    int order1 = typemap[a->card_class->type] * 10000 + a->card_class->id;
    int order2 = typemap[b->card_class->type] * 10000 + b->card_class->id;
    if(order1 != order2)
        return order1 < order2;
    else
        return CompareBySuitNumber(a,b);
}

