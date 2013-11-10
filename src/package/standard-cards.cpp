#include "standard.h"
#include "standard-equips.h"
#include "maneuvering.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "jsonutils.h"
#include "protocol.h"

Slash::Slash(Suit suit, int number): BasicCard(suit, number)
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
            } else {
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
    } else {
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

    /* actually it's not proper to put the codes here.
       considering the nasty design of the client and the convenience as well,
       I just move them here */
    if (objectName() == "slash" && use.m_isOwnerUse) {
        bool has_changed = false;
        QString skill_name = getSkillName();
        if (!skill_name.isEmpty()) {
            const Skill *skill = Sanguosha->getSkill(skill_name);
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
    if (use.card->isVirtualCard() && use.card->subcardsLength() == 0
        && !player->hasFlag("slashDisableExtraTarget")) {
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

    if (player->getPhase() == Player::Play && player->hasFlag("Global_MoreSlashInOneTurn")) {
        if (player->hasSkill("paoxiao")) {
            player->setFlags("-Global_MoreSlashInOneTurn");
            room->broadcastSkillInvoke("paoxiao");
            room->notifySkillInvoked(player, "paoxiao");
        }
    }
    if (use.to.size() > 1 && player->hasSkill("duanbing")) {
        room->broadcastSkillInvoke("duanbing");
        room->notifySkillInvoked(player, "duanbing");
    }

    if (use.card->isVirtualCard()) {
        if (use.card->getSkillName() == "spear")
            room->setEmotion(player, "weapon/spear");
        else if (use.card->getSkillName() == "fan")
            room->setEmotion(player, "weapon/fan");
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

    QVariantList jink_list = effect.from->tag["Jink_" + toString()].toList();
    effect.jink_num = jink_list.takeFirst().toInt();
    if (jink_list.isEmpty())
        effect.from->tag.remove("Jink_" + toString());
    else
        effect.from->tag["Jink_" + toString()] = QVariant::fromValue(jink_list);

    room->slashEffect(effect);
}

bool Slash::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return !targets.isEmpty();
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
        ++ rangefix;

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
    if (targets.length() >= slash_targets) {
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
            return !duanbing_targets.isEmpty() || Self->distanceTo(to_select, rangefix) == 1;
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

ThunderSlash::ThunderSlash(Suit suit, int number)
    : NatureSlash(suit, number, DamageStruct::Thunder)
{
    setObjectName("thunder_slash");
}

FireSlash::FireSlash(Suit suit, int number)
    : NatureSlash(suit, number, DamageStruct::Fire)
{
    setObjectName("fire_slash");
}

Jink::Jink(Suit suit, int number): BasicCard(suit, number)
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

Peach::Peach(Suit suit, int number): BasicCard(suit, number) {
    setObjectName("peach");
    target_fixed = true;
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

Analeptic::Analeptic(Card::Suit suit, int number)
    : BasicCard(suit, number)
{
    setObjectName("analeptic");
    target_fixed = true;
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

void Analeptic::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    BasicCard::use(room, source, targets);
    if (targets.isEmpty())
        room->cardEffect(this, source, source);
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
    } else {
        room->addPlayerMark(effect.to, "drank");
    }
}

Crossbow::Crossbow(Suit suit, int number)
    : Weapon(suit, number, 1)
{
    setObjectName("Crossbow");
}

class DoubleSwordSkill: public WeaponSkill {
public:
    DoubleSwordSkill(): WeaponSkill("DoubleSword") {
        events << TargetConfirmed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return WeaponSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from != player)
            return false;

        foreach (ServerPlayer *to, use.to) {
            if (((use.from->isMale() && to->isFemale()) || (use.from->isFemale() && to->isMale()))
                && use.card->isKindOf("Slash")) {
                if (use.from->askForSkillInvoke(objectName())) {
                    room->setEmotion(use.from, "weapon/double_sword");

                    bool draw_card = false;
                    if (!to->canDiscard(to, "h"))
                        draw_card = true;
                    else {
                        QString prompt = "double-sword-card:" + use.from->objectName();
                        const Card *card = room->askForCard(to, ".", prompt, data);
                        if (!card) draw_card = true;
                    }
                    if (draw_card)
                       use.from->drawCards(1);
                }
            }
        }

        return false;
    }
};

DoubleSword::DoubleSword(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("DoubleSword");
}

class QinggangSwordSkill: public WeaponSkill {
public:
    QinggangSwordSkill(): WeaponSkill("QinggangSword") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (WeaponSkill::triggerable(use.from) && use.from == player && use.card->isKindOf("Slash")) {
            bool do_anim = false;
            foreach (ServerPlayer *p, use.to.toSet()) {
                if (p->getMark("Equips_of_Others_Nullified_to_You") == 0) {
                    do_anim = (p->getArmor() && p->hasArmorEffect(p->getArmor()->objectName())) || p->hasArmorEffect("bazhen");
                    p->addQinggangTag(use.card);
                }
            }
            if (do_anim)
                room->setEmotion(use.from, "weapon/qinggang_sword");
        }
        return false;
    }
};

QinggangSword::QinggangSword(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("QinggangSword");
}

class SpearSkill: public ViewAsSkill {
public:
    SpearSkill(): ViewAsSkill("Spear") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= 2 && Slash::IsAvailable(player)
               && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getHandcardNum() >= 2 && pattern == "slash" && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        Slash *slash = new Slash(Card::SuitToBeDecided, 0);
        slash->setSkillName(objectName());
        slash->addSubcards(cards);

        return slash;
    }
};

Spear::Spear(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("Spear");
}

class AxeViewAsSkill: public ViewAsSkill {
public:
    AxeViewAsSkill(): ViewAsSkill("Axe") {
        response_pattern = "@Axe";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && to_select != Self->getWeapon() && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class AxeSkill: public WeaponSkill {
public:
    AxeSkill(): WeaponSkill("Axe") {
        events << SlashMissed;
        view_as_skill = new AxeViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if (!effect.to->isAlive() || effect.to->getMark("Equips_of_Others_Nullified_to_You") > 0)
            return false;

        const Card *card = NULL;
        if (player->getCardCount(true) >= 3) // Need 2 more cards except from the weapon itself
            card = room->askForCard(player, "@Axe", "@Axe:" + effect.to->objectName(), data, objectName());
        if (card) {
            room->setEmotion(player, "weapon/axe");
            room->slashResult(effect, NULL);
        }

        return false;
    }
};

Axe::Axe(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("Axe");
}

class HalberdSkill: public TargetModSkill {
public:
    HalberdSkill(): TargetModSkill("Halberd") {
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if (from->hasWeapon("Halberd") && from->isLastHandCard(card))
            return 2;
        else
            return 0;
    }
};

class KylinBowSkill: public WeaponSkill {
public:
    KylinBowSkill(): WeaponSkill("KylinBow") {
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        QStringList horses;
        if (damage.card && damage.card->isKindOf("Slash") && damage.by_user && !damage.chain && !damage.transfer
            && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0) {
            if (damage.to->getDefensiveHorse() && damage.from->canDiscard(damage.to, damage.to->getDefensiveHorse()->getEffectiveId()))
                horses << "dhorse";
            if (damage.to->getOffensiveHorse() && damage.from->canDiscard(damage.to, damage.to->getOffensiveHorse()->getEffectiveId()))
                horses << "ohorse";

            if (horses.isEmpty())
                return false;

            if (player == NULL) return false;
            if (!player->askForSkillInvoke(objectName(), data))
                return false;

            room->setEmotion(player, "weapon/kylin_bow");

            QString horse_type = room->askForChoice(player, objectName(), horses.join("+"));

            if (horse_type == "dhorse")
                room->throwCard(damage.to->getDefensiveHorse(), damage.to, damage.from);
            else if (horse_type == "ohorse")
                room->throwCard(damage.to->getOffensiveHorse(), damage.to, damage.from);
        }

        return false;
    }
};

KylinBow::KylinBow(Suit suit, int number)
    : Weapon(suit, number, 5)
{
    setObjectName("KylinBow");
}

class EightDiagramSkill: public ArmorSkill {
public:
    EightDiagramSkill(): ArmorSkill("EightDiagram") {
        events << CardAsked;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toStringList().first();
        if (asked == "jink") {
            if (room->askForSkillInvoke(player, "EightDiagram")) {
                int armor_id = player->getArmor()->getId();
                room->setCardFlag(armor_id, "using");
                room->setEmotion(player, "armor/eight_diagram");
                JudgeStruct judge;
                judge.pattern = ".|red";
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);
                room->setCardFlag(armor_id, "-using");

                if (judge.isGood()) {
                    Jink *jink = new Jink(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);

                    return true;
                }
            }
        }
        return false;
    }

    int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

EightDiagram::EightDiagram(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("EightDiagram");
}

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
    DummyCard *dummy = new DummyCard(VariantList2IntList(ag_list));
    CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "amazing_grace", QString());
    room->throwCard(dummy, reason, NULL);
    delete dummy;
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
    foreach (QVariant card_id, ag_list)
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
                                         "savage-assault-slash:"+ effect.from->objectName(),
                                         QVariant::fromValue(effect),
                                         Card::MethodResponse,
                                         effect.from->isAlive() ? effect.from : NULL);
    if (slash) {
        if (slash->getSkillName() == "spear") room->setEmotion(effect.to, "weapon/spear");
        room->setEmotion(effect.to, "killer");
    } else{
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
    if (jink && jink->getSkillName() != "eight_diagram" && jink->getSkillName() != "bazhen")
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
    foreach (const Player *p, player->getAliveSiblings()) {
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
        // roomscene such that if it is collateral, then targetFilter's result is overrode
        Q_ASSERT(targets.length() <= 2);
        if (targets.length() == 2) return false;
        const Player *slashFrom = targets[0];
        /* @todo: develop a new mechanism of filtering targets
                    to remove the coupling here and to fix the similar bugs caused by TongJi */
        if (to_select == Self && to_select->hasSkill("kongcheng") && Self->isLastHandCard(this, true))
            return false;
        return slashFrom->canSlash(to_select);
    } else {
        if (!to_select->getWeapon() || to_select == Self)
            return false;
        foreach (const Player *p, to_select->getAliveSiblings()) {
            if (to_select->canSlash(p)
                && (!(p == Self && p->hasSkill("kongcheng") && Self->isLastHandCard(this, true))))
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
    killer->tag["collateralVictim"] = QVariant::fromValue((PlayerStar)victim);

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
    ServerPlayer *victim = effect.to->tag["collateralVictim"].value<PlayerStar>();
    effect.to->tag.remove("collateralVictim");
    if (!victim) return;
    WrappedCard *weapon = killer->getWeapon();

    QString prompt = QString("collateral-slash:%1:%2").arg(victim->objectName()).arg(source->objectName());

    if (victim->isDead()) {
        if (source->isAlive() && killer->isAlive() && killer->getWeapon())
            source->obtainCard(weapon);
    }
    else if (source->isDead()) {
        if (killer->isAlive())
            doCollateral(room, killer, victim, prompt);
    } else {
        if (killer->isDead()) {
            ; // do nothing
        } else if (!killer->getWeapon()) {
            doCollateral(room, killer, victim, prompt);
        } else {
            if (!doCollateral(room, killer, victim, prompt)) {
                if (killer->getWeapon())
                    source->obtainCard(weapon);
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
    CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
    room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);
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

    forever {
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
        } else {
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
        ++ rangefix;

    if (Self->distanceTo(to_select, rangefix) > distance_limit)
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
    if (Self->isCardLimited(this, Card::MethodUse))
        return targets.length() == 0;
    int total_num = 2 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (getSkillName().contains("guhuo") || getSkillName() == "qice")
        return targets.length() > 0 && targets.length() <= total_num;
    else
        return targets.length() <= total_num;
}

void IronChain::onUse(Room *room, const CardUseStruct &card_use) const{
    if (card_use.to.isEmpty()) {
        CardMoveReason reason(CardMoveReason::S_REASON_RECAST, card_use.from->objectName());
        reason.m_skillName = this->getSkillName();
        room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason);
        card_use.from->broadcastSkillInvoke("@recast");

        LogMessage log;
        log.type = "#Card_Recast";
        log.from = card_use.from;
        log.card_str = card_use.card->toString();
        room->sendLog(log);

        card_use.from->drawCards(1);
    } else
        TrickCard::onUse(room, card_use);
}

void IronChain::onEffect(const CardEffectStruct &effect) const{
    effect.to->setChained(!effect.to->isChained());

    Room *room = effect.to->getRoom();

    room->broadcastProperty(effect.to, "chained");
    room->setEmotion(effect.to, "chain");
    room->getThread()->trigger(ChainStateChanged, room, effect.to);
}

AwaitExhausted::AwaitExhausted(Card::Suit suit, int number): TrickCard(suit, number){
    setObjectName("await_exhausted");
    target_fixed = true;
}


QString AwaitExhausted::getSubtype() const{
    return "await_exhausted";
}

void AwaitExhausted::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct new_use = card_use;
    new_use.to.append(new_use.from);
    foreach (ServerPlayer *p, room->getOtherPlayers(new_use.from)) {
        if (p->isFriendWith(new_use.from))
            new_use.to.append(p);
    }

    if (getSkillName() == "neo2013duoshi")
        room->addPlayerHistory(card_use.from, "NeoDuoshiAE", 1);

    TrickCard::onUse(room, new_use);
}

void AwaitExhausted::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *target, targets) {
        CardEffectStruct effect;
        effect.card = this;
        effect.from = source;
        effect.to = target;

        room->cardEffect(effect);
    }

    foreach (ServerPlayer *target, targets) {
        if (target->hasFlag("AwaitExhaustedEffected")) {
            room->setPlayerFlag(target, "-AwaitExhaustedEffected");
            room->askForDiscard(target, objectName(), 2, 2, false, true);
        }
    }

    if (room->getCardPlace(getEffectiveId()) == Player::PlaceTable) {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName(), QString(), this->getSkillName(), QString());
        if (targets.size() == 1) reason.m_targetId = targets.first()->objectName();
        room->moveCardTo(this, source, NULL, Player::DiscardPile, reason, true);
    }
}

void AwaitExhausted::onEffect(const CardEffectStruct &effect) const {
    effect.to->drawCards(2);
    effect.to->getRoom()->setPlayerFlag(effect.to, "AwaitExhaustedEffected");
}

KnownBoth::KnownBoth(Card::Suit suit, int number): SingleTargetTrick(suit, number){
    setObjectName("known_both");
    can_recast = true;
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
    if (Self->isCardLimited(this, Card::MethodUse))
        return targets.length() == 0;

    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE)
        return targets.length() != 0;

    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    return targets.length() <= total_num;
}

void KnownBoth::onUse(Room *room, const CardUseStruct &card_use) const{
    if (card_use.to.isEmpty()){
        CardMoveReason reason(CardMoveReason::S_REASON_RECAST, card_use.from->objectName());
        reason.m_skillName = this->getSkillName();
        room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason);
        card_use.from->broadcastSkillInvoke("@recast");

        LogMessage log;
        log.type = "#Card_Recast";
        log.from = card_use.from;
        log.card_str = card_use.card->toString();
        room->sendLog(log);

        card_use.from->drawCards(1);
    }
    else
        SingleTargetTrick::onUse(room, card_use);
}

