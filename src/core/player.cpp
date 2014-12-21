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

#include "player.h"
#include "engine.h"
#include "room.h"
#include "client.h"
#include "standard-basics.h"
#include "settings.h"

Player::Player(QObject *parent)
    : QObject(parent), general(NULL), general2(NULL),
      headSkinId(0), deputySkinId(0), owner(false),
      m_gender(General::Sexless), hp(-1), max_hp(-1),
      role_shown(false), state("online"), seat(0), alive(true),
      actual_general1(NULL), actual_general2(NULL),
      general1_showed(false), general2_showed(false),
      phase(NotActive),
      weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL), treasure(NULL),
      face_up(true), chained(false), removed(false), scenario_role_shown(false)
{
}

void Player::setScreenName(const QString &screen_name) {
    this->screen_name = screen_name;
}

QString Player::screenName() const{
    return screen_name;
}

bool Player::isOwner() const{
    return owner;
}

void Player::setOwner(bool owner) {
    if (this->owner != owner) {
        this->owner = owner;
        emit owner_changed(owner);
    }
}

bool Player::hasShownRole() const {
    return role_shown;
}

void Player::setShownRole(bool shown) {
    this->role_shown = shown;
}

void Player::setHp(int hp) {
    if (this->hp != hp) {
        this->hp = hp;
        emit hp_changed();
    }
}

int Player::getHp() const{
    return hp;
}

int Player::getMaxHp() const{
    return max_hp;
}

void Player::setMaxHp(int max_hp) {
    if (this->max_hp == max_hp)
        return;
    this->max_hp = max_hp;
    if (hp > max_hp)
        hp = max_hp;
    emit hp_changed();
}

int Player::getLostHp() const{
    return max_hp - qMax(hp, 0);
}

bool Player::isWounded() const{
    if (hp < 0)
        return true;
    else
        return hp < max_hp;
}

General::Gender Player::getGender() const{
    return m_gender;
}

void Player::setGender(General::Gender gender) {
    m_gender = gender;
}

bool Player::isMale() const{
    return m_gender == General::Male;
}

bool Player::isFemale() const{
    return m_gender == General::Female;
}

bool Player::isNeuter() const{
    return m_gender == General::Neuter;
}

int Player::getSeat() const{
    return seat;
}

void Player::setSeat(int seat) {
    this->seat = seat;
}

void Player::setDisableShow(const QString &flags, const QString &reason){
    if (flags.contains('h')){
        if (disableShow(true).contains(reason))
            return;
    }
    if (flags.contains('d')){
        if (disableShow(false).contains(reason))
            return;
    }

    QString dis_str = flags + ',' + reason;
    disable_show << dis_str;
    emit disable_show_changed();
}

void Player::removeDisableShow(const QString &reason){
    QStringList remove_list;
    foreach(QString dis_str, disable_show){
        QString dis_reason = dis_str.split(',').at(1);
        if (dis_reason == reason)
            remove_list << dis_str;
    }

    if (remove_list.isEmpty()) return;

    foreach(QString to_remove, remove_list){
        disable_show.removeOne(to_remove);
    }

    emit disable_show_changed();
}

QStringList Player::disableShow(bool head) const{
    QChar head_flag = 'h';
    if (!head)
        head_flag = 'd';

    QStringList r;
    foreach(QString dis_str, disable_show){
        QStringList dis_list = dis_str.split(',');
        if (dis_list.at(0).contains(head_flag))
            r << dis_list.at(1);
    }

    return r;
}


bool Player::isAdjacentTo(const Player *another) const{
    return getNextAlive() == another
        || another->getNextAlive() == this;
}

bool Player::isAlive() const{
    return alive;
}

bool Player::isDead() const{
    return !alive;
}

void Player::setAlive(bool alive) {
    this->alive = alive;
}

QString Player::getFlags() const{
    return QStringList(flags.toList()).join("|");
}

QStringList Player::getFlagList() const{
    return QStringList(flags.toList());
}

void Player::setFlags(const QString &flag) {
    if (flag == ".") {
        clearFlags();
        return;
    }
    static QChar unset_symbol('-');
    if (flag.startsWith(unset_symbol)) {
        QString copy = flag;
        copy.remove(unset_symbol);
        flags.remove(copy);
    }
    else {
        flags.insert(flag);
    }
}

bool Player::hasFlag(const QString &flag) const{
    return flags.contains(flag);
}

void Player::clearFlags() {
    flags.clear();
}

int Player::getAttackRange(bool include_weapon) const{
    if (hasFlag("InfinityAttackRange") || getMark("InfinityAttackRange") > 0)
        return 1000;

    include_weapon = include_weapon && weapon != NULL;

    int fixeddis = Sanguosha->correctAttackRange(this, include_weapon, true);
    if (fixeddis > 0)
        return fixeddis;

    int original_range = 1, weapon_range = 0;

    if (include_weapon){
        const Weapon *card = qobject_cast<const Weapon *>(weapon->getRealCard());
        Q_ASSERT(card);
        weapon_range = card->getRange();
    }

    int real_range = qMax(original_range, weapon_range) + Sanguosha->correctAttackRange(this, include_weapon, false);

    if (real_range < 0)
        real_range = 0;

    return real_range;
}

bool Player::inMyAttackRange(const Player *other) const{
    if (distanceTo(other) == -1)
        return false;
    return distanceTo(other) <= getAttackRange();
}

void Player::setFixedDistance(const Player *player, int distance) {
    if (distance == -1)
        fixed_distance.remove(player);
    else
        fixed_distance.insert(player, distance);
}

