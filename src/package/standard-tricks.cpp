/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "standard-tricks.h"
#include "standard-package.h"
#include "room.h"
#include "util.h"
#include "engine.h"
#include "skill.h"
#include "json.h"

AmazingGrace::AmazingGrace(Suit suit, int number)
    : GlobalEffect(suit, number)
{
    setObjectName("amazing_grace");
    has_preact = true;
}

void AmazingGrace::clearRestCards(Room *room) const{
    room->clearAG();

    QVariantList ag_list = room->getTag("AmazingGrace").toList();
    if (ag_list.isEmpty()) return;
    DummyCard dummy(VariantList2IntList(ag_list));
    CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "amazing_grace", QString());
    room->throwCard(&dummy, reason, NULL);
}

void AmazingGrace::doPreAction(Room *room, const CardUseStruct &) const{
    QList<int> card_ids = room->getNCards(room->getAllPlayers().length());
    room->fillAG(card_ids);
    room->setTag("AmazingGrace", IntList2VariantList(card_ids));
}

void AmazingGrace::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    try {
        GlobalEffect::use(room, source, targets);
        clearRestCards(room);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange)
            clearRestCards(room);
        throw triggerEvent;
    }
}

void AmazingGrace::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("AmazingGrace").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, ag_list)
        card_ids << card_id.toInt();

    int card_id = room->askForAG(effect.to, card_ids, false, objectName());
    card_ids.removeOne(card_id);

    room->takeAG(effect.to, card_id);
    ag_list.removeOne(card_id);

    room->setTag("AmazingGrace", ag_list);
}

GodSalvation::GodSalvation(Suit suit, int number)
    : GlobalEffect(suit, number)
{
    setObjectName("god_salvation");
}

bool GodSalvation::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->isWounded() && TrickCard::isCancelable(effect);
}

void GodSalvation::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (!effect.to->isWounded());
    else {
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }
}

SavageAssault::SavageAssault(Suit suit, int number)
    : AOE(suit, number)
{
    setObjectName("savage_assault");
}

void SavageAssault::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *slash = room->askForCard(effect.to,
        "slash",
        "savage-assault-slash:" + effect.from->objectName(),
        QVariant::fromValue(effect),
        Card::MethodResponse,
        effect.from->isAlive() ? effect.from : NULL);
    if (slash) {
        if (slash->getSkillName() == "Spear") room->setEmotion(effect.to, "weapon/spear");
        room->setEmotion(effect.to, "killer");
    }
    else{
        room->damage(DamageStruct(this, effect.from->isAlive() ? effect.from : NULL, effect.to));
        room->getThread()->delay();
    }
}

ArcheryAttack::ArcheryAttack(Card::Suit suit, int number)
    : AOE(suit, number)
{
    setObjectName("archery_attack");
}

void ArcheryAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *jink = room->askForCard(effect.to,
        "jink",
        "archery-attack-jink:" + effect.from->objectName(),
        QVariant::fromValue(effect),
        Card::MethodResponse,
        effect.from->isAlive() ? effect.from : NULL);
    if (jink && jink->getSkillName() != "EightDiagram")
        room->setEmotion(effect.to, "jink");

    if (!jink){
        room->damage(DamageStruct(this, effect.from->isAlive() ? effect.from : NULL, effect.to));
        room->getThread()->delay();
    }
}

Collateral::Collateral(Card::Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("collateral");
}

bool Collateral::isAvailable(const Player *player) const{
    bool canUse = false;
    foreach(const Player *p, player->getAliveSiblings()) {
        if (p->getWeapon()) {
            canUse = true;
            break;
        }
    }
    return canUse && SingleTargetTrick::isAvailable(player);
}

bool Collateral::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

bool Collateral::targetFilter(const QList<const Player *> &targets,
    const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty()) {
        // @todo: fix this. We should probably keep the codes here, but change the code in
        // roomscene such that if it is collateral, then targetFilter's result is overriden
        Q_ASSERT(targets.length() <= 2);
        if (targets.length() == 2) return false;
        const Player *slashFrom = targets[0];
        return slashFrom->canSlash(to_select);
    }
    else {
        if (!to_select->getWeapon() || to_select == Self)
            return false;
        foreach(const Player *p, to_select->getAliveSiblings()) {
            if (to_select->canSlash(p))
                return true;
        }
    }
    return false;
}

