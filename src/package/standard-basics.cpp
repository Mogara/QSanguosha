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

#include "standard-basics.h"
#include "standard-package.h"
#include "engine.h"

Slash::Slash(Suit suit, int number) : BasicCard(suit, number)
{
    setObjectName("slash");
    nature = DamageStruct::Normal;
    drank = 0;
}

DamageStruct::Nature Slash::getNature() const{
    return nature;
}

void Slash::setNature(DamageStruct::Nature nature) {
    this->nature = nature;
}

bool Slash::IsAvailable(const Player *player, const Card *slash, bool considerSpecificAssignee) {
    Slash *newslash = new Slash(Card::NoSuit, 0);
    newslash->deleteLater();
#define THIS_SLASH (slash == NULL ? newslash : slash)
    if (player->isCardLimited(THIS_SLASH, Card::MethodUse))
        return false;

    if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
        QList<int> ids;
        if (slash) {
            if (slash->isVirtualCard()) {
                if (slash->subcardsLength() > 0)
                    ids = slash->getSubcards();
            }
            else {
                ids << slash->getEffectiveId();
            }
        }
        bool has_weapon = player->hasWeapon("Crossbow") && ids.contains(player->getWeapon()->getEffectiveId());
        if ((!has_weapon && player->hasWeapon("Crossbow")) || player->canSlashWithoutCrossbow(THIS_SLASH))
            return true;

        if (considerSpecificAssignee) {
            QStringList assignee_list = player->property("extra_slash_specific_assignee").toString().split("+");
            if (!assignee_list.isEmpty()) {
                foreach (const Player *p, player->getAliveSiblings()) {
                    if (assignee_list.contains(p->objectName()) && player->canSlash(p, THIS_SLASH))
                        return true;
                }
            }
        }
        return false;
    }
    else {
        return true;
    }
#undef THIS_SLASH
}

bool Slash::IsSpecificAssignee(const Player *player, const Player *from, const Card *slash) {
    if (from->hasFlag("slashTargetFix") && player->hasFlag("SlashAssignee"))
        return true;
    else if (from->getPhase() == Player::Play && Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY
        && !Slash::IsAvailable(from, slash, false)) {
        QStringList assignee_list = from->property("extra_slash_specific_assignee").toString().split("+");
        if (assignee_list.contains(player->objectName())) return true;
    }
    return false;
}


bool Slash::isAvailable(const Player *player) const{
    return IsAvailable(player, this) && BasicCard::isAvailable(player);
}

QString Slash::getSubtype() const{
    return "attack_card";
}