int Player::originalRightDistanceTo(const Player *other) const{
    int right = 0;
    Player *next_p = parent()->findChild<Player *>(objectName());
    while (next_p != other) {
        next_p = next_p->getNextAlive();
        right++;
    }
    return right;
}

int Player::distanceTo(const Player *other, int distance_fix) const{
    if (this == other || isDead() || other->isDead())
        return 0;

    if (isRemoved() || other->isRemoved())
        return -1;

    if (fixed_distance.contains(other))
        return fixed_distance.value(other);

    int right = originalRightDistanceTo(other);
    int left = aliveCount(false) - right;
    int distance = qMin(left, right);

    distance += Sanguosha->correctDistance(this, other);
    distance += distance_fix;

    // keep the distance >=1
    if (distance < 1)
        distance = 1;

    return distance;
}

void Player::setGeneral(const General *new_general) {
    if (this->general != new_general) {
        this->general = new_general;

        if (new_general && kingdom.isEmpty())
            setKingdom(new_general->getKingdom());

        emit general_changed();
    }
}

void Player::setGeneralName(const QString &general_name) {
    const General *new_general = Sanguosha->getGeneral(general_name);
    Q_ASSERT(general_name.isNull() || general_name.isEmpty() || new_general != NULL);
    setGeneral(new_general);
}

QString Player::getGeneralName() const{
    if (general)
        return general->objectName();
    else
        return QString();
}

void Player::setGeneral2Name(const QString &general_name) {
    const General *new_general = Sanguosha->getGeneral(general_name);
    if (general2 != new_general) {
        general2 = new_general;

        emit general2_changed();
    }
}

QString Player::getGeneral2Name() const{
    if (general2)
        return general2->objectName();
    else
        return QString();
}

const General *Player::getGeneral2() const{
    return general2;
}

QString Player::getFootnoteName() const{
    if (general && general->objectName() != "anjiang" && !general->objectName().contains("sujiang"))
        return general->objectName();
    else if (general2 && general2->objectName() != "anjiang" && !general2->objectName().contains("sujiang"))
        return general2->objectName();
    else if (property("UI_Seat").toInt())
        return Sanguosha->translate(QString("SEAT(%1)").arg(QString::number(property("UI_Seat").toInt())));
    else
        return QString();
}

QString Player::getState() const{
    return state;
}

void Player::setState(const QString &state) {
    if (this->state != state) {
        this->state = state;
        emit state_changed();
    }
}

void Player::setRole(const QString &role) {
    if (role.isEmpty()) return;
    if (this->role != role) {
        this->role = role;
        emit role_changed(role);
        if (role == "careerist")
            emit kingdom_changed("careerist");
        else
            emit kingdom_changed(HegemonyMode::GetMappedKingdom(role));
    }
}

QString Player::getRole() const{
    return role;
}

Player::Role Player::getRoleEnum() const{
    static QMap<QString, Role> role_map;
    if (role_map.isEmpty()) {
        role_map.insert("lord", Lord);
        role_map.insert("loyalist", Loyalist);
        role_map.insert("rebel", Rebel);
        role_map.insert("renegade", Renegade);
    }

    return role_map.value(role);
}

const General *Player::getAvatarGeneral() const{
    if (general)
        return general;

    QString general_name = property("avatar").toString();
    if (general_name.isEmpty())
        return NULL;
    return Sanguosha->getGeneral(general_name);
}

const General *Player::getGeneral() const{
    return general;
}

bool Player::isLord() const{
    return getLord() == this;
}

bool Player::hasSkill(const QString &skill_name, bool include_lose) const{
    const TriggerSkill *trigger = Sanguosha->getTriggerSkill(skill_name);
    if (trigger && trigger->isGlobal()) return true;

    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill == NULL)
        return false;

    if (!skill->isVisible()){
        const Skill *main_skill = Sanguosha->getMainSkill(skill_name);
        if (main_skill != NULL)
            return hasSkill(main_skill);
    }

    if (!include_lose && !hasEquipSkill(skill_name) && !getAcquiredSkills().contains(skill_name) && ownSkill(skill_name) && !hasShownSkill(skill_name) && !disableShow(inHeadSkills(skill_name)).isEmpty())
        return false;

    return head_skills.value(skill_name, false)
        || deputy_skills.value(skill_name, false)
        || head_acquired_skills.contains(skill_name)
        || deputy_acquired_skills.contains(skill_name);
}

bool Player::hasSkill(const Skill *skill, bool include_lose) const{
    return hasSkill(skill->objectName(), include_lose);
}

bool Player::hasSkills(const QString &skill_name, bool include_lose) const{
    foreach(QString skill, skill_name.split("|")) {
        bool checkpoint = true;
        foreach(QString sk, skill.split("+")) {
            if (!hasSkill(sk, include_lose)) {
                checkpoint = false;
                break;
            }
        }
        if (checkpoint) return true;
    }
    return false;
}

bool Player::hasInnateSkill(const QString &skill_name) const{
    if (general && general->hasSkill(skill_name))
        return true;

    if (general2 && general2->hasSkill(skill_name))
        return true;

    return false;
}

bool Player::hasLordSkill(const QString &skill_name, bool include_lose) const{
    const Skill *skill = Sanguosha->getSkill(skill_name);

    return skill && skill->isLordSkill() && isLord() && hasSkill(skill_name, include_lose);
}

void Player::acquireSkill(const QString &skill_name, bool head) {
    QSet<QString> &skills = head ? head_acquired_skills : deputy_acquired_skills;
    skills.insert(skill_name);
}

