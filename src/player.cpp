#include "player.h"
#include "engine.h"
#include "room.h"
#include "client.h"
#include "standard.h"

Player::Player(QObject *parent)
    :QObject(parent), general(NULL),
    hp(-1), max_hp(-1), max_cards(-1), state("online"), seat(0), alive(true),
    attack_range(1), phase(NotActive),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    face_up(true)
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

    return qMin(distance, 1);
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

    phase = phase_map.value(phase_str, NotActive);
}

const EquipCard *Player::getEquip(const QString &subtype) const{
    if(subtype == "weapon")
        return weapon;
    else if(subtype == "armor")
        return armor;
    else if(subtype == "defensive_horse")
        return defensive_horse;
    else if(subtype == "offensive_horse")
        return offensive_horse;
    else
        return NULL;
}

void Player::setEquip(const EquipCard *card){
    QString subtype = card->getSubtype();
    if(subtype == "weapon")
        weapon = qobject_cast<const Weapon*>(card);
    else if(subtype == "armor")
        armor = qobject_cast<const Armor*>(card);
    else if(subtype == "defensive_horse")
        defensive_horse = qobject_cast<const Horse*>(card);
    else if(subtype == "offensive_horse")
        offensive_horse = qobject_cast<const Horse*>(card);
}

void Player::removeEquip(const EquipCard *equip){
    QString subtype = equip->getSubtype();
    if(subtype == "weapon")
        weapon = NULL;
    else if(subtype == "armor")
        armor = NULL;
    else if(subtype == "defensive_horse")
        defensive_horse = NULL;
    else if(subtype == "offensive_horse")
        offensive_horse = NULL;
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


QStack<const Card *> Player::getJudgingArea() const{
    return judging_area;
}

Player::Phase Player::getPhase() const{
    return phase;
}

void Player::setPhase(Phase phase){
    this->phase = phase;
}

Player::Phase Player::getNextPhase() const{
    if(phase == NotActive)
        return NotActive;
    else{
        int phase_num = static_cast<int>(phase);
        return static_cast<Phase>(phase_num + 1);
    }
}

bool Player::faceUp() const{
    return face_up;
}

void Player::turnOver(){
    face_up = !face_up;
}

int Player::getMaxCards() const{
    return max_cards;
}

void Player::setMaxCards(int max_cards){
    this->max_cards = max_cards;
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
}

void Player::removeDelayedTrick(const Card *trick){
    int index = judging_area.indexOf(trick);
    if(index >= 0)
        judging_area.remove(index);
}
