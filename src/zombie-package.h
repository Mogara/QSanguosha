#ifndef ZOMBIEPACKAGE_H
#define ZOMBIEPACKAGE_H

#include "standard-skillcards.h"
#include "maneuvering.h"

class GanranEquip: public IronChain{
    Q_OBJECT

public:
    Q_INVOKABLE GanranEquip(Card::Suit suit, int number);
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class PeachingCard: public QingnangCard{
    Q_OBJECT

public:
    Q_INVOKABLE PeachingCard();
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class ZombiePackage : public Package{
    Q_OBJECT

public:
    ZombiePackage();
};
#endif // ZOMBIEPACKAGE_H
