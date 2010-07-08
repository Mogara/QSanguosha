#ifndef CLIENTPLAYER_H
#define CLIENTPLAYER_H

#include "player.h"

class ClientPlayer : public Player
{
    Q_OBJECT

public:
    explicit ClientPlayer(QObject *parent);
    void drawNCard(int card_num);
    virtual int getHandcardNum() const;

private:
    int handcard_num;
};

#endif // CLIENTPLAYER_H
