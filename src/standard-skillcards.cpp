#include "standard.h"
#include "room.h"
#include "clientplayer.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
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

        room->moveCard(move);
    }
}

JieyinCard::JieyinCard(){

}

bool JieyinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->recover(source, 1);
    room->recover(targets.first(), 1);
}
