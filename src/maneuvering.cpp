#include "maneuvering.h"
#include "client.h"

Analeptic::Analeptic(Card::Suit suit, int number)
    :BasicCard(suit, number)
{
    target_fixed = true;
}

QString Analeptic::getSubtype() const{
    return "buff_card";
}

bool Analeptic::isAvailable() const{
    return ClientInstance->getPlayer()->hasFlag("drank");
}

void Analeptic::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->setPlayerFlag(source, "drank");
}
