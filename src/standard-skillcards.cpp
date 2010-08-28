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
        CardMoveStruct move;
        move.card_id = card_id;
        move.from = source;
        move.to = target;
        move.from_place = move.to_place = Player::Hand;
        move.open = false;

        room->moveCard(NULL, move);
    }
}
