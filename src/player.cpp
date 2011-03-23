#include "player.h"
#include "engine.h"
#include "room.h"
#include "client.h"
#include "standard.h"
#include "settings.h"

Player::Player(QObject *parent)
    :QObject(parent), owner(false), general(NULL), general2(NULL),
    hp(-1), max_hp(-1), xueyi(0), state("online"), seat(0), alive(true),
    attack_range(1), phase(NotActive),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    face_up(true), chained(false)
{
}

void Player::setScreenName(const QString &screen_name){
    this->screen_name = screen_name;
}

QString Player::screenName() const{
    return screen_name;
}

bool Player::isOwner() const{
    return owner;
}

void Player::setOwner(bool owner){
    if(this->owner != owner){
        this->owner = owner;
        emit owner_changed(owner);
    }
}

void Player::setHp(int hp){
    if(hp >= 0 && hp <= max_hp && this->hp != hp){
        this->hp = hp;
        emit state_changed();
    }
}

int Player::getHp() const{
    return hp;
}

int Player::getMaxHP() const{
    return max_hp;
}

void Player::setMaxHP(int max_hp){
    if(this->max_hp == max_hp)
        return;

    this->max_hp = max_hp;
    if(hp > max_hp)
        hp = max_hp;

    emit state_changed();
}

int Player::getLostHp() const{
    return max_hp - hp;
}

bool Player::isWounded() const{
    if(hp < 0)
        return true;
    else
        return hp < max_hp;
}

int Player::getSeat() const{
    return seat;
}

void Player::setSeat(int seat){
    this->seat = seat;
}

bool Player::isAlive() const{
    return alive;
}

bool Player::isDead() const{
    return !alive;
}

void Player::setAlive(bool alive){
    if(this->alive && alive == false){
        this->alive = alive;
    }
}

QString Player::getFlags() const{
    QStringList flags_list;
    foreach(QString flag, flags)
        flags_list << flag;

    return flags_list.join("+");
}

void Player::setFlags(const QString &flag){
    static QChar unset_symbol('-');
    if(flag.startsWith(unset_symbol)){
        QString copy = flag;
        copy.remove(unset_symbol);
        flags.remove(copy);

        if(copy == "drank")
            emit drank_changed(false);
    }else{
        if(flag == "drank")
            emit drank_changed(true);

        flags.insert(flag);
    }
}

bool Player::hasFlag(const QString &flag) const{
    return flags.contains(flag);
}

void Player::setAttackRange(int attack_range){
    this->attack_range = attack_range;
}

int Player::getAttackRange() const{
    return attack_range;
}

bool Player::inMyAttackRange(const Player *other) const{
    return distanceTo(other) <= attack_range;
}

int Player::distanceTo(const Player *other) const{
    if(this == other)
        return 0;

    if(hasSkill("changqu") && other->getRoleEnum() == Player::Lord)
        return 1;

    int right = qAbs(seat - other->seat);
    int left = aliveCount() - right;
    int distance = qMin(left, right);

    if(getOffensiveHorse())
        distance --;

    if(hasSkill("mashu") || (hasSkill("yicong") && getHp() > 2))
        distance --;

    if(other->getDefensiveHorse())
        distance ++;

    if(other->hasSkill("feiying") || (other->hasSkill("yicong") && other->getHp() <= 2))
        distance ++;

    if(distance < 1)
        distance = 1;
    return distance;
}

int Player::getGeneralMaxHP() const{
    if(general2 == NULL)
        return general->getMaxHp();

    int first = general->getMaxHp();
    int second = general2->getMaxHp();

    int max_hp;
    switch(Config.MaxHpScheme){
    case 2: max_hp = (first + second)/2; break;
    case 1: max_hp = qMin(first, second); break;
    case 0:
    default:
        max_hp = first + second - 3; break;
    }

    return qMin(max_hp, 8);
}

void Player::setGeneral(const General *new_general){
    if(this->general != new_general){
        this->general = new_general;
        if(new_general){
            setKingdom(general->getKingdom());
        }

        emit general_changed();
    }
}

void Player::setGeneralName(const QString &general_name){
    const General *new_general = Sanguosha->getGeneral(general_name);
    setGeneral(new_general);
}

