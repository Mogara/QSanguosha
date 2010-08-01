#include "standard.h"
#include "room.h"

bool ZhihengCard::targetFixed() const{
    return true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->drawCards(source, subcards.length());
}