void Player::detachSkill(const QString &skill_name) {
    head_acquired_skills.remove(skill_name);
    deputy_acquired_skills.remove(skill_name);
}

void Player::detachAllSkills() {
    head_acquired_skills.clear();
    deputy_acquired_skills.clear();
}

void Player::addSkill(const QString &skill_name, bool head_skill) {
    const Skill *skill = Sanguosha->getSkill(skill_name);
    Q_ASSERT(skill);
    if (head_skill)
        head_skills[skill_name] = !skill->canPreshow() || general1_showed;
    else
        deputy_skills[skill_name] = !skill->canPreshow() || general2_showed;
}

void Player::loseSkill(const QString &skill_name) {
    head_skills.remove(skill_name);
    deputy_skills.remove(skill_name);
}

QString Player::getPhaseString() const{
    switch (phase) {
    case RoundStart: return "round_start";
    case Start: return "start";
    case Judge: return "judge";
    case Draw: return "draw";
    case Play: return "play";
    case Discard: return "discard";
    case Finish: return "finish";
    case NotActive:
    default:
        return "not_active";
    }
}

void Player::setPhaseString(const QString &phase_str) {
    static QMap<QString, Phase> phase_map;
    if (phase_map.isEmpty()) {
        phase_map.insert("round_start", RoundStart);
        phase_map.insert("start", Start);
        phase_map.insert("judge", Judge);
        phase_map.insert("draw", Draw);
        phase_map.insert("play", Play);
        phase_map.insert("discard", Discard);
        phase_map.insert("finish", Finish);
        phase_map.insert("not_active", NotActive);
    }

    setPhase(phase_map.value(phase_str, NotActive));
}

void Player::setEquip(WrappedCard *equip) {
    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getRealCard());
    Q_ASSERT(card != NULL);
    switch (card->location()) {
    case EquipCard::WeaponLocation: weapon = equip; break;
    case EquipCard::ArmorLocation: armor = equip; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; break;
    case EquipCard::TreasureLocation: treasure = equip; break;
    }
}

void Player::removeEquip(WrappedCard *equip) {
    const EquipCard *card = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(equip->getId()));
    Q_ASSERT(card != NULL);
    switch (card->location()) {
    case EquipCard::WeaponLocation: weapon = NULL; break;
    case EquipCard::ArmorLocation: armor = NULL; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = NULL; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = NULL; break;
    case EquipCard::TreasureLocation: treasure = NULL; break;
    }
}

bool Player::hasEquip(const Card *card) const{
    Q_ASSERT(card != NULL);
    int weapon_id = -1, armor_id = -1, def_id = -1, off_id = -1, tr_id = -1;
    if (weapon) weapon_id = weapon->getEffectiveId();
    if (armor) armor_id = armor->getEffectiveId();
    if (defensive_horse) def_id = defensive_horse->getEffectiveId();
    if (offensive_horse) off_id = offensive_horse->getEffectiveId();
    if (treasure) tr_id = treasure->getEffectiveId();
    QList<int> ids;
    if (card->isVirtualCard())
        ids << card->getSubcards();
    else
        ids << card->getId();
    if (ids.isEmpty()) return false;
    foreach(int id, ids) {
        if (id != weapon_id && id != armor_id && id != def_id && id != off_id && id != tr_id)
            return false;
    }
    return true;
}

bool Player::hasEquip() const{
    return weapon != NULL || armor != NULL || defensive_horse != NULL || offensive_horse != NULL || treasure != NULL;
}

WrappedCard *Player::getWeapon() const{
    return weapon;
}

WrappedCard *Player::getArmor() const{
    return armor;
}

WrappedCard *Player::getDefensiveHorse() const{
    return defensive_horse;
}

WrappedCard *Player::getOffensiveHorse() const{
    return offensive_horse;
}

WrappedCard *Player::getTreasure() const{
    return treasure;
}

QList<const Card *> Player::getEquips() const{
    QList<const Card *> equips;
    if (weapon)
        equips << weapon;
    if (armor)
        equips << armor;
    if (defensive_horse)
        equips << defensive_horse;
    if (offensive_horse)
        equips << offensive_horse;
    if (treasure)
        equips << treasure;

    return equips;
}

const EquipCard *Player::getEquip(int index) const{
    WrappedCard *equip;
    switch (index) {
    case 0: equip = weapon; break;
    case 1: equip = armor; break;
    case 2: equip = defensive_horse; break;
    case 3: equip = offensive_horse; break;
    case 4: equip = treasure; break;
    default:
        return NULL;
    }
    if (equip != NULL)
        return qobject_cast<const EquipCard *>(equip->getRealCard());

    return NULL;
}

bool Player::hasWeapon(const QString &weapon_name) const{
    if (!weapon || getMark("Equips_Nullified_to_Yourself") > 0) return false;
    if (weapon->objectName() == weapon_name || weapon->isKindOf(weapon_name.toStdString().c_str())) return true;
    const Card *real_weapon = Sanguosha->getEngineCard(weapon->getEffectiveId());
    return real_weapon->objectName() == weapon_name || real_weapon->isKindOf(weapon_name.toStdString().c_str());
}

bool Player::hasArmorEffect(const QString &armor_name) const{
    if (!tag["Qinggang"].toStringList().isEmpty() || getMark("Armor_Nullified") > 0
        || getMark("Equips_Nullified_to_Yourself") > 0)
        return false;
    if (armor_name == "bazhen")
        return armor == NULL && alive && hasSkill("bazhen");
    else {
        if (!armor) return false;
        if (armor->objectName() == armor_name || armor->isKindOf(armor_name.toStdString().c_str())) return true;
        const Card *real_armor = Sanguosha->getEngineCard(armor->getEffectiveId());
        return real_armor->objectName() == armor_name || real_armor->isKindOf(armor_name.toStdString().c_str());
    }
    return false;
}

