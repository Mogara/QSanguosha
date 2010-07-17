#include "player.h"
#include "engine.h"

Player::Player(QObject *parent)
    :QObject(parent), general(NULL),
    hp(-1), max_hp(-1), state("online"), seat(0),
    src_correct(0), dest_correct(0),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL)
{
}

void Player::setHp(int hp){
    if(hp >= 0 && hp <= max_hp)
        this->hp = hp;
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

void Player::setCorrect(int src_correct, int dest_correct){
    this->src_correct = src_correct;
    this->dest_correct = dest_correct;
}

int Player::distanceTo(const Player *other) const{
    int right = qAbs(seat - other->seat);
    int left = parent()->children().count() - right;
    return qMin(left, right) + src_correct + other->dest_correct;
}

int Player::getGeneralMaxHP() const{
    Q_ASSERT(general != NULL);
    return general->getMaxHp();
}

void Player::setGeneral(const QString &general_name){
    const General *new_general = Sanguosha->getGeneral(general_name);

    if(this->general != new_general){
        this->general = new_general;
        if(new_general)
            setHp(getMaxHP());        

        emit general_changed();
    }
}

QString Player::getGeneral() const{
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
        emit state_changed(state);
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

QString Player::getPhase() const{
    return phase;
}

void Player::setPhase(const QString &phase){
    this->phase = phase;
}

const Card *Player::replaceEquip(const Card *equip){
    const Card *uninstall = NULL;
    QString subtype = equip->getSubtype();
    if(subtype == "weapon"){
        uninstall = weapon;
        weapon = equip;
    }else if(subtype == "armor"){
        uninstall = armor;
        armor = equip;
    }else if(subtype == "defensive_horse"){
        uninstall = defensive_horse;
        defensive_horse = equip;
    }else if(subtype == "offensive_horse"){
        uninstall = offensive_horse;
        offensive_horse = equip;
    }

    return uninstall;
}

void Player::removeEquip(const Card *equip){
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