void Collateral::onUse(Room *room, const CardUseStruct &card_use) const{
    Q_ASSERT(card_use.to.length() == 2);
    ServerPlayer *killer = card_use.to.at(0);
    ServerPlayer *victim = card_use.to.at(1);

    CardUseStruct new_use = card_use;
    new_use.to.removeAt(1);
    killer->tag["collateralVictim"] = QVariant::fromValue(victim);

    SingleTargetTrick::onUse(room, new_use);
}

bool Collateral::doCollateral(Room *room, ServerPlayer *killer, ServerPlayer *victim, const QString &prompt) const{
    bool useSlash = false;
    if (killer->canSlash(victim, NULL, false))
        useSlash = room->askForUseSlashTo(killer, victim, prompt);
    return useSlash;
}

void Collateral::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *source = effect.from;
    Room *room = source->getRoom();
    ServerPlayer *killer = effect.to;
    ServerPlayer *victim = effect.to->tag["collateralVictim"].value<ServerPlayer *>();
    effect.to->tag.remove("collateralVictim");
    if (!victim) return;
    WrappedCard *weapon = killer->getWeapon();

    QString prompt = QString("collateral-slash:%1:%2").arg(victim->objectName()).arg(source->objectName());

    if (victim->isDead()) {
        if (source->isAlive() && killer->isAlive() && killer->getWeapon()){
            CardMoveReason reason(CardMoveReason::S_REASON_GIVE, killer->objectName());
            room->obtainCard(source, weapon, reason);
        }
    }
    else if (source->isDead()) {
        if (killer->isAlive())
            doCollateral(room, killer, victim, prompt);
    }
    else {
        if (killer->isDead()) {
            ; // do nothing
        }
        else if (!killer->getWeapon()) {
            doCollateral(room, killer, victim, prompt);
        }
        else {
            if (!doCollateral(room, killer, victim, prompt)) {
                if (killer->getWeapon()){
                    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, killer->objectName());
                    room->obtainCard(source, weapon, reason);
                }
            }
        }
    }
}

Nullification::Nullification(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    target_fixed = true;
    setObjectName("nullification");
}

void Nullification::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    // does nothing, just throw it
    QList<int> table_cardids = room->getCardIdsOnTable(this);
    if (!table_cardids.isEmpty()) {
        DummyCard dummy(table_cardids);
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
        room->moveCardTo(&dummy, NULL, Player::DiscardPile, reason);
    }
}

bool Nullification::isAvailable(const Player *) const{
    return false;
}

HegNullification::HegNullification(Suit suit, int number)
    : Nullification(suit, number)
{
    target_fixed = true;
    setObjectName("heg_nullification");
}

ExNihilo::ExNihilo(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("ex_nihilo");
    target_fixed = true;
}

void ExNihilo::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    SingleTargetTrick::onUse(room, use);
}

bool ExNihilo::isAvailable(const Player *player) const{
    return !player->isProhibited(player, this) && TrickCard::isAvailable(player);
}

void ExNihilo::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

Duel::Duel(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("duel");
}

bool Duel::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;
    if (to_select == Self)
        return false;

    return true;
}

void Duel::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    room->setEmotion(first, "duel");
    room->setEmotion(second, "duel");

    forever{
        if (!first->isAlive())
        break;
        if (second->getMark("WushuangTarget") > 0) {
            const Card *slash = room->askForCard(first,
                "slash",
                "@wushuang-slash-1:" + second->objectName(),
                QVariant::fromValue(effect),
                Card::MethodResponse,
                second);
            if (slash == NULL)
                break;

            slash = room->askForCard(first, "slash",
                "@wushuang-slash-2:" + second->objectName(),
                QVariant::fromValue(effect),
                Card::MethodResponse,
                second);
            if (slash == NULL)
                break;
        }
        else {
            const Card *slash = room->askForCard(first,
                "slash",
                "duel-slash:" + second->objectName(),
                QVariant::fromValue(effect),
                Card::MethodResponse,
                second);
            if (slash == NULL)
                break;
        }

        qSwap(first, second);
    }

    room->setPlayerMark(first, "WushuangTarget", 0);
    room->setPlayerMark(second, "WushuangTarget", 0);

    DamageStruct damage(this, second->isAlive() ? second : NULL, first);
    if (second != effect.from)
        damage.by_user = false;
    room->damage(damage);
}

QStringList Duel::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

Snatch::Snatch(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("snatch");
}