bool Player::hasTreasure(const QString &treasure_name) const{
    if (!treasure || getMark("Equips_Nullified_to_Yourself") > 0) return false;
    if (treasure->objectName() == treasure_name || treasure->isKindOf(treasure_name.toStdString().c_str())) return true;
    const Card *real_treasure = Sanguosha->getEngineCard(treasure->getEffectiveId());
    return real_treasure->objectName() == treasure_name || real_treasure->isKindOf(treasure_name.toStdString().c_str());
}

QList<const Card *> Player::getJudgingArea() const{
    QList<const Card *>cards;
    foreach(int card_id, judging_area)
        cards.append(Sanguosha->getCard(card_id));
    return cards;
}

QList<int> Player::getJudgingAreaID() const{ //for marshal
    return judging_area;
}

Player::Phase Player::getPhase() const{
    return phase;
}

void Player::setPhase(Phase phase) {
    this->phase = phase;
    emit phase_changed();
}

bool Player::faceUp() const{
    return face_up;
}

void Player::setFaceUp(bool face_up) {
    if (this->face_up != face_up) {
        this->face_up = face_up;
        emit state_changed();
    }
}

QString Player::getKingdom() const{
    if (kingdom.isEmpty() && general)
        return general->getKingdom();
    else
        return kingdom;
}

void Player::setKingdom(const QString &kingdom) {
    if (this->kingdom != kingdom) {
        this->kingdom = kingdom;
        if (role == "careerist") return;
        emit kingdom_changed(kingdom);
    }
}

bool Player::isKongcheng() const{
    return getHandcardNum() == 0;
}

bool Player::isNude() const{
    return isKongcheng() && !hasEquip();
}

bool Player::isAllNude() const{
    return isNude() && judging_area.isEmpty();
}

bool Player::canDiscard(const Player *to, const QString &flags) const{
    static QChar handcard_flag('h');
    static QChar equip_flag('e');
    static QChar judging_flag('j');

    if (flags.contains(handcard_flag) && !to->isKongcheng()) return true;
    if (flags.contains(judging_flag) && !to->getJudgingArea().isEmpty()) return true;
    if (flags.contains(equip_flag) && to->hasEquip()) return true;
    return false;
}

bool Player::canDiscard(const Player *to, int card_id) const{
    if (this == to) {
        if (isJilei(Sanguosha->getCard(card_id)))
            return false;
    }
    return true;
}

void Player::addDelayedTrick(const Card *trick) {
    judging_area << trick->getId();
}

void Player::removeDelayedTrick(const Card *trick) {
    int index = judging_area.indexOf(trick->getId());
    if (index >= 0)
        judging_area.removeAt(index);
}

bool Player::containsTrick(const QString &trick_name) const{
    foreach(int trick_id, judging_area) {
        WrappedCard *trick = Sanguosha->getWrappedCard(trick_id);
        if (trick->objectName() == trick_name)
            return true;
    }
    return false;
}

bool Player::isChained() const{
    return chained;
}

void Player::setChained(bool chained) {
    if (this->chained != chained) {
        Sanguosha->playSystemAudioEffect("chained");
        this->chained = chained;
        emit state_changed();
    }
}

bool Player::canBeChainedBy(const Player *_source) const{
    const Player *source = _source ? _source : this;
    if (isChained()) {
        return true;
    } else {
        if (hasArmorEffect("IronArmor")) {
            QStringList big_kingdoms = source->getBigKingdoms("IronArmor", MaxCardsType::Normal);
            if (!big_kingdoms.isEmpty()) {
                QString kingdom;
                if (!hasShownOneGeneral())
                    return false;
                if (getRole() == "careerist")
                    kingdom = objectName();
                else
                    kingdom = getKingdom();
                if (!big_kingdoms.contains(kingdom))
                    return false;
            }
        }
        return true;
    }
}

bool Player::isRemoved() const{
    return removed;
}

void Player::setRemoved(bool removed) {
    if (this->removed != removed) {
        this->removed = removed;
        emit removedChanged();
    }
}

bool Player::isDuanchang(const bool head) const{
    if (head && property("Duanchang").toStringList().contains("head"))
        return true;
    else if (!head && property("Duanchang").toStringList().contains("deputy"))
        return true;
    else
        return false;
}

void Player::addMark(const QString &mark, int add_num) {
    int value = marks.value(mark, 0);
    value += add_num;
    setMark(mark, value);
}

void Player::removeMark(const QString &mark, int remove_num) {
    int value = marks.value(mark, 0);
    value -= remove_num;
    value = qMax(0, value);
    setMark(mark, value);
}

void Player::setMark(const QString &mark, int value) {
    if (marks[mark] != value)
        marks[mark] = value;
}

int Player::getMark(const QString &mark) const{
    return marks.value(mark, 0);
}

bool Player::canSlash(const Player *other, const Card *slash, bool distance_limit,
    int rangefix, const QList<const Player *> &others) const{
    if (other == this || !other->isAlive())
        return false;

    Slash *newslash = new Slash(Card::NoSuit, 0);
    newslash->deleteLater();

    if (isProhibited(other, slash == NULL ? newslash : slash, others))
        return false;

    int distance = distanceTo(other, rangefix);
    if (distance == -1)
        return false;

    if (distance_limit)
        return distance <= getAttackRange() + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, this, slash == NULL ? newslash : slash);
    else
        return true;
}

