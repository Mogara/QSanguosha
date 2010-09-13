#include "player.h"
#include "engine.h"
#include "room.h"
#include "client.h"
#include "standard.h"

Player::Player(QObject *parent)
    :QObject(parent), general(NULL),
    hp(-1), max_hp(-1), xueyi(0), state("online"), seat(0), alive(true),
    attack_range(1), phase(NotActive),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    face_up(true), chained(false)
{
    correct.equip_dest = 0;
    correct.equip_src = 0;
    correct.skill_src = 0;
    correct.skill_dest = 0;
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
    this->max_hp = max_hp;
    if(hp > max_hp)
        hp = max_hp;
}

int Player::getLostHp() const{
    return max_hp - hp;
}

bool Player::isWounded() const{
    if(hp < 0)
        return true;
    else
        return hp < general->getMaxHp();
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
    }else
        flags.insert(flag);
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

QString Player::getCorrect() const{
    return QString("%1:%2:%3:%4")
            .arg(correct.equip_src)
            .arg(correct.equip_dest)
            .arg(correct.skill_src)
            .arg(correct.skill_dest);
}

void Player::setCorrect(const QString &correct_str){
    QRegExp pattern("(\\w+):(-?\\d+)");

    pattern.indexIn(correct_str);
    QStringList texts = pattern.capturedTexts();
    QString field = texts.at(1);
    int value = texts.at(2).toInt();

    if(field == "equip_src")
        correct.equip_src = value;
    else if(field == "equip_dest")
        correct.equip_dest = value;
    else if(field == "skill_src")
        correct.skill_src = value;
    else if(field == "skill_dest")
        correct.skill_dest = value;
}

bool Player::inMyAttackRange(const Player *other) const{
    return distanceTo(other) <= attack_range;
}

int Player::distanceTo(const Player *other) const{
    if(this == other)
        return 0;

    int right = qAbs(seat - other->seat);
    int left = aliveCount() - right;
    int distance = qMin(left, right);

    distance += correct.equip_src;
    distance += correct.skill_src;

    distance += other->correct.equip_dest;
    distance += other->correct.skill_dest;

    if(distance < 1)
        distance = 1;
    return distance;
}

int Player::getGeneralMaxHP() const{
    Q_ASSERT(general != NULL);
    return general->getMaxHp();
}

void Player::setGeneralName(const QString &general_name){
    const General *new_general = Sanguosha->getGeneral(general_name);

    if(this->general != new_general){
        this->general = new_general;
        if(new_general)
            setHp(getMaxHP());

        emit general_changed();
    }
}

QString Player::getGeneralName() const{
    if(general)
        return general->objectName();
    else
        return "";
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
        return general->hasSkill(skill_name);
    else
        return false;
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

bool Player::hasWeapon(const QString &weapon_name) const{
    return weapon && weapon->objectName() == weapon_name;
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
    }
}

bool Player::faceUp() const{
    return face_up;
}

void Player::setFaceUp(bool face_up){
    this->face_up = face_up;
}

void Player::turnOver(){
    face_up = !face_up;
}

int Player::getMaxCards() const{
    return hp + xueyi;
}

int Player::getXueyi() const{
    return xueyi;
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

        emit state_changed();
    }
}

int Player::getMark(const QString &mark) const{
    return marks.value(mark, 0);
}

bool Player::canSlash(const Player *other) const{
    if(other->hasSkill("kongcheng") && other->isKongcheng())
        return false;

    return distanceTo(other) <= getAttackRange();
}