QString Player::getGeneralName() const{
    if(general)
        return general->objectName();
    else
        return "";
}

void Player::setGeneral2Name(const QString &general_name){
    const General *new_general = Sanguosha->getGeneral(general_name);
    if(general2 != new_general){
        general2 = new_general;

        emit general2_changed();
    }
}

QString Player::getGeneral2Name() const{
    if(general2)
        return general2->objectName();
    else
        return "";
}

const General *Player::getGeneral2() const{
    return general2;
}

QString Player::getState() const{
    return state;
}

void Player::setState(const QString &state){
    if(this->state != state){
        this->state = state;
        emit state_changed();
    }
}

void Player::setRole(const QString &role){
    if(this->role != role){
        this->role = role;        
        emit role_changed(role);
    }
}

QString Player::getRole() const{
    return role;
}

Player::Role Player::getRoleEnum() const{
    static QMap<QString, Role> role_map;
    if(role_map.isEmpty()){
        role_map.insert("lord", Lord);
        role_map.insert("loyalist", Loyalist);
        role_map.insert("rebel", Rebel);
        role_map.insert("renegade", Renegade);
    }

    return role_map.value(role);
}

const General *Player::getAvatarGeneral() const{
    if(general)
        return general;

    QString general_name = property("avatar").toString();
    if(general_name.isEmpty())
        return NULL;
    return Sanguosha->getGeneral(general_name);
}

const General *Player::getGeneral() const{
    return general;
}

bool Player::hasSkill(const QString &skill_name) const{
    if(general)
        return general->hasSkill(skill_name) || acquired_skills.contains(skill_name);
    else
        return false;
}

void Player::acquireSkill(const QString &skill_name){
    acquired_skills.insert(skill_name);
}

void Player::loseSkill(const QString &skill_name){
    acquired_skills.remove(skill_name);
}

