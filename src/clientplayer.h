#ifndef CLIENTPLAYER_H
#define CLIENTPLAYER_H

#include "player.h"

class ClientPlayer : public Player
{
    Q_OBJECT

public:
    explicit ClientPlayer(QObject *parent);
    void drawNCard(int card_num);
    QList<const Card *> getCards() const;

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);

    QList<int> nullifications() const;

private:
    int handcard_num;
    QList<const Card *> known_cards;
};

#endif // CLIENTPLAYER_H
