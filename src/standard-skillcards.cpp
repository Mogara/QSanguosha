#include "standard.h"
#include "room.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(source, this);
    room->drawCards(source, subcards.length());
}


RendeCard::RendeCard(){
}

void RendeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    foreach(int card_id, subcards){
        room->moveCard(source, Player::Hand, target, Player::Hand, card_id);
    }
}