QString Player::getPhaseString() const{
    switch(phase){
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

void Player::setPhaseString(const QString &phase_str){
    static QMap<QString, Phase> phase_map;
    if(phase_map.isEmpty()){
        phase_map.insert("start",Start);
        phase_map.insert("judge", Judge);
        phase_map.insert("draw", Draw);
        phase_map.insert("play", Play);
        phase_map.insert("discard", Discard);
        phase_map.insert("finish", Finish);
        phase_map.insert("not_active", NotActive);
    }

    setPhase(phase_map.value(phase_str, NotActive));
}

void Player::setEquip(const EquipCard *card){
    switch(card->location()){
    case EquipCard::WeaponLocation: weapon = qobject_cast<const Weapon*>(card); break;
    case EquipCard::ArmorLocation: armor = qobject_cast<const Armor*>(card); break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = qobject_cast<const Horse*>(card); break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = qobject_cast<const Horse*>(card); break;
    }
}

void Player::removeEquip(const EquipCard *equip){
    switch(equip->location()){
    case EquipCard::WeaponLocation: weapon = NULL; break;
    case EquipCard::ArmorLocation: armor = NULL; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = NULL; break;
    case EquipCard::OffensiveHorseLocation:offensive_horse = NULL; break;
    }
}

bool Player::hasEquip(const Card *card) const{
    return weapon == card || armor == card || defensive_horse == card || offensive_horse == card;
}

const Weapon *Player::getWeapon() const{
    return weapon;
}

const Armor *Player::getArmor() const{
    return armor;
}

const Horse *Player::getDefensiveHorse() const{
    return defensive_horse;
}

const Horse *Player::getOffensiveHorse() const{
    return offensive_horse;
}

QList<const Card *> Player::getEquips() const{
    QList<const Card *> equips;
    if(weapon)
        equips << weapon;
    if(armor)
        equips << armor;
    if(offensive_horse)
        equips << offensive_horse;
    if(defensive_horse)
        equips << defensive_horse;

    return equips;
}

bool Player::hasWeapon(const QString &weapon_name) const{
    return weapon && weapon->objectName() == weapon_name;
}

bool Player::hasArmorEffect(const QString &armor_name) const{
    return armor && !hasFlag("armor_nullified") && armor->objectName() == armor_name;
}

QStack<const Card *> Player::getJudgingArea() const{
    return judging_area;
}

Player::Phase Player::getPhase() const{
    return phase;
}

void Player::setPhase(Phase phase){
    if(this->phase != phase){
        this->phase = phase;

        if(phase == Player::Start){
            emit turn_started();
        }

        emit phase_changed();
    }
}

bool Player::faceUp() const{
    return face_up;
}

void Player::setFaceUp(bool face_up){
    if(this->face_up != face_up){
        this->face_up = face_up;

        emit state_changed();
    }
}

int Player::getMaxCards() const{
    int extra = 0;
    if(Config.MaxHpScheme == 2 && general2){
        int total = general->getMaxHp() + general2->getMaxHp();
        if(total % 2 != 0)
            extra = 1;
    }

    return hp + xueyi + extra;
}

int Player::getXueyi() const{
    return xueyi;
}

QString Player::getKingdom() const{
    return kingdom;
}

void Player::setKingdom(const QString &kingdom){
    if(this->kingdom != kingdom){
        this->kingdom = kingdom;
        emit kingdom_changed();
    }
}

QString Player::getKingdomIcon() const{
    return QString("image/kingdom/icon/%1.png").arg(kingdom);
}

QString Player::getKingdomFrame() const{
    return QString("image/kingdom/frame/%1.png").arg(kingdom);
}

void Player::setXueyi(int xueyi){
    this->xueyi = xueyi;
}

bool Player::isKongcheng() const{
    return getHandcardNum() == 0;
}

bool Player::isNude() const{
    return getHandcardNum() == 0 && getWeapon() == NULL && getArmor() == NULL
            && getDefensiveHorse() == NULL && getOffensiveHorse() == NULL;
}

bool Player::isAllNude() const{
    return isNude() && judging_area.isEmpty();
}

void Player::addDelayedTrick(const Card *trick){
    judging_area.push(trick);
    delayed_tricks.push(DelayedTrick::CastFrom(trick));
}

void Player::removeDelayedTrick(const Card *trick){
    int index = judging_area.indexOf(trick);
    if(index >= 0){
        judging_area.remove(index);
        delayed_tricks.remove(index);
    }
}

const DelayedTrick *Player::topDelayedTrick() const{
    if(delayed_tricks.isEmpty())
        return NULL;
    else
        return delayed_tricks.top();
}

QStack<const DelayedTrick *> Player::delayedTricks() const{
    return delayed_tricks;
}

bool Player::containsTrick(const QString &trick_name) const{
    QVectorIterator<const DelayedTrick *> itor(delayed_tricks);
    while(itor.hasNext()){      
        const DelayedTrick *trick = itor.next();
        if(trick->objectName() == trick_name)
            return true;
    }

    return false;
}

bool Player::isChained() const{
    return chained;
}

void Player::setChained(bool chained){
    if(this->chained != chained){
        this->chained = chained;
        emit state_changed();
    }
}

void Player::addMark(const QString &mark){
    int value = marks.value(mark, 0);
    value++;
    setMark(mark, value);
}

void Player::removeMark(const QString &mark){
    int value = marks.value(mark, 0);
    value--;
    value = qMin(0, value);
    setMark(mark, value);
}

void Player::setMark(const QString &mark, int value){
    if(marks[mark] != value){
        marks[mark] = value;
    }
}

int Player::getMark(const QString &mark) const{
    return marks.value(mark, 0);
}

bool Player::canSlash(const Player *other, bool distance_limit) const{
    if(other->hasSkill("kongcheng") && other->isKongcheng())
        return false;

    if(other == this)
        return false;

    if(distance_limit)
        return distanceTo(other) <= getAttackRange();
    else
        return true;
}

int Player::getCardCount(bool include_equip) const{
    int count = getHandcardNum();

    if(include_equip){
        if(weapon)
            count ++;

        if(armor)
            count ++;

        if(defensive_horse)
            count ++;

        if(offensive_horse)
            count ++;
    }

    return count;
}

QList<int> &Player::getPile(const QString &pile_name){
    return piles[pile_name];
}

QList<Player::Phase> &Player::getPhases(){
    return phases;
}

void Player::skip(Player::Phase phase){
    phases.removeOne(phase);
}
