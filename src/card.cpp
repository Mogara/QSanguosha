#include "card.h"
#include "settings.h"
#include "engine.h"

Card::Card(Suit suit, int number)
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

bool Card::isAvailable(const Client *) const{
    return true;
}

Event *Card::generate(Room *room){
    return NULL;
}

QString Card::toString() const{
    return QString("%1[%2 %3]").arg(objectName()).arg(getSuitString()).arg(getNumberString());
}

bool Card::isVirtualCard() const{
    return id >= 0;
}

const Card *Card::Parse(const QString &str){
    static QRegExp pattern("(\\w+)\\[(\\w+) (\\w+)\\]");
    pattern.indexIn(str);
    QStringList texts = pattern.capturedTexts();
    QString name = texts.at(1);
    QString suit_string = texts.at(2);
    QString number_string = texts.at(3);
    Suit suit = NoSuit;
    int number = 0;

    if(suit_string == "spade")
        suit = Spade;
    else if(suit_string == "club")
        suit = Club;
    else if(suit_string == "heart")
        suit = Heart;
    else if(suit_string == "diamond")
        suit = Diamond;    

    if(number_string == "A")
        number = 1;
    else if(number_string == "J")
        number = 11;
    else if(number_string == "Q")
        number = 12;
    else if(number_string == "K")
        number = 13;
    else
        number = number_string.toInt();

    return Sanguosha->cloneCard(name, suit, number);
}

bool Card::targetFixed(const Client *client) const{
    return false;
}

void Card::targetRange(const Client *, int *min, int *max, bool *include_self) const{
    *min = *max = 1;
    *include_self = false;
}

bool Card::targetFilter(const QList<const ClientPlayer *> &targets) const{
    return true;
}

void Card::use(Client *client, ClientPlayer *user, ClientPlayer *target) const{

}

void Card::use(Client *client, ClientPlayer *user, const QList<ClientPlayer *> &targets) const{
    foreach(ClientPlayer *target, targets){
        use(client, user, target);
    }
}

void Card::use(Room *room, ServerPlayer *user, ServerPlayer *target) const{

}

void Card::use(Room *room, ServerPlayer *user, const QList<ServerPlayer *> &targets) const{
    foreach(ServerPlayer *target, targets){
        use(room, user, target);
    }
}

void Card::addSubcard(const Card *card){
    if(card->isVirtualCard())
        qWarning(qPrintable(tr("Subcard must not be virtual card!")));
    else
        subcards << card;
}
