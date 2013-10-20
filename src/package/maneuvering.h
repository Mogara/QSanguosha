#ifndef _MANEUVERING_H
#define _MANEUVERING_H

#include "standard.h"

class GudingBlade: public Weapon {
    Q_OBJECT

public:
    Q_INVOKABLE GudingBlade(Card::Suit suit, int number);
};

class ManeuveringPackage: public Package {
    Q_OBJECT

public:
    ManeuveringPackage();
};

#endif