void KnownBoth::onEffect(const CardEffectStruct &effect) const {
    QStringList choices;
    if (!effect.to->isKongcheng())
        choices << "handcards";
    if (!effect.to->hasShownGeneral1())
        choices << "head_general";
    if (!effect.to->hasShownGeneral2())
        choices << "deputy_general";

    Room *room = effect.from->getRoom();

    QString choice = room->askForChoice(effect.from, objectName(), 
                                        choices.join("+"), QVariant::fromValue(effect.to));

    if (choice == "handcards")
        room->showAllCards(effect.to, effect.from);
    else {
        QStringList list = room->getTag(effect.to->objectName()).toStringList();
        list.removeAt(choice == "head_general"? 1 : 0);
        foreach (QString name, list) {
            LogMessage log;
            log.type = "$KnownBothViewGeneral";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = name;
            room->doNotify(effect.from, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
        }
        Json::Value arg(Json::arrayValue);
        arg[0] = QSanProtocol::Utils::toJsonString(objectName());
        arg[1] = QSanProtocol::Utils::toJsonArray(list);
        room->doNotify(effect.from, QSanProtocol::S_COMMAND_VIEW_GENERALS, arg);
    }
}

BefriendAttacking::BefriendAttacking(Card::Suit suit, int number): SingleTargetTrick(suit, number) {
    setObjectName("befriend_attacking");
}

bool BefriendAttacking::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    return to_select->hasShownOneGeneral() && !Self->isFriendWith(to_select);
}

