#include "clientplayer.h"
#include "skill.h"
#include "client.h"

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

void ClientPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            known_cards << card;
            handcard_num++;
            break;
        }
    case Equip: replaceEquip(card); break;
    default:
        // FIXME
        ;
    }
}

void ClientPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            handcard_num--;
            known_cards.removeOne(card);
            break;
        }
    case Equip: removeEquip(card); break;
    default:
        // FIXME
        ;
    }
}

QList<const Card *> ClientPlayer::getCards() const{
    return known_cards;
}