bool Player::canSlash(const Player *other, bool distance_limit, int rangefix, const QList<const Player *> &others) const{
    return canSlash(other, NULL, distance_limit, rangefix, others);
}

int Player::getCardCount(bool include_equip) const{
    int count = getHandcardNum();
    if (include_equip) {
        if (weapon != NULL) count++;
        if (armor != NULL) count++;
        if (defensive_horse != NULL) count++;
        if (offensive_horse != NULL) count++;
        if (treasure != NULL) count++;
    }
    return count;
}

QList<int> Player::getPile(const QString &pile_name) const{
    return piles[pile_name];
}

QStringList Player::getPileNames() const{
    QStringList names;
    foreach(QString pile_name, piles.keys())
        names.append(pile_name);
    return names;
}

QString Player::getPileName(int card_id) const {
    foreach(QString pile_name, piles.keys()) {
        QList<int> pile = piles[pile_name];
        if (pile.contains(card_id))
            return pile_name;
    }

    return QString();
}

bool Player::pileOpen(const QString &pile_name, const QString &player) const {
    return pile_open[pile_name].contains(player);
}

void Player::setPileOpen(const QString &pile_name, const QString &player) {
    if (pile_open[pile_name].contains(player)) return;
    pile_open[pile_name].append(player);
}

void Player::addHistory(const QString &name, int times) {
    history[name] += times;
}

int Player::getSlashCount() const{
    return history.value("Slash", 0)
        + history.value("ThunderSlash", 0)
        + history.value("FireSlash", 0);
}

void Player::clearHistory(const QString &name) {
    if (name.isEmpty())
        history.clear();
    else
        history.remove(name);
}

bool Player::hasUsed(const QString &card_class) const{
    return history.value(card_class, 0) > 0;
}

int Player::usedTimes(const QString &card_class) const{
    return history.value(card_class, 0);
}

bool Player::hasEquipSkill(const QString &skill_name) const{
    if (weapon) {
        const Weapon *weaponc = qobject_cast<const Weapon *>(weapon->getRealCard());
        if (Sanguosha->getSkill(weaponc) && Sanguosha->getSkill(weaponc)->objectName() == skill_name)
            return true;
    }
    if (armor) {
        const Armor *armorc = qobject_cast<const Armor *>(armor->getRealCard());
        if (Sanguosha->getSkill(armorc) && Sanguosha->getSkill(armorc)->objectName() == skill_name)
            return true;
    }
    if (treasure) {
        const Treasure *treasurec = qobject_cast<const Treasure *>(treasure->getRealCard());
        if (Sanguosha->getSkill(treasurec) && Sanguosha->getSkill(treasurec)->objectName() == skill_name)
            return true;
    }
    return false;
}

QSet<const TriggerSkill *> Player::getTriggerSkills() const{
    QSet<const TriggerSkill *> skillList;
    const QList<QString> skills = head_skills.keys() + deputy_skills.keys() + head_acquired_skills.toList() + deputy_acquired_skills.toList();

    foreach(QString skill_name, skills) {
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if (skill && !hasEquipSkill(skill->objectName()))
            skillList << skill;
    }

    return skillList;
}

QSet<const Skill *> Player::getSkills(bool include_equip, bool visible_only) const{
    return getSkillList(include_equip, visible_only).toSet();
}

QList<const Skill *> Player::getSkillList(bool include_equip, bool visible_only) const{
    QList<const Skill *> skillList;
    const QList<QString> skills = head_skills.keys() + deputy_skills.keys() + head_acquired_skills.toList() + deputy_acquired_skills.toList();

    foreach(QString skill_name, skills) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill != NULL) {
            if ((include_equip || !hasEquipSkill(skill->objectName())) && (!visible_only || skill->isVisible()))
                skillList << skill;
            if (skill->isVisible() && !visible_only) {
                QList<const Skill *> related_skill = Sanguosha->getRelatedSkills(skill->objectName());
                foreach(const Skill *s, related_skill)
                    if (!skillList.contains(s) && !s->isVisible())
                        skillList << s;
            }
        }
    }

    return skillList;
}

QList<const Skill *> Player::getHeadSkillList(bool visible_only) const{
    QList<const Skill *> skillList;

    foreach(QString skill_name, head_skills.keys()) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill
            && (!hasEquipSkill(skill->objectName()))
            && (!visible_only || skill->isVisible()))
            skillList << skill;
    }

    return skillList;
}

QList<const Skill *> Player::getDeputySkillList(bool visible_only) const{
    QList<const Skill *> skillList;

    foreach(QString skill_name, deputy_skills.keys()) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill
            && (!hasEquipSkill(skill->objectName()))
            && (!visible_only || skill->isVisible()))
            skillList << skill;
    }

    return skillList;
}

QSet<const Skill *> Player::getVisibleSkills(bool include_equip) const{
    return getVisibleSkillList(include_equip).toSet();
}

QList<const Skill *> Player::getVisibleSkillList(bool include_equip) const{
    return getSkillList(include_equip, true);
}

QSet<QString> Player::getAcquiredSkills() const{
    return head_acquired_skills + deputy_acquired_skills;
}