bool BefriendAttacking::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const {
    return targets.length() > 0;
}

void BefriendAttacking::onEffect(const CardEffectStruct &effect) const {
    effect.to->drawCards(1);
    effect.from->drawCards(3);
}

bool BefriendAttacking::isAvailable(const Player *player) const {
    return player->hasShownOneGeneral();
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
        rangefix += 1;

    if (Self->distanceTo(to_select, rangefix) > distance_limit)
        return false;

    return true;
}

void SupplyShortage::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Draw);
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

Lightning::Lightning(Suit suit, int number):Disaster(suit, number) {
    setObjectName("lightning");

    judge.pattern = ".|spade|2~9";
    judge.good = false;
    judge.reason = objectName();
}

void Lightning::takeEffect(ServerPlayer *target) const{
    target->getRoom()->damage(DamageStruct(this, NULL, target, 3, DamageStruct::Thunder));
}

// EX cards

class IceSwordSkill: public WeaponSkill {
public:
    IceSwordSkill(): WeaponSkill("IceSword") {
        events << DamageCaused;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if (damage.card && damage.card->isKindOf("Slash")
            && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0
            && !damage.to->isNude() && damage.by_user
            && !damage.chain && !damage.transfer && player->askForSkillInvoke("IceSword", data)) {
                room->setEmotion(player, "weapon/ice_sword");
                if (damage.from->canDiscard(damage.to, "he")) {
                    int card_id = room->askForCardChosen(player, damage.to, "he", "IceSword", false, Card::MethodDiscard);
                    room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);

                    if (damage.from->isAlive() && damage.to->isAlive() && damage.from->canDiscard(damage.to, "he")) {
                        card_id = room->askForCardChosen(player, damage.to, "he", "IceSword", false, Card::MethodDiscard);
                        room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);
                    }
                }
                return true;
        }
        return false;
    }
};