void Slash::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    ServerPlayer *player = use.from;

    if (player->hasFlag("slashTargetFix")) {
        room->setPlayerFlag(player, "-slashTargetFix");
        room->setPlayerFlag(player, "-slashTargetFixToOne");
        foreach (ServerPlayer *target, room->getAlivePlayers())
            if (target->hasFlag("SlashAssignee"))
                room->setPlayerFlag(target, "-SlashAssignee");
    }

    if (player->hasFlag("HalberdSlashFilter")) {
        if (player->getWeapon() != NULL)
            room->setCardFlag(player->getWeapon()->getId(), "-using");
        room->setPlayerFlag(player, "-HalberdSlashFilter");
        room->setPlayerMark(player, "halberd_count", card_use.to.length() - 1);
    }

    /* actually it's not proper to put the codes here.
       considering the nasty design of the client and the convenience as well,
       I just move them here */
    if (objectName() == "slash" && use.m_isOwnerUse) {
        bool has_changed = false;
        QString skill_name = getSkillName();
        if (!skill_name.isEmpty()) {
            const ViewAsSkill *skill = Sanguosha->getViewAsSkill(skill_name);
            if (skill && !skill->inherits("FilterSkill"))
                has_changed = true;
        }
        if (!has_changed || subcardsLength() == 0) {
            QVariant data = QVariant::fromValue(use);
            if (use.card->objectName() == "slash" && player->hasWeapon("Fan")) {
                FireSlash *fire_slash = new FireSlash(getSuit(), getNumber());
                if (!isVirtualCard() || subcardsLength() > 0)
                    fire_slash->addSubcard(this);
                fire_slash->setSkillName("Fan");
                bool can_use = true;
                foreach (ServerPlayer *p, use.to) {
                    if (!player->canSlash(p, fire_slash, false)) {
                        can_use = false;
                        break;
                    }
                }
                if (can_use && room->askForSkillInvoke(player, "Fan", data))
                    use.card = fire_slash;
                else
                    delete fire_slash;
            }
        }
    }
    if (((use.card->isVirtualCard() && use.card->subcardsLength() == 0) || player->hasFlag("HalberdUse"))
        && !player->hasFlag("slashDisableExtraTarget")) {
        if (!player->hasFlag("HalberdUse") && player->hasWeapon("Halberd")) {
            room->setPlayerFlag(player, "HalberdSlashFilter");
            forever {
                QList<ServerPlayer *> targets_ts;
                QList<const Player *> targets_const;
                foreach (ServerPlayer *p, use.to)
                    targets_const << qobject_cast<const Player *>(p);
                foreach (ServerPlayer *p, room->getAlivePlayers())
                    if (!use.to.contains(p) && use.card->targetFilter(targets_const, p, use.from))
                        targets_ts << p;
                if (targets_ts.isEmpty())
                break;

                ServerPlayer *extra_target = room->askForPlayerChosen(player, targets_ts, "Halberd", "@halberd_extra_targets", true);
                if (extra_target) {
                    room->setPlayerFlag(player, "HalberdUse");
                    room->addPlayerMark(player, "halberd_count");
                    use.to.append(extra_target);
                    room->sortByActionOrder(use.to);
                } else
                    break;
            }
            room->setPlayerFlag(player, "-HalberdSlashFilter");
        }

        QList<ServerPlayer *> targets_ts;
        while (true) {
            QList<const Player *> targets_const;
            foreach (ServerPlayer *p, use.to)
                targets_const << qobject_cast<const Player *>(p);
            foreach (ServerPlayer *p, room->getAlivePlayers())
                if (!use.to.contains(p) && use.card->targetFilter(targets_const, p, use.from))
                    targets_ts << p;
            if (targets_ts.isEmpty())
                break;

            ServerPlayer *extra_target = room->askForPlayerChosen(player, targets_ts, "slash_extra_targets", "@slash_extra_targets", true);
            if (extra_target) {
                use.to.append(extra_target);
                room->sortByActionOrder(use.to);
            } else
                break;
            targets_ts.clear();
            targets_const.clear();
        }
    }

    if (player->hasFlag("slashNoDistanceLimit"))
        room->setPlayerFlag(player, "-slashNoDistanceLimit");
    if (player->hasFlag("slashDisableExtraTarget"))
        room->setPlayerFlag(player, "-slashDisableExtraTarget");
    // for Paoxiao
    if (player->getPhase() == Player::Play && player->hasFlag("Global_MoreSlashInOneTurn")) {
        if (player->hasSkill("paoxiao")) {
            if (!player->hasShownSkill("paoxiao"))
                player->showGeneral(player->inHeadSkills("paoxiao"));
            player->setFlags("-Global_MoreSlashInOneTurn");
            room->broadcastSkillInvoke("paoxiao", player);
            room->notifySkillInvoked(player, "paoxiao");
        }
    }
    // for Tianyi
    if ((use.to.size() > 1 + player->getMark("halberd_count")
        || (player->hasFlag("Global_MoreSlashInOneTurn") && player->getSlashCount() == 2))
        && player->hasFlag("TianyiSuccess") && player->getPhase() == Player::Play) {
        if (player->hasFlag("Global_MoreSlashInOneTurn")) // Tianyi just let player could use one more Slash
            room->setPlayerFlag(player, "-Global_MoreSlashInOneTurn");
        room->broadcastSkillInvoke("tianyi", 1, player);
    }
    // for Duanbing
    if (use.to.size() > 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, player, this)
        && player->hasSkill("duanbing")) {
        if (!player->hasShownSkill("duanbing"))
            player->showGeneral(player->inHeadSkills("duanbing"));
        room->broadcastSkillInvoke("duanbing", player);
        room->notifySkillInvoked(player, "duanbing");
    }

    if (use.card->isVirtualCard()) {
        if (use.card->getSkillName() == "Spear")
            room->setEmotion(player, "weapon/spear");
        else if (use.card->getSkillName() == "Fan")
            room->setEmotion(player, "weapon/fan");
    }

    if (use.from->hasFlag("HalberdUse")) {
        use.from->setFlags("-HalberdUse");
        room->setEmotion(player, "weapon/halberd");

        LogMessage log;
        log.type = "#HalberdUse";
        log.from = use.from;
        room->sendLog(log);

        room->setPlayerMark(player, "halberd_count", 0);

        use.card->setFlags("halberd_slash");
    }

    if (player->getPhase() == Player::Play
        && player->hasFlag("Global_MoreSlashInOneTurn")
        && player->hasWeapon("Crossbow")
        && !player->hasSkill("paoxiao")) {
        player->setFlags("-Global_MoreSlashInOneTurn");
        room->setEmotion(player, "weapon/crossbow");
    }
    if (use.card->isKindOf("ThunderSlash"))
        room->setEmotion(player, "thunder_slash");
    else if (use.card->isKindOf("FireSlash"))
        room->setEmotion(player, "fire_slash");
    else if (use.card->isRed())
        room->setEmotion(player, "slash_red");
    else if (use.card->isBlack())
        room->setEmotion(player, "slash_black");
    else
        room->setEmotion(player, "killer");

    BasicCard::onUse(room, use);
}

