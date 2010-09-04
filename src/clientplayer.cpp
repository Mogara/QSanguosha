#include "clientplayer.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "standard.h"

ClientPlayer::ClientPlayer(Client *client)
    :Player(client), handcard_num(0)
{
}

void ClientPlayer::drawNCard(int card_num){
    handcard_num += card_num;
}

int ClientPlayer::aliveCount() const{
    return ClientInstance->alivePlayerCount();
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
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            setEquip(equip);
            break;
        }
    case DelayedTrick:{
            addDelayedTrick(card);
            break;
        }
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
    case Equip:{
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            removeEquip(equip);
            break;
        }
    case DelayedTrick:{           
            removeDelayedTrick(card);
            break;
        }

    default:
        // FIXME
        ;
    }
}

QList<const Card *> ClientPlayer::getCards() const{
    return known_cards;
}

QList<int> ClientPlayer::nullifications() const{
    QList<int> card_ids;

    foreach(const Card *card, known_cards){
        if(card->objectName() == "nullification")
            card_ids << card->getId();
    }

    return card_ids;
}

void ClientPlayer::MoveCard(const CardMoveStructForClient &move){
    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from)
        move.from->removeCard(card, move.from_place);
    else
        ClientInstance->discarded_list.removeOne(card);

    if(move.to)
        move.to->addCard(card, move.to_place);
    else
        ClientInstance->discarded_list.prepend(card);
}
