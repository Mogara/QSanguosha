#ifndef _NOSTALGIA_H
#define _NOSTALGIA_H

#include "package.h"
#include "card.h"
#include "standard.h"

class NostalgiaPackage: public Package {
    Q_OBJECT

public:
    NostalgiaPackage();
};

class MoonSpear: public Weapon {
    Q_OBJECT

public:
    Q_INVOKABLE MoonSpear(Card::Suit suit = Diamond, int number = 12);
};

class NostalStandardPackage: public Package {
    Q_OBJECT

public:
    NostalStandardPackage();
};

class NostalWindPackage: public Package {
    Q_OBJECT

public:
    NostalWindPackage();
};

#endif

