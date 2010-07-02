#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"

#include <QObject>

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(bool wounded READ isWounded STORED false)
    Q_PROPERTY(General *general READ getGeneral);

public:
    explicit Player(General *general, QObject *parent = 0);

    // property getters
    int getHp() const;
    void setHp(int hp);
    bool isWounded() const;
    General *getGeneral() const;

private:
    General *general;
    int hp;
};

#endif // PLAYER_H