void Slash::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();
    if (card_effect.from->getMark("drank") > 0) {
        room->setCardFlag(this, "drank");
        this->drank = card_effect.from->getMark("drank");
        room->setPlayerMark(card_effect.from, "drank", 0);
    }

    SlashEffectStruct effect;
    effect.from = card_effect.from;
    effect.nature = nature;
    effect.slash = this;

    effect.to = card_effect.to;
    effect.drank = this->drank;
    effect.nullified = card_effect.nullified;

    QVariantList jink_list = effect.from->tag["Jink_" + toString()].toList();
    effect.jink_num = jink_list.takeFirst().toInt();
    if (jink_list.isEmpty())
        effect.from->tag.remove("Jink_" + toString());
    else
        effect.from->tag["Jink_" + toString()] = QVariant::fromValue(jink_list);

    room->slashEffect(effect);
}

bool Slash::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int slash_targets = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    bool distance_limit = ((1 + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this)) < 500);
    if (Self->hasFlag("slashNoDistanceLimit"))
        distance_limit = false;

    int rangefix = 0;
    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        rangefix += weapon->getRange() - Self->getAttackRange(false);
    }

    if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
        ++rangefix;

    bool has_specific_assignee = false;
    foreach (const Player *p, Self->getAliveSiblings()) {
        if (Slash::IsSpecificAssignee(p, Self, this)) {
            has_specific_assignee = true;
            break;
        }
    }

    if (has_specific_assignee) {
        if (targets.isEmpty())
            return Slash::IsSpecificAssignee(to_select, Self, this) && Self->canSlash(to_select, this, distance_limit, rangefix);
        else {
            if (Self->hasFlag("slashDisableExtraTarget")) return false;
            bool canSelect = false;
            foreach (const Player *p, targets) {
                if (Slash::IsSpecificAssignee(p, Self, this)) {
                    canSelect = true;
                    break;
                }
            }
            if (!canSelect) return false;
        }
    }

    if (!Self->canSlash(to_select, this, distance_limit, rangefix, targets)) return false;
    if (Self->hasFlag("HalberdSlashFilter")) {
        QSet<QString> kingdoms;
        foreach (const Player *p, targets) {
            if (!p->hasShownOneGeneral() || p->getRole() == "careerist")
                continue;
            kingdoms << p->getKingdom();
        }
        if (to_select->getMark("Equips_of_Others_Nullified_to_You") > 0)
            return false;
        if (to_select->hasShownOneGeneral() && to_select->getRole() == "careerist") // careerist!
            return true;
        if (to_select->hasShownOneGeneral() && kingdoms.contains(to_select->getKingdom()))
            return false;
    } else if (targets.length() >= slash_targets) {
        if (Self->hasSkill("duanbing") && targets.length() == slash_targets) {
            QList<const Player *> duanbing_targets;
            bool no_other_assignee = true;
            foreach (const Player *p, targets) {
                if (Self->distanceTo(p, rangefix) == 1)
                    duanbing_targets << p;
                else if (no_other_assignee && Slash::IsSpecificAssignee(p, Self, this))
                    no_other_assignee = false;
            }
            if (no_other_assignee && duanbing_targets.length() == 1 && Slash::IsSpecificAssignee(duanbing_targets.first(), Self, this))
                return Self->distanceTo(to_select, rangefix) == 1;
            return Self->distanceTo(to_select, rangefix) == 1;
        } else
            return false;
    }

    return true;
}

NatureSlash::NatureSlash(Suit suit, int number, DamageStruct::Nature nature)
    : Slash(suit, number)
{
    this->nature = nature;
}

bool NatureSlash::match(const QString &pattern) const{
    QStringList patterns = pattern.split("+");
    if (patterns.contains("slash"))
        return true;
    else
        return Slash::match(pattern);
}

ThunderSlash::ThunderSlash(Suit suit, int number, bool is_transferable)
    : NatureSlash(suit, number, DamageStruct::Thunder)
{
    setObjectName("thunder_slash");
    transferable = is_transferable;
}

FireSlash::FireSlash(Suit suit, int number)
    : NatureSlash(suit, number, DamageStruct::Fire)
{
    setObjectName("fire_slash");
}

Jink::Jink(Suit suit, int number) : BasicCard(suit, number)
{
    setObjectName("jink");
    target_fixed = true;
}

QString Jink::getSubtype() const{
    return "defense_card";
}

bool Jink::isAvailable(const Player *) const{
    return false;
}

Peach::Peach(Suit suit, int number, bool is_transferable) : BasicCard(suit, number) {
    setObjectName("peach");
    target_fixed = true;
    transferable = is_transferable;
}

QString Peach::getSubtype() const{
    return "recover_card";
}

