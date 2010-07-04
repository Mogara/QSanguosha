#include "player.h"
#include "engine.h"

Player::Player(QObject *parent)
    :QObject(parent), general(NULL), hp(-1)
{
}

void Player::setHp(int hp){
    if(hp > 0 && hp <= general->getMaxHp())
        this->hp = hp;
}

int Player::getHp() const{
    return hp;
}

bool Player::isWounded() const{
    return hp < general->getMaxHp();
}

void Player::setGeneral(const General *general){
    if(this->general != general){
        this->general = general;
        emit general_changed(general);
    }
}

const General *Player::getGeneral() const{
    return general;
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
