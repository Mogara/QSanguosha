#ifndef _NOSTALGIA_H
#define _NOSTALGIA_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "standard-skillcards.h"

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

class NostalYJCMPackage: public Package {
    Q_OBJECT

public:
    NostalYJCMPackage();
};

class NostalYJCM2012Package: public Package {
    Q_OBJECT

public:
    NostalYJCM2012Package();
};

class NosJujianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NosJujianCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosXuanhuoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NosXuanhuoCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosJiefanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NosJiefanCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};


class NosRendeCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NosRendeCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosLijianCard: public LijianCard {
    Q_OBJECT

public:
    Q_INVOKABLE NosLijianCard();
};

#endif