QString Player::getSkillDescription(bool inToolTip) const{
    QString description = QString();
    QString color = inToolTip ? Config.SkillDescriptionInToolTipColor.name() : Config.SkillDescriptionInOverviewColor.name();

    foreach(const Skill *skill, getVisibleSkillList()) {
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription(inToolTip);
        desc.replace("\n", "<br/>");
        description.append(QString("<font color=%1><b>%2</b>:</font> %3 <br/> <br/>").arg(color).arg(skill_name).arg(desc));
    }

    if (description.isEmpty()) description = tr("<font color=%1>No skills</font>").arg(color);
    return description;
}

QString Player::getHeadSkillDescription() const {
    QString description = QString();

    foreach(const Skill *skill, getVisibleSkillList()) {
        if (!inHeadSkills(skill->objectName())) continue;
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<font color=%1><b>%2</b>:</font> %3 <br/> <br/>").arg(Config.SkillDescriptionInToolTipColor.name()).arg(skill_name).arg(desc));
    }
    return description;
}

QString Player::getDeputySkillDescription() const {
    QString description = QString();
    foreach(const Skill *skill, getVisibleSkillList()) {
        if (inHeadSkills(skill->objectName())) continue;
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<font color=%1><b>%2</b>:</font> %3 <br/> <br/>").arg(Config.SkillDescriptionInToolTipColor.name()).arg(skill_name).arg(desc));
    }
    return description;
}


bool Player::isProhibited(const Player *to, const Card *card, const QList<const Player *> &others) const{
    return Sanguosha->isProhibited(this, to, card, others);
}

bool Player::canSlashWithoutCrossbow(const Card *slash) const{
    Slash *newslash = new Slash(Card::NoSuit, 0);
    newslash->deleteLater();
#define THIS_SLASH (slash == NULL ? newslash : slash)
    int slash_count = getSlashCount();
    int valid_slash_count = 1;
    valid_slash_count += Sanguosha->correctCardTarget(TargetModSkill::Residue, this, THIS_SLASH);
    return slash_count < valid_slash_count;
#undef THIS_SLASH
}

void Player::setCardLimitation(const QString &limit_list, const QString &pattern, bool single_turn) {
    QStringList limit_type = limit_list.split(",");
    QString _pattern = pattern;
    if (!pattern.endsWith("$1") && !pattern.endsWith("$0")) {
        QString symb = single_turn ? "$1" : "$0";
        _pattern = _pattern + symb;
    }
    foreach(QString limit, limit_type) {
        Card::HandlingMethod method = Sanguosha->getCardHandlingMethod(limit);
        card_limitation[method] << _pattern;
    }
}

void Player::removeCardLimitation(const QString &limit_list, const QString &pattern) {
    QStringList limit_type = limit_list.split(",");
    QString _pattern = pattern;
    if (!_pattern.endsWith("$1") && !_pattern.endsWith("$0"))
        _pattern = _pattern + "$0";
    foreach(QString limit, limit_type) {
        Card::HandlingMethod method = Sanguosha->getCardHandlingMethod(limit);
        card_limitation[method].removeOne(_pattern);
    }
}

void Player::clearCardLimitation(bool single_turn) {
    QList<Card::HandlingMethod> limit_type;
    limit_type << Card::MethodUse << Card::MethodResponse << Card::MethodDiscard
        << Card::MethodRecast << Card::MethodPindian;
    foreach(Card::HandlingMethod method, limit_type) {
        QStringList limit_patterns = card_limitation[method];
        foreach(QString pattern, limit_patterns) {
            if (!single_turn || pattern.endsWith("$1"))
                card_limitation[method].removeAll(pattern);
        }
    }
}

bool Player::isCardLimited(const Card *card, Card::HandlingMethod method, bool isHandcard) const{
    if (method == Card::MethodNone)
        return false;
    if (card->getTypeId() == Card::TypeSkill && method == card->getHandlingMethod()) {
        foreach(int card_id, card->getSubcards()) {
            const Card *c = Sanguosha->getCard(card_id);
            foreach(QString pattern, card_limitation[method]) {
                QString _pattern = pattern.split("$").first();
                if (isHandcard)
                    _pattern.replace("hand", ".");
                ExpPattern p(_pattern);
                if (p.match(this, c)) return true;
            }
        }
    }
    else {
        foreach(QString pattern, card_limitation[method]) {
            QString _pattern = pattern.split("$").first();
            if (isHandcard)
                _pattern.replace("hand", ".");
            ExpPattern p(_pattern);
            if (p.match(this, card)) return true;
        }
    }

    return false;
}

void Player::addQinggangTag(const Card *card) {
    QStringList qinggang = this->tag["Qinggang"].toStringList();
    QString card_string = card->toString();
    if (!qinggang.contains(card_string))
        qinggang << card_string;
    this->tag["Qinggang"] = QVariant::fromValue(qinggang);
}

void Player::removeQinggangTag(const Card *card) {
    QStringList qinggang = this->tag["Qinggang"].toStringList();
    if (!qinggang.isEmpty()) {
        qinggang.removeOne(card->toString());
        this->tag["Qinggang"] = qinggang;
    }
}

const Player *Player::getLord(bool include_death) const{
    if (getActualGeneral1() && getActualGeneral1()->isLord())
        return this;
    QList<const Player *> sib = include_death ? getSiblings() : getAliveSiblings();
    sib << this;
    foreach(const Player *p, sib){
        if (p->getGeneral() && p->getGeneral()->isLord() && p->getKingdom() == kingdom)
            return p;
    }

    return NULL;
}

