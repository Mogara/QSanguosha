#ifndef MANEUVERING_H
#define MANEUVERING_H

#include "standard.h"

class Analeptic: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Analeptic(Card::Suit suit, int number);
    virtual QString getSubtype() const;

    virtual bool isAvailable() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ManeuveringPackage: public Package{
    Q_OBJECT

public:
    ManeuveringPackage();
};

#endif // MANEUVERING_H
