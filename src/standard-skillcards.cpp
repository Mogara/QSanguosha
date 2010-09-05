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

TuxiCard::TuxiCard(){
}

bool TuxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() >= 2)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(ServerPlayer *target, targets){
        room->obtainCard(source, target->getRandomHandCard());
    }
}

KurouCard::KurouCard(){
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    // FIXME
    room->damage(source, 1);
    room->drawCards(source, 2);
}

LijianCard::LijianCard(){

}

bool LijianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!to_select->getGeneral()->isMale())
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

bool LijianCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

void LijianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    CardEffectStruct effect;
    effect.card = new Duel(Card::NoSuit, 0);
    effect.from = from;
    effect.to = to;

    room->cardEffect(effect);
}

QingnangCard::QingnangCard(){
    target_fixed = true;
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->recover(source);
}
