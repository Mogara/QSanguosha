#include "card.h"
#include "settings.h"

Card::Card(enum Suit suit, int number)
    :suit(suit), number(number), id(-1)
{
    if(number < 1 || number > 13)
        number = 0;
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

int Card::getID() const{
    return id;
}

void Card::setID(int id){
    this->id = id;
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

Card::Suit Card::getSuit() const{
    return suit;
}

bool Card::match(const QString &pattern) const{
    return pattern.isEmpty() || objectName() == pattern ||
            getType() == pattern || getSubtype() == pattern;
}

bool Card::CompareBySuitNumber(const Card *a, const Card *b){
    if(a->suit != b->suit)
        return a->suit < b->suit;
    else
        return a->number < b->number;
}

bool Card::CompareByType(const Card *a, const Card *b){
    int order1 = a->getTypeId() * 10000 + a->id;
    int order2 = b->getTypeId() * 10000 + b->id;
    if(order1 != order2)
        return order1 < order2;
    else
        return CompareBySuitNumber(a,b);
}

QString Card::getPixmapPath() const{
    return QString("%1/cards/%2").arg(parent()->objectName()).arg(objectName());
}

QString Card::getPackage() const{
    if(parent())
        return parent()->objectName();
    else
        return "";
}

QString Card::getSubtype() const{
    return "";
}
