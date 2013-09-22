#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"

ZhihengCard::ZhihengCard() {
    target_fixed = true;
    mute = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    if (source->hasInnateSkill("zhiheng") || !source->hasSkill("jilve"))
        room->broadcastSkillInvoke("zhiheng");
    else
        room->broadcastSkillInvoke("jilve", 4);
    if (source->isAlive())
        room->drawCards(source, subcards.length());
}

RendeCard::RendeCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void RendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    int old_value = source->getMark("rende");
    QList<int> rende_list;
    if (old_value > 0)
        rende_list = StringList2IntList(source->property("rende").toString().split("+"));
    else
        rende_list = source->handCards();
    foreach (int id, this->subcards)
        rende_list.removeOne(id);
    room->setPlayerProperty(source, "rende", IntList2StringList(rende_list).join("+"));

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "rende", QString());
    room->obtainCard(target, this, reason, false);

    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if (old_value < 2 && new_value >= 2) {
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }

    if (room->getMode() == "04_1v3" && source->getMark("rende") >= 2) return;
    if (source->isKongcheng() || source->isDead() || rende_list.isEmpty()) return;
    room->addPlayerHistory(source, "RendeCard", -1);
    if (!room->askForUseCard(source, "@@rende", "@rende-give", -1, Card::MethodNone))
        room->addPlayerHistory(source, "RendeCard");
}

JieyinCard::JieyinCard() {
}

bool JieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    return to_select->isMale() && to_select->isWounded() && to_select != Self;
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

TuxiCard::TuxiCard() {
}

bool TuxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= 2 || to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if (effect.from->isAlive() && !effect.to->isKongcheng()) {
        int card_id = room->askForCardChosen(effect.from, effect.to, "h", "tuxi");
        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
        room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, false);
    }
}

FanjianCard::FanjianCard() {
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "fanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);
    room->showCard(target, card_id);

    if (card->getSuit() != suit)
        room->damage(DamageStruct("fanjian", zhouyu, target));
}

KurouCard::KurouCard() {
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if (source->isAlive())
        room->drawCards(source, 2);
}

LijianCard::LijianCard(bool cancelable): duel_cancelable(cancelable) {
    mute = true;
}

bool LijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!to_select->isMale())
        return false;

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->deleteLater();
    if (targets.isEmpty() && Self->isProhibited(to_select, duel))
        return false;

    if (targets.length() == 1 && to_select->isCardLimited(duel, Card::MethodUse))
        return false;

    return targets.length() < 2 && to_select != Self;
}

bool LijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LijianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *diaochan = card_use.from;

    LogMessage log;
    log.from = diaochan;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, diaochan, data);
    room->broadcastSkillInvoke("lijian");

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, diaochan->objectName(), QString(), "lijian", QString());
    room->moveCardTo(this, diaochan, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, diaochan, data);
    thread->trigger(CardFinished, room, diaochan, data);
}

void LijianCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setCancelable(duel_cancelable);
    duel->setSkillName(QString("_%1").arg(getSkillName()));
    if (!from->isCardLimited(duel, Card::MethodUse) && !from->isProhibited(to, duel))
        room->useCard(CardUseStruct(duel, from, to));
    else
        delete duel;
}

QingnangCard::QingnangCard() {
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

LiuliCard::LiuliCard() {
}

bool LiuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->hasFlag("LiuliSlashSource") || to_select == Self)
        return false;

    const Player *from = NULL;
    foreach (const Player *p, Self->getAliveSiblings()) {
        if (p->hasFlag("LiuliSlashSource")) {
            from = p;
            break;
        }
    }

    const Card *slash = Card::Parse(Self->property("liuli").toString());
    if (from && !from->canSlash(to_select, slash, false))
        return false;

    int card_id = subcards.first();
    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getId() == card_id) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - Self->getAttackRange(false);
    } else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getId() == card_id) {
        range_fix += 1;
    }

    return Self->distanceTo(to_select, range_fix) <= Self->getAttackRange();
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("LiuliTarget");
}

JijiangCard::JijiangCard() {
}

bool JijiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(NoSuit, 0);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *JijiangCard::validate(CardUseStruct &cardUse) const{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *liubei = cardUse.from;
    QList<ServerPlayer *> targets = cardUse.to;
    Room *room = liubei->getRoom();
    liubei->broadcastSkillInvoke(this);
    room->notifySkillInvoked(liubei, "jijiang");

    LogMessage log;
    log.from = liubei;
    log.to = targets;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    const Card *slash = NULL;

    QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
    foreach (ServerPlayer *target, targets)
        target->setFlags("JijiangTarget");
    foreach (ServerPlayer *liege, lieges) {
        try {
            slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(),
                                     QVariant(), Card::MethodResponse, liubei, false, QString(), true);
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
                foreach (ServerPlayer *target, targets)
                    target->setFlags("-JijiangTarget");
            }
            throw triggerEvent;
        }

        if (slash) {
            foreach (ServerPlayer *target, targets)
                target->setFlags("-JijiangTarget");

            return slash;
        }
    }
    foreach (ServerPlayer *target, targets)
        target->setFlags("-JijiangTarget");
    room->setPlayerFlag(liubei, "Global_JijiangFailed");
    return NULL;
}
