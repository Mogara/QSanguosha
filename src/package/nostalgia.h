#ifndef _NOSTALGIA_H
#define _NOSTALGIA_H

#include "package.h"
#include "card.h"
#include "standard.h"
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

class NosGuhuoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NosGuhuoCard();
    bool nosguhuo(ServerPlayer *yuji) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

#endif