IceSword::IceSword(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("IceSword");
}

class RenwangShieldSkill: public ArmorSkill {
public:
    RenwangShieldSkill(): ArmorSkill("RenwangShield") {
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.slash->isBlack()) {
            LogMessage log;
            log.type = "#ArmorNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            player->getRoom()->sendLog(log);

            room->setEmotion(player, "armor/renwang_shield");
            effect.to->setFlags("Global_NonSkillNullify");
            return true;
        } else
            return false;
    }
};

RenwangShield::RenwangShield(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("RenwangShield");
}

class FanSkill: public OneCardViewAsSkill {
public:
    FanSkill(): OneCardViewAsSkill("Fan") {
        filter_pattern = "%slash";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player) && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
            && pattern == "slash" && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *acard = new FireSlash(originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        return acard;
    }
};

Fan::Fan(Suit suit, int number)
    : Weapon(suit, number, 4)
{
    setObjectName("Fan");
}

SixSwords::SixSwords(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("SixSwords");
}

Triblade::Triblade(Card::Suit suit, int number): Weapon(suit, number, 3){
    setObjectName("Triblade");
}

TribladeSkillCard::TribladeSkillCard(): SkillCard(){
    setObjectName("Triblade");
}

bool TribladeSkillCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() == 0 && to_select->hasFlag("TribladeCanBeSelected");
}

void TribladeSkillCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->damage(DamageStruct("Triblade", source, targets[0]));
}

class TribladeSkillVS: public OneCardViewAsSkill{
public:
    TribladeSkillVS(): OneCardViewAsSkill("Triblade"){
        response_pattern = "@@Triblade";
        filter_pattern = ".|.|.|hand!";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TribladeSkillCard *c = new TribladeSkillCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class TribladeSkill: public WeaponSkill{
public:
    TribladeSkill(): WeaponSkill("Triblade"){
        events << Damage;
        view_as_skill = new TribladeSkillVS;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to && damage.to->isAlive() && damage.card && damage.card->isKindOf("Slash")
            && damage.by_user && !damage.chain && !damage.transfer){
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *p, room->getOtherPlayers(damage.to))
                    if (damage.to->distanceTo(p) == 1){
                        players << p;
                        room->setPlayerFlag(p, "TribladeCanBeSelected");
                    }
                    if (players.isEmpty())
                        return false;
                    room->askForUseCard(player, "@@Triblade", "@triblade");
        }

        foreach(ServerPlayer *p, room->getAllPlayers())
            if (p->hasFlag("TribladeCanBeSelected"))
                room->setPlayerFlag(p, "TribladeCanBeSelected");

        return false;
    }
};

class VineSkill: public ArmorSkill {
public:
    VineSkill(): ArmorSkill("Vine") {
        events << DamageInflicted << SlashEffected << CardEffected;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.nature == DamageStruct::Normal) {
                room->setEmotion(player, "armor/vine");
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                room->sendLog(log);

                effect.to->setFlags("Global_NonSkillNullify");
                return true;
            }
        } else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("SavageAssault") || effect.card->isKindOf("ArcheryAttack")) {
                room->setEmotion(player, "armor/vine");
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.card->objectName();
                room->sendLog(log);

                effect.to->setFlags("Global_NonSkillNullify");
                return true;
            }
        } else if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.nature == DamageStruct::Fire) {
                room->setEmotion(player, "armor/vineburn");
                LogMessage log;
                log.type = "#VineDamage";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(++ damage.damage);
                room->sendLog(log);

                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

Vine::Vine(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("Vine");
}

class SilverLionSkill: public ArmorSkill {
public:
    SilverLionSkill(): ArmorSkill("SilverLion") {
        events << DamageInflicted << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == DamageInflicted && ArmorSkill::triggerable(player)) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.damage > 1) {
                room->setEmotion(player, "armor/silver_lion");
                LogMessage log;
                log.type = "#SilverLion";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = objectName();
                room->sendLog(log);

                damage.damage = 1;
                data = QVariant::fromValue(damage);
            }
        } else if (player->hasFlag("SilverLionRecover")) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != player || !move.from_places.contains(Player::PlaceEquip))
                return false;
            for (int i = 0; i < move.card_ids.size(); i++) {
                if (move.from_places[i] != Player::PlaceEquip) continue;
                const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
                if (card->objectName() == objectName()) {
                    player->setFlags("-SilverLionRecover");
                    if (player->isWounded()) {
                        room->setEmotion(player, "armor/silver_lion");
                        RecoverStruct recover;
                        recover.card = card;
                        room->recover(player, recover);
                    }
                    return false;
                }
            }

        }
        return false;
    }
};