bool Snatch::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    if (to_select->isAllNude())
        return false;

    if (to_select == Self)
        return false;

    int distance_limit = 1 + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this);
    int rangefix = 0;
    if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
        ++rangefix;
    if (m_skillName == "jixi")
        ++rangefix;

    int distance = Self->distanceTo(to_select, rangefix);
    if (distance == -1 || distance > distance_limit)
        return false;

    return true;
}

void Snatch::onEffect(const CardEffectStruct &effect) const{
    if (effect.from->isDead())
        return;
    if (effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
    room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
}

QStringList Snatch::checkTargetModSkillShow(const CardUseStruct &use) const{
    QSet<QString> show;
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            show << main_skill->objectName();
        }
    }
    int distance_max = 1;
    foreach(ServerPlayer *p, use.to){
        distance_max = qMax(distance_max, use.from->distanceTo(p));
    }
    if (distance_max > 1){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = distance_max - 1;
        if (m_skillName == "jixi") //dirty but important hack!!!!!!!!!!!!
            ++n;

        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getDistanceLimit(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            show << main_skill->objectName();
        }
    }
    return show.toList();
}

Dismantlement::Dismantlement(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("dismantlement");
}

bool Dismantlement::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    if (to_select->isAllNude())
        return false;

    if (to_select == Self)
        return false;

    return true;
}

void Dismantlement::onEffect(const CardEffectStruct &effect) const{
    if (effect.from->isDead())
        return;

    Room *room = effect.to->getRoom();
    if (!effect.from->canDiscard(effect.to, "hej"))
        return;

    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName(), false, Card::MethodDiscard);
    room->throwCard(card_id, room->getCardPlace(card_id) == Player::PlaceDelayedTrick ? NULL : effect.to, effect.from);
}

QStringList Dismantlement::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

IronChain::IronChain(Card::Suit suit, int number)
    : TrickCard(suit, number)
{
    setObjectName("iron_chain");
    can_recast = true;
}

QString IronChain::getSubtype() const{
    return "damage_spread";
}

bool IronChain::targetFilter(const QList<const Player *> &targets, const Player *, const Player *Self) const{
    int total_num = 2 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;
    if (Self->isCardLimited(this, Card::MethodUse))
        return false;

    return true;
}

bool IronChain::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    bool rec = (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY);
    QList<int> sub;
    if (isVirtualCard())
        sub = subcards;
    else
        sub << getEffectiveId();
    foreach (int id, sub) {
        if (Self->getPile("wooden_ox").contains(id)) {
            rec = false;
            break;
        }
    }

    if (rec && Self->isCardLimited(this, Card::MethodUse))
        return targets.length() == 0;
    int total_num = 2 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() > total_num)
        return false;
    return rec || targets.length() > 0;
}

void IronChain::onUse(Room *room, const CardUseStruct &card_use) const{
    if (card_use.to.isEmpty()) {
        CardMoveReason reason(CardMoveReason::S_REASON_RECAST, card_use.from->objectName());
        reason.m_skillName = getSkillName();
        room->moveCardTo(this, card_use.from, NULL, Player::PlaceTable, reason, true);
        card_use.from->broadcastSkillInvoke("@recast");

        LogMessage log;
        log.type = "#Card_Recast";
        log.from = card_use.from;
        log.card_str = card_use.card->toString();
        room->sendLog(log);

        QString skill_name = card_use.card->showSkill();
        if (!skill_name.isNull() && card_use.from->ownSkill(skill_name) && !card_use.from->hasShownSkill(skill_name))
            card_use.from->showGeneral(card_use.from->inHeadSkills(skill_name));

        QList<int> table_cardids = room->getCardIdsOnTable(this);
        if (!table_cardids.isEmpty()) {
            DummyCard dummy(table_cardids);
            room->moveCardTo(&dummy, card_use.from, NULL, Player::DiscardPile, reason, true);
        }

        card_use.from->drawCards(1);
    } else
        TrickCard::onUse(room, card_use);
}

void IronChain::onEffect(const CardEffectStruct &effect) const{
    if (!effect.to->canBeChainedBy(effect.from))
        return;
    effect.to->setChained(!effect.to->isChained());

    Room *room = effect.to->getRoom();

    room->broadcastProperty(effect.to, "chained");
    room->setEmotion(effect.to, "chain");
    room->getThread()->trigger(ChainStateChanged, room, effect.to);
}