void Player::copyFrom(Player *p) {
    Player *b = this;
    Player *a = p;

    b->marks = QMap<QString, int>(a->marks);
    b->piles = QMap<QString, QList<int> >(a->piles);
    b->head_acquired_skills = QSet<QString>(a->head_acquired_skills);
    b->deputy_acquired_skills = QSet<QString>(a->deputy_acquired_skills);
    b->flags = QSet<QString>(a->flags);
    b->history = QHash<QString, int>(a->history);
    b->m_gender = a->m_gender;

    b->hp = a->hp;
    b->max_hp = a->max_hp;
    b->kingdom = a->kingdom;
    b->role = a->role;
    b->seat = a->seat;
    b->alive = a->alive;

    b->phase = a->phase;
    b->weapon = a->weapon;
    b->armor = a->armor;
    b->defensive_horse = a->defensive_horse;
    b->offensive_horse = a->offensive_horse;
    b->treasure = a->treasure;
    b->face_up = a->face_up;
    b->chained = a->chained;
    b->judging_area = QList<int>(a->judging_area);
    b->fixed_distance = QHash<const Player *, int>(a->fixed_distance);
    b->card_limitation = QMap<Card::HandlingMethod, QStringList>(a->card_limitation);

    b->tag = QVariantMap(a->tag);
}

QList<const Player *> Player::getSiblings() const{
    QList<const Player *> siblings;
    if (parent()) {
        siblings = parent()->findChildren<const Player *>();
        siblings.removeOne(this);
    }
    return siblings;
}

QList<const Player *> Player::getAliveSiblings() const{
    QList<const Player *> siblings = getSiblings();
    foreach(const Player *p, siblings) {
        if (!p->isAlive())
            siblings.removeOne(p);
    }
    return siblings;
}

bool Player::hasShownSkill(const Skill *skill) const{
    if (skill == NULL)
        return false;

    if (head_acquired_skills.contains(skill->objectName()) || deputy_acquired_skills.contains(skill->objectName()))
        return true;

    if (skill->inherits("ArmorSkill") || skill->inherits("WeaponSkill") || skill->inherits("TreasureSkill"))
        return true;

    if (skill->inherits("TriggerSkill")) {
        const TriggerSkill *tr_skill = qobject_cast<const TriggerSkill *>(skill);
        if (tr_skill && tr_skill->isGlobal())
            return true;
    }

    if (!skill->isVisible()){
        const Skill *main_skill = Sanguosha->getMainSkill(skill->objectName());
        if (main_skill != NULL)
            return hasShownSkill(main_skill);
        else
            return false;
    }

    if (general1_showed && head_skills.contains(skill->objectName()))
        return true;
    else if (general2_showed && deputy_skills.contains(skill->objectName()))
        return true;
    return false;
}

bool Player::hasShownSkill(const QString &skill_name) const{
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill == NULL){
        QObject *roomObject = Sanguosha->currentRoomObject();
        if (roomObject != NULL && roomObject->inherits("Room")){
            Room *room = Sanguosha->currentRoom();
            room->output("no such skill " + skill_name);
            qWarning("%s", QString("no such skill " + skill_name).toStdString().c_str());
        }
        return false;
    }
    return hasShownSkill(skill);
}

bool Player::hasShownSkills(const QString &skill_name) const{
    foreach(QString skill, skill_name.split("|")) {
        bool checkpoint = true;
        foreach(QString sk, skill.split("+")) {
            if (!hasShownSkill(sk)) {
                checkpoint = false;
                break;
            }
        }
        if (checkpoint) return true;
    }
    return false;
}

void Player::preshowSkill(const QString &skill_name) {
    if (hasShownSkill(skill_name))
        return;

    if (head_skills.contains(skill_name))
        head_skills[skill_name] = !head_skills.value(skill_name);
    if (deputy_skills.contains(skill_name))
        deputy_skills[skill_name] = !deputy_skills.value(skill_name);
}

bool Player::inHeadSkills(const QString &skill_name) const {
    return head_skills.contains(skill_name) || head_acquired_skills.contains(skill_name);
}

bool Player::inHeadSkills(const Skill *skill) const {
    if (skill == NULL)
        return false;

    return inHeadSkills(skill->objectName());
}

bool Player::inDeputySkills(const QString &skill_name) const {
    return deputy_skills.contains(skill_name) || deputy_acquired_skills.contains(skill_name);
}

bool Player::inDeputySkills(const Skill *skill) const {
    if (skill == NULL)
        return false;

    return inDeputySkills(skill->objectName());
}

const General *Player::getActualGeneral1() const {
    return actual_general1;
}

const General *Player::getActualGeneral2() const {
    return actual_general2;
}

QString Player::getActualGeneral1Name() const {
    if (actual_general1)
        return actual_general1->objectName();
    else
        return QString();
}

QString Player::getActualGeneral2Name() const {
    if (actual_general2)
        return actual_general2->objectName();
    else
        return QString();
}

void Player::setActualGeneral1(const General *general) {
    actual_general1 = general;
}

void Player::setActualGeneral2(const General *general) {
    actual_general2 = general;
}

void Player::setActualGeneral1Name(const QString &name) {
    const General *general = Sanguosha->getGeneral(name);
    Q_ASSERT(!(name.isNull() || name.isEmpty() || general == NULL));
    setActualGeneral1(general);
}

void Player::setActualGeneral2Name(const QString &name) {
    const General *general = Sanguosha->getGeneral(name);
    Q_ASSERT(!(name.isNull() || name.isEmpty() || general == NULL));
    setActualGeneral2(general);
}

bool Player::hasShownGeneral1() const {
    return general1_showed;
}

bool Player::hasShownGeneral2() const {
    return general2_showed;
}

