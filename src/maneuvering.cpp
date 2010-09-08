#include "maneuvering.h"
#include "client.h"

Analeptic::Analeptic(Card::Suit suit, int number)
    :BasicCard(suit, number)
{
    setObjectName("analeptic");
    target_fixed = true;
}

QString Analeptic::getSubtype() const{
    return "buff_card";
}

bool Analeptic::isAvailable() const{
    return !Self->hasFlag("drank");
}

void Analeptic::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->setPlayerFlag(source, "drank");
}

ManeuveringPackage::ManeuveringPackage()
    :Package("maneuvering")
{
    t["maneuvering"] = tr("maneuvering");
    t["buff_card"] = tr("buff_card");
    t["analeptic"] = tr("analeptic");

    QList<Card *> cards;
    cards << new Analeptic(Card::Spade, 3)
            << new Analeptic(Card::Club, 3);

    foreach(Card *card, cards)
        card->setParent(this);
}

extern "C"{
    Q_DECL_EXPORT Package *NewManeuvering(){
        return new ManeuveringPackage;
    }
}