QStringList IronChain::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 3){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 2;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

AwaitExhausted::AwaitExhausted(Card::Suit suit, int number) : TrickCard(suit, number){
    setObjectName("await_exhausted");
    target_fixed = true;
}

QString AwaitExhausted::getSubtype() const{
    return "await_exhausted";
}

bool AwaitExhausted::isAvailable(const Player *player) const{
    bool canUse = false;
    if (!player->isProhibited(player, this))
        canUse = true;
    if (!canUse) {
        QList<const Player *> players = player->getAliveSiblings();
        foreach(const Player *p, players) {
            if (player->isProhibited(p, this))
                continue;
            if (player->isFriendWith(p)) {
                canUse = true;
                break;
            }
        }
    }

    return canUse && TrickCard::isAvailable(player);
}

void AwaitExhausted::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct new_use = card_use;
    if (!card_use.from->isProhibited(card_use.from, this))
        new_use.to << new_use.from;
    foreach(ServerPlayer *p, room->getOtherPlayers(new_use.from)) {
        if (p->isFriendWith(new_use.from)) {
            const ProhibitSkill *skill = room->isProhibited(card_use.from, p, this);
            if (skill) {
                LogMessage log;
                log.type = "#SkillAvoid";
                log.from = p;
                log.arg = skill->objectName();
                log.arg2 = objectName();
                room->sendLog(log);

                room->broadcastSkillInvoke(skill->objectName(), p);
            } else {
                new_use.to << p;
            }
        }
    }

    if (getSkillName() == "duoshi")
        room->addPlayerHistory(new_use.from, "DuoshiAE", 1);

    TrickCard::onUse(room, new_use);
}

void AwaitExhausted::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QStringList nullified_list = room->getTag("CardUseNullifiedList").toStringList();
    bool all_nullified = nullified_list.contains("_ALL_TARGETS");
    foreach(ServerPlayer *target, targets) {
        CardEffectStruct effect;
        effect.card = this;
        effect.from = source;
        effect.to = target;
        effect.multiple = (targets.length() > 1);
        effect.nullified = (all_nullified || nullified_list.contains(target->objectName()));

        room->cardEffect(effect);
    }

    foreach(ServerPlayer *target, targets) {
        if (target->hasFlag("AwaitExhaustedEffected")) {
            room->setPlayerFlag(target, "-AwaitExhaustedEffected");
            room->askForDiscard(target, objectName(), 2, 2, false, true);
        }
    }

    QList<int> table_cardids = room->getCardIdsOnTable(this);
    if (!table_cardids.isEmpty()) {
        DummyCard dummy(table_cardids);
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName(), QString(), this->getSkillName(), QString());
        if (targets.size() == 1) reason.m_targetId = targets.first()->objectName();
        room->moveCardTo(&dummy, source, NULL, Player::DiscardPile, reason, true);
    }
}

void AwaitExhausted::onEffect(const CardEffectStruct &effect) const {
    effect.to->drawCards(2);
    effect.to->getRoom()->setPlayerFlag(effect.to, "AwaitExhaustedEffected");
}

KnownBoth::KnownBoth(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number)
{
    setObjectName("known_both");
    can_recast = true;
}

bool KnownBoth::isAvailable(const Player *player) const{
    bool can_use = false;
    foreach (const Player *p, player->getSiblings()) {
        if (player->isProhibited(p, this))
            continue;
        if (p->isKongcheng() && p->hasShownAllGenerals())
            continue;
        can_use = true;
        break;
    }
    bool can_rec = true;
    QList<int> sub;
    if (isVirtualCard())
        sub = subcards;
    else
        sub << getEffectiveId();
    if (sub.isEmpty() || sub.contains(-1))
        can_rec = false;
    return (can_use && !player->isCardLimited(this, Card::MethodUse))
           || (can_rec && can_recast && !player->isCardLimited(this, Card::MethodRecast));
}

bool KnownBoth::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (Self->isCardLimited(this, Card::MethodUse))
        return false;

    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num || to_select == Self)
        return false;

    return !to_select->isKongcheng() || !to_select->hasShownAllGenerals();
}

bool KnownBoth::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    bool rec = (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY);
    QList<int> sub;
    if (isVirtualCard())
        sub = subcards;
    else
        sub << getEffectiveId();
    foreach (int id, sub) {
        if (Self->getPile("wooden_ox").contains(id)) {
            rec = false;
            break;
        }
    }

    if (rec && Self->isCardLimited(this, Card::MethodUse))
        return targets.length() == 0;
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() > total_num)
        return false;
    return rec || targets.length() > 0;
}