void Peach::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    BasicCard::onUse(room, use);
}

void Peach::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setEmotion(effect.from, "peach");

    // recover hp
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

bool Peach::isAvailable(const Player *player) const{
    return player->isWounded() && !player->isProhibited(player, this) && BasicCard::isAvailable(player);
}

Analeptic::Analeptic(Card::Suit suit, int number, bool is_transferable)
    : BasicCard(suit, number)
{
    setObjectName("analeptic");
    target_fixed = true;
    transferable = is_transferable;
}

QString Analeptic::getSubtype() const{
    return "buff_card";
}

bool Analeptic::IsAvailable(const Player *player, const Card *analeptic) {
    Analeptic *newanal = new Analeptic(Card::NoSuit, 0);
    newanal->deleteLater();
#define THIS_ANAL (analeptic == NULL ? newanal : analeptic)
    if (player->isCardLimited(THIS_ANAL, Card::MethodUse) || player->isProhibited(player, THIS_ANAL))
        return false;

    return player->usedTimes("Analeptic") <= Sanguosha->correctCardTarget(TargetModSkill::Residue, player, THIS_ANAL);
#undef THIS_ANAL
}

bool Analeptic::isAvailable(const Player *player) const{
    return IsAvailable(player, this) && BasicCard::isAvailable(player);
}

void Analeptic::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    BasicCard::onUse(room, use);
}

void Analeptic::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setEmotion(effect.to, "analeptic");

    if (effect.to->hasFlag("Global_Dying") && Sanguosha->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY) {
        // recover hp
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }
    else {
        room->addPlayerMark(effect.to, "drank");
    }
}

QStringList Analeptic::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.card == NULL)
        return QStringList();

    if (use.from->usedTimes(getClassName()) >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach (const Skill *skill, skills){
            if (from->hasSkill(skill) && skill->inherits("TargetModSkill")){
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.from->usedTimes(getClassName()) - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach (const TargetModSkill *tarmod, tarmods_copy){
            if (tarmod->getResidueNum(from, use.card) == 0) {
                tarmods.removeOne(tarmod);
                continue;
            }

            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getResidueNum(from, use.card);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach (const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

QList<Card *> StandardCardPackage::basicCards() {
    QList<Card *> cards;


    cards
        << new Slash(Card::Spade, 5)
        << new Slash(Card::Spade, 7)
        << new Slash(Card::Spade, 8)
        << new Slash(Card::Spade, 8)
        << new Slash(Card::Spade, 9)
        << new Slash(Card::Spade, 10)
        << new Slash(Card::Spade, 11)

        << new Slash(Card::Club, 2)
        << new Slash(Card::Club, 3)
        << new Slash(Card::Club, 4)
        << new Slash(Card::Club, 5)
        << new Slash(Card::Club, 8)
        << new Slash(Card::Club, 9)
        << new Slash(Card::Club, 10)
        << new Slash(Card::Club, 11)
        << new Slash(Card::Club, 11)

        << new Slash(Card::Heart, 10)
        << new Slash(Card::Heart, 12)

        << new Slash(Card::Diamond, 10)
        << new Slash(Card::Diamond, 11)
        << new Slash(Card::Diamond, 12)

        << new FireSlash(Card::Heart, 4)

        << new FireSlash(Card::Diamond, 4)
        << new FireSlash(Card::Diamond, 5)

        << new ThunderSlash(Card::Spade, 6)
        << new ThunderSlash(Card::Spade, 7)

        << new ThunderSlash(Card::Club, 6)
        << new ThunderSlash(Card::Club, 7)
        << new ThunderSlash(Card::Club, 8)

        << new Jink(Card::Heart, 2)
        << new Jink(Card::Heart, 11)
        << new Jink(Card::Heart, 13)

        << new Jink(Card::Diamond, 2)
        << new Jink(Card::Diamond, 3)
        << new Jink(Card::Diamond, 6)
        << new Jink(Card::Diamond, 7)
        << new Jink(Card::Diamond, 7)
        << new Jink(Card::Diamond, 8)
        << new Jink(Card::Diamond, 8)
        << new Jink(Card::Diamond, 9)
        << new Jink(Card::Diamond, 10)
        << new Jink(Card::Diamond, 11)
        << new Jink(Card::Diamond, 13)

        << new Peach(Card::Heart, 4)
        << new Peach(Card::Heart, 6)
        << new Peach(Card::Heart, 7)
        << new Peach(Card::Heart, 8)
        << new Peach(Card::Heart, 9)
        << new Peach(Card::Heart, 10)
        << new Peach(Card::Heart, 12)

        << new Peach(Card::Diamond, 2)

        << new Analeptic(Card::Spade, 9)

        << new Analeptic(Card::Club, 9)

        << new Analeptic(Card::Diamond, 9);

    return cards;
}
