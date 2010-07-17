#include "clientplayer.h"

ClientPlayer::ClientPlayer(QObject *parent)
    :Player(parent), handcard_num(0)
{
}

void ClientPlayer::drawNCard(int card_num){
    handcard_num += card_num;
}

int ClientPlayer::getHandcardNum() const{
    return handcard_num;
}

void ClientPlayer::addCard(const Card *card, const QString &location){
    if(location == "equip")
        replaceEquip(card);
    else if(location == "hand"){
        known_cards << card;
        handcard_num ++;
    }
}

void ClientPlayer::removeCard(const Card *card, const QString &location){
    if(location == "hand"){
        handcard_num --;
        known_cards.removeOne(card);        
    }else if(location == "equip")
        removeEquip(card);
}