bool Player::hasShownOneGeneral() const {
    return scenario_role_shown || general1_showed || (general2 && general2_showed);
}

bool Player::hasShownAllGenerals() const {
    return general1_showed && (!general2 || general2_showed);
}

void Player::setGeneral1Showed(bool showed) {
    this->general1_showed = showed;
    emit head_state_changed();
}

void Player::setGeneral2Showed(bool showed) {
    this->general2_showed = showed;
    emit deputy_state_changed();
}

void Player::setSkillPreshowed(const QString &skill, bool preshowed) {
    if (head_skills.contains(skill))
        head_skills[skill] = preshowed;
    else if (deputy_skills.contains(skill))
        deputy_skills[skill] = preshowed;
}

void Player::setSkillsPreshowed(const QString &flags, bool preshowed) {
    if (flags.contains("h")) {
        foreach(QString skill, head_skills.keys()) {
            if (!Sanguosha->getSkill(skill)->canPreshow()) continue;
            head_skills[skill] = preshowed;
        }
    }
    if (flags.contains("d")) {
        foreach(QString skill, deputy_skills.keys()) {
            if (!Sanguosha->getSkill(skill)->canPreshow()) continue;
            deputy_skills[skill] = preshowed;
        }
    }
}

bool Player::hasPreshowedSkill(const QString &name) const {
    return head_skills.value(name, false) || deputy_skills.value(name, false);
}

bool Player::hasPreshowedSkill(const Skill *skill) const{
    return hasPreshowedSkill(skill->objectName());
}

bool Player::isHidden(const bool &head_general) const {
    if (head_general ? general1_showed : general2_showed) return false;
    const QList<const Skill *> skills = head_general ? getHeadSkillList() : getDeputySkillList();
    int count = 0;
    foreach(const Skill *skill, skills) {
        if (skill->canPreshow() && hasPreshowedSkill(skill->objectName()))
            return false;
        else if (!skill->canPreshow())
            ++count;
    }
    return count != skills.length();
}

bool Player::ownSkill(const QString &skill_name) const {
    return head_skills.contains(skill_name) || deputy_skills.contains(skill_name);
}

bool Player::ownSkill(const Skill *skill) const{
    return ownSkill(skill->objectName());
}

bool Player::isFriendWith(const Player *player) const {
    Q_ASSERT(player);
    if (player == NULL)
        return false;
    if (!hasShownOneGeneral() || !player->hasShownOneGeneral())
        return false;

    if (this == player)
        return true;

    if (role == "careerist" || player->role == "careerist")
        return false;

    return kingdom == player->kingdom;
}

bool Player::willBeFriendWith(const Player *player) const
{
    if (this == player)
        return true;
    if (player == NULL)
        return false;
    if (!player->hasShownOneGeneral())
        return false;
    if (!hasShownOneGeneral()) {
        QString kingdom = getActualGeneral1()->getKingdom();
        int i = 1;
        bool has_lord = isAlive() && isLord();

        if (!has_lord) {
            foreach(const Player *p, getSiblings()) {
                if (p->getKingdom() == kingdom) {
                    if (p->isAlive() && p->isLord()) {
                        has_lord = true;
                        break;
                    }
                    if (p->hasShownOneGeneral() && p->getRole() != "careerist")
                        ++i;
                }
            }
        }

        if (!has_lord && i > (parent()->findChildren<const Player *>().length() / 2))
            return false;
        else if (kingdom == player->getKingdom())
            return true;
    }
    return false;
}

void Player::setNext(Player *next)
{
    this->next = next->objectName();
}

void Player::setNext(const QString &next)
{
    this->next = next;
}

Player *Player::getNext(bool ignoreRemoved) const
{
    Player *next_p = parent()->findChild<Player *>(next);
    if (ignoreRemoved && next_p->isRemoved())
        return next_p->getNext(ignoreRemoved);
    return next_p;
}

QString Player::getNextName() const
{
    return next;
}

Player *Player::getLast(bool ignoreRemoved) const
{
    foreach(Player *p, parent()->findChildren<Player *>())
        if (p->getNext(ignoreRemoved) == this)
            return p;
    return NULL;
}

Player *Player::getNextAlive(int n, bool ignoreRemoved) const
{
    bool hasAlive = (aliveCount(!ignoreRemoved) > 0);
    Player *next = parent()->findChild<Player *>(objectName());
    if (!hasAlive) return next;
    for (int i = 0; i < n; ++i) {
        do next = next->getNext(ignoreRemoved);
        while (next->isDead());
    }
    return next;
}

Player *Player::getLastAlive(int n, bool ignoreRemoved) const
{
    return getNextAlive(aliveCount(!ignoreRemoved) - n, ignoreRemoved);
}

QList<const Player *> Player::getFormation() const
{
    QList<const Player *> teammates;
    teammates << this;
    int n = aliveCount(false);
    int num = n;
    for (int i = 1; i < n; ++i) {
        Player *target = getNextAlive(i);
        if (isFriendWith(target))
            teammates << target;
        else {
            num = i;
            break;
        }
    }

    n -= num;
    for (int i = 1; i < n; ++i) {
        Player *target = getLastAlive(i);
        if (isFriendWith(target))
            teammates << target;
        else break;
    }

    return teammates;
}

void Player::setHeadSkinId(int id)
{
    headSkinId = id;
}

int Player::getHeadSkinId() const
{
    return headSkinId;
}

void Player::setDeputySkinId(int id)
{
    deputySkinId = id;
}

int Player::getDeputySkinId() const
{
    return deputySkinId;
}
