#include "player.h"
#include "engine.h"

Player::Player(QObject *parent)
    :QObject(parent), general(NULL), hp(-1), max_hp(-1), state("online")
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

