#include "standard.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"

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

FanjianCard::FanjianCard(){

}

void FanjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    int card_id = source->getRandomHandCard();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = this;
        damage.from = source;
        damage.to = target;
        damage.damage = 1;

        room->damage(damage);
    }

    if(target->isAlive()){
        target->obtainCard(card);
    }
}

KurouCard::KurouCard(){
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    DamageStruct damage;
    damage.card = this;
    damage.from = source;
    damage.to = source;
    damage.damage = 1;
    room->damage(damage);

    if(source->isAlive())
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

GuicaiCard::GuicaiCard(){
    target_fixed = true;
}

LiuliCard::LiuliCard(){
}

bool LiuliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    return Self->inMyAttackRange(to_select);
}
