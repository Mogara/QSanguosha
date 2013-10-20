#ifndef _MANEUVERING_H
#define _MANEUVERING_H

#include "standard.h"

class GudingBlade: public Weapon {
    Q_OBJECT

public:
    Q_INVOKABLE GudingBlade(Card::Suit suit, int number);
};

class Vine: public Armor {
    Q_OBJECT

public:
    Q_INVOKABLE Vine(Card::Suit suit, int number);
};

class SilverLion: public Armor {
    Q_OBJECT

public:
    Q_INVOKABLE SilverLion(Card::Suit suit, int number);

    virtual void onUninstall(ServerPlayer *player) const;
};

class ManeuveringPackage: public Package {
    Q_OBJECT

public:
    ManeuveringPackage();
};

#endif

