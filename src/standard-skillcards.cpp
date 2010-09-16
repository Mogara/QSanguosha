#include "standard.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
}

void ZhihengCard::use(const QList<const ClientPlayer *> &) const{
    ClientInstance->turn_tag.insert("zhiheng_used", true);
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
    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    source->setMark("rende", new_value);

    if(old_value < 2 && new_value >= 2)
        room->recover(source);
}

JieyinCard::JieyinCard(){

}

bool JieyinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

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
    room->loseHp(source);
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

bool QingnangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    if(Self->isWounded())
        return targets.length() <= 1;
    else
        return targets.length() == 1;
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target;
    if(!targets.isEmpty())
        target = targets.first();
    else
        target = source;

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->recover(effect.to, 1);
}

GuicaiCard::GuicaiCard(){
    target_fixed = true;
}

LiuliCard::LiuliCard(){
}

void LiuliCard::setSlashSource(const QString &slash_source){
    this->slash_source = slash_source;
}

bool LiuliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select->objectName() == slash_source)
        return false;

    return Self->inMyAttackRange(to_select);
}

HujiaCard::HujiaCard(){
    target_fixed = true;
}

CheatCard::CheatCard(){
    target_fixed = true;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->obtainCard(source, subcards.first());
}