void KnownBoth::onUse(Room *room, const CardUseStruct &card_use) const{
    if (card_use.to.isEmpty()){
        CardMoveReason reason(CardMoveReason::S_REASON_RECAST, card_use.from->objectName());
        reason.m_skillName = getSkillName();
        room->moveCardTo(this, card_use.from, NULL, Player::PlaceTable, reason);
        card_use.from->broadcastSkillInvoke("@recast");

        LogMessage log;
        log.type = "#Card_Recast";
        log.from = card_use.from;
        log.card_str = card_use.card->toString();
        room->sendLog(log);

        QString skill_name = card_use.card->showSkill();
        if (!skill_name.isNull() && card_use.from->ownSkill(skill_name) && !card_use.from->hasShownSkill(skill_name))
            card_use.from->showGeneral(card_use.from->inHeadSkills(skill_name));

        QList<int> table_cardids = room->getCardIdsOnTable(this);
        if (!table_cardids.isEmpty()) {
            DummyCard dummy(table_cardids);
            room->moveCardTo(&dummy, card_use.from, NULL, Player::DiscardPile, reason, true);
        }

        card_use.from->drawCards(1);
    } else
        SingleTargetTrick::onUse(room, card_use);
}

void KnownBoth::onEffect(const CardEffectStruct &effect) const {
    QStringList choices;
    if (!effect.to->isKongcheng())
        choices << "handcards";
    if (!effect.to->hasShownGeneral1())
        choices << "head_general";
    if (effect.to->getGeneral2() && !effect.to->hasShownGeneral2())
        choices << "deputy_general";

    Room *room = effect.from->getRoom();

    effect.to->setFlags("KnownBothTarget");// For AI
    QString choice = room->askForChoice(effect.from, objectName(),
        choices.join("+"), QVariant::fromValue(effect.to));
    effect.to->setFlags("-KnownBothTarget");
    LogMessage log;
    log.type = "#KnownBothView";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = choice;
    foreach(ServerPlayer *p, room->getOtherPlayers(effect.from, true)){
        room->doNotify(p, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
    }

    if (choice == "handcards")
        room->showAllCards(effect.to, effect.from);
    else {
        QStringList list = room->getTag(effect.to->objectName()).toStringList();
        list.removeAt(choice == "head_general" ? 1 : 0);
        foreach(QString name, list) {
            LogMessage log;
            log.type = "$KnownBothViewGeneral";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = name;
            log.arg2 = choice;
            room->doNotify(effect.from, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
        }
        JsonArray arg;
        arg << objectName();
        arg << JsonUtils::toJsonArray(list);
        room->doNotify(effect.from, QSanProtocol::S_COMMAND_VIEW_GENERALS, arg);
    }
}

QStringList KnownBoth::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

BefriendAttacking::BefriendAttacking(Card::Suit suit, int number) : SingleTargetTrick(suit, number) {
    setObjectName("befriend_attacking");
}

bool BefriendAttacking::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    return to_select->hasShownOneGeneral() && !Self->isFriendWith(to_select);
}

void BefriendAttacking::onEffect(const CardEffectStruct &effect) const {
    effect.to->drawCards(1);
    effect.from->drawCards(3);
}

bool BefriendAttacking::isAvailable(const Player *player) const {
    return player->hasShownOneGeneral() && TrickCard::isAvailable(player);
}

QStringList BefriendAttacking::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

FireAttack::FireAttack(Card::Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("fire_attack");
}

bool FireAttack::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    if (to_select->isKongcheng())
        return false;

    if (to_select == Self)
        return !Self->isLastHandCard(this, true);
    else
        return true;
}

void FireAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if (effect.to->isKongcheng())
        return;

    const Card *card = room->askForCardShow(effect.to, effect.from, objectName());
    room->showCard(effect.to, card->getEffectiveId());

    QString suit_str = card->getSuitString();
    QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
    QString prompt = QString("@fire-attack:%1::%2").arg(effect.to->objectName()).arg(suit_str);
    if (effect.from->isAlive()) {
        const Card *card_to_throw = room->askForCard(effect.from, pattern, prompt);
        if (card_to_throw)
            room->damage(DamageStruct(this, effect.from, effect.to, 1, DamageStruct::Fire));
        else
            effect.from->setFlags("FireAttackFailed_" + effect.to->objectName()); // For AI
    }

    if (card->isVirtualCard())
        delete card;
}

