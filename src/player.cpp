#include "player.h"

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

void Player::setGeneral(General *general){
    this->general = general;
    hp = general->getMaxHp();
}

General *Player::getGeneral() const{
    return general;
}
