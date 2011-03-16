#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
    once = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->drawCards(source, subcards.length());
}


RendeCard::RendeCard(){
    will_throw = false;
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
    once = true;
}

bool JieyinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    room->recover(effect.from, 1, true);
    room->recover(effect.to, 1, true);
}

TuxiCard::TuxiCard(){
}

bool TuxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "h", "tuxi");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, Room::Bad);
    room->setEmotion(effect.from, Room::Good);
}

FanjianCard::FanjianCard(){
    once = true;
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->showCard(zhouyu, card_id);
    room->getThread()->delay();

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

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
    once = true;
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
    room->throwCard(this);

    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setCancelable(false);

    CardEffectStruct effect;
    effect.card = duel;
    effect.from = from;
    effect.to = to;

    room->cardEffect(effect);
}

QingnangCard::QingnangCard(){
    once = true;
}

bool QingnangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    if(targets.length() > 1)
        return false;

    const ClientPlayer *to_cure = targets.isEmpty() ? Self : targets.first();
    return to_cure->isWounded();
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

void GuicaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwSpecialCard();

    room->moveCardTo(this, NULL, Player::Special, true);
    room->setEmotion(source, Room::Normal);
}

LiuliCard::LiuliCard()
    :is_weapon(false)
{
}

void LiuliCard::setSlashSource(const QString &slash_source){
    this->slash_source = slash_source;
}

void LiuliCard::setIsWeapon(bool is_weapon)
{
    this->is_weapon = is_weapon;
}

bool LiuliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select->objectName() == slash_source)
        return false;

    if(to_select == Self)
        return false;

    if(is_weapon)
        return Self->distanceTo(to_select) <= 1;
    else
        return Self->inMyAttackRange(to_select);
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "liuli_target");
}

JijiangCard::JijiangCard(){

}

bool JijiangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void JijiangCard::use(Room *room, ServerPlayer *liubei, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
    const Card *slash = NULL;
    foreach(ServerPlayer *liege, lieges){
        QString result = room->askForChoice(liege, "jijiang", "accept+ignore");
        if(result == "ignore")
            continue;

        slash = room->askForCard(liege, "slash", "@jijiang-slash");
        if(slash){
            liubei->invoke("increaseSlashCount");
            room->cardEffect(slash, liubei, targets.first());
            return;
        }
    }
}

CheatCard::CheatCard(){
    target_fixed = true;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