QStringList FireAttack::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

Indulgence::Indulgence(Suit suit, int number)
    : DelayedTrick(suit, number)
{
    setObjectName("indulgence");

    judge.pattern = ".|heart";
    judge.good = true;
    judge.reason = objectName();
}

bool Indulgence::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->containsTrick(objectName()) && to_select != Self;
}

void Indulgence::takeEffect(ServerPlayer *target) const{
    target->clearHistory();
#ifndef QT_NO_DEBUG
    if (!target->getAI() && target->askForSkillInvoke("userdefine:cancelIndulgence")) return;
#endif
    target->skip(Player::Play);
}

SupplyShortage::SupplyShortage(Card::Suit suit, int number)
    : DelayedTrick(suit, number)
{
    setObjectName("supply_shortage");

    judge.pattern = ".|club";
    judge.good = true;
    judge.reason = objectName();
}

bool SupplyShortage::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select->containsTrick(objectName()) || to_select == Self)
        return false;

    int distance_limit = 1 + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this);
    int rangefix = 0;
    if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
        ++rangefix;

    int distance = Self->distanceTo(to_select, rangefix);
    if (distance == -1 || distance > distance_limit)
        return false;

    return true;
}

void SupplyShortage::takeEffect(ServerPlayer *target) const{
#ifndef QT_NO_DEBUG
    if (!target->getAI() && target->askForSkillInvoke("userdefine:cancelSupplyShortage")) return;
#endif
    target->skip(Player::Draw);
}

QStringList SupplyShortage::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.from->distanceTo(use.to.first()) > 1){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.from->distanceTo(use.to.first()) - 1;

        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getDistanceLimit(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

Disaster::Disaster(Card::Suit suit, int number)
    : DelayedTrick(suit, number, true)
{
    target_fixed = true;
}

void Disaster::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    DelayedTrick::onUse(room, use);
}

bool Disaster::isAvailable(const Player *player) const{
    if (player->containsTrick(objectName()))
        return false;

    return !player->isProhibited(player, this) && DelayedTrick::isAvailable(player);
}

Lightning::Lightning(Suit suit, int number) :Disaster(suit, number) {
    setObjectName("lightning");

    judge.pattern = ".|spade|2~9";
    judge.good = false;
    judge.reason = objectName();
}

void Lightning::takeEffect(ServerPlayer *target) const{
#ifndef QT_NO_DEBUG
    if (!target->getAI() && target->askForSkillInvoke("userdefine:cancelLightning")) return;
#endif
    target->getRoom()->damage(DamageStruct(this, NULL, target, 3, DamageStruct::Thunder));
}

QList<Card *> StandardCardPackage::trickCards(){
    QList<Card *> cards;

    cards
        << new AmazingGrace
        << new GodSalvation
        << new SavageAssault(Card::Spade, 13)
        << new SavageAssault(Card::Club, 7)
        << new ArcheryAttack
        << new Duel(Card::Spade, 1)
        << new Duel(Card::Club, 1)
        << new ExNihilo(Card::Heart, 7)
        << new ExNihilo(Card::Heart, 8)
        << new Snatch(Card::Spade, 3)
        << new Snatch(Card::Spade, 4)
        << new Snatch(Card::Diamond, 3)
        << new Dismantlement(Card::Spade, 3)
        << new Dismantlement(Card::Spade, 4)
        << new Dismantlement(Card::Heart, 12)
        << new IronChain(Card::Spade, 12)
        << new IronChain(Card::Club, 12)
        << new IronChain(Card::Club, 13)
        << new FireAttack(Card::Heart, 2)
        << new FireAttack(Card::Heart, 3)
        << new Collateral
        << new Nullification
        << new HegNullification(Card::Club, 13)
        << new HegNullification(Card::Diamond, 12)
        << new AwaitExhausted(Card::Heart, 11)
        << new AwaitExhausted(Card::Diamond, 4)
        << new KnownBoth(Card::Club, 3)
        << new KnownBoth(Card::Club, 4)
        << new BefriendAttacking
        << new Indulgence(Card::Club, 6)
        << new Indulgence(Card::Heart, 6)
        << new SupplyShortage(Card::Spade, 10)
        << new SupplyShortage(Card::Club, 10)
        << new Lightning;

    return cards;
}
