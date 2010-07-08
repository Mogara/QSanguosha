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
