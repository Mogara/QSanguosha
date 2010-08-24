#ifndef CLIENTPLAYER_H
#define CLIENTPLAYER_H

#include "player.h"

class ClientPlayer;
class Client;

struct CardMoveStructForClient{
    int card_id;
    ClientPlayer *from, *to;
    Player::Place from_place, to_place;

    void parse(const QString &str, bool *ok);
};

class ClientPlayer : public Player
{
    Q_OBJECT

public:
    explicit ClientPlayer(Client *client);
    void drawNCard(int card_num);
    QList<const Card *> getCards() const;

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);

    QList<int> nullifications() const;

    static void MoveCard(const CardMoveStructForClient &move);

private:
    int handcard_num;
    QList<const Card *> known_cards;
};

#endif // CLIENTPLAYER_H
