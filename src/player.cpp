#include "player.h"

Player::Player(General *general, QObject *parent)
    :QObject(parent), general(general)
{
    hp = general->getMaxHp();
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

General *Player::getGeneral() const{
    return general;
}