SilverLion::SilverLion(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("SilverLion");
}

void SilverLion::onUninstall(ServerPlayer *player) const{
    if (player->isAlive() && player->hasArmorEffect(objectName()))
        player->setFlags("SilverLionRecover");
}

class HorseSkill: public DistanceSkill {
public:
    HorseSkill():DistanceSkill("Horse"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        const Horse *horse = NULL;
        if (from->getOffensiveHorse() && from->getMark("Equips_Nullified_to_Yourself") == 0) {
            horse = qobject_cast<const Horse *>(from->getOffensiveHorse()->getRealCard());
            correct += horse->getCorrect();
        }
        if (to->getDefensiveHorse() && to->getMark("Equips_Nullified_to_Yourself") == 0) {
            horse = qobject_cast<const Horse *>(to->getDefensiveHorse()->getRealCard());
            correct += horse->getCorrect();
        }

        return correct;
    }
};

StandardCardPackage::StandardCardPackage()
    : Package("standard_cards", Package::CardPack)
{
    QList<Card *> cards;

    cards << new Slash(Card::Spade, 5)
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

          << new Analeptic(Card::Diamond, 9)

          << new Crossbow
          << new DoubleSword
          << new QinggangSword
          << new IceSword
          << new Spear
          << new Fan
          << new Axe
          << new KylinBow
          << new SixSwords
          << new Triblade

          << new EightDiagram
          << new RenwangShield
          << new Vine
          << new SilverLion;

    skills << new DoubleSwordSkill << new QinggangSwordSkill << new IceSwordSkill
           << new SpearSkill << new FanSkill << new AxeSkill << new KylinBowSkill 
           << new TribladeSkill << new HalberdSkill << new EightDiagramSkill
           << new RenwangShieldSkill << new VineSkill << new SilverLionSkill;

    QList<Card *> horses;
    horses << new DefensiveHorse(Card::Spade, 5)
           << new DefensiveHorse(Card::Club, 5)
           << new DefensiveHorse(Card::Heart, 13)
           << new OffensiveHorse(Card::Heart, 5)
           << new OffensiveHorse(Card::Spade, 13)
           << new OffensiveHorse(Card::Diamond, 13);

    horses.at(0)->setObjectName("JueYing");
    horses.at(1)->setObjectName("DiLu");
    horses.at(2)->setObjectName("ZhuaHuangFeiDian");
    horses.at(3)->setObjectName("ChiTu");
    horses.at(4)->setObjectName("DaYuan");
    horses.at(5)->setObjectName("ZiXing");

    cards << horses;

    skills << new HorseSkill;

    cards << new AmazingGrace
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

    foreach (Card *card, cards)
        card->setParent(this);
}

StandardExCardPackage::StandardExCardPackage()
    : Package("standard_ex_cards", Package::CardPack)
{
    QList<Card *> cards;
    cards << new IceSword(Card::Spade, 2)
          << new RenwangShield(Card::Club, 2)
          << new Lightning(Card::Heart, 12)
          << new Nullification(Card::Diamond, 12);

    foreach (Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(StandardCard)
ADD_PACKAGE(StandardExCard)

