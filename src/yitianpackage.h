#ifndef YITIANPACKAGE_H
#define YITIANPACKAGE_H

#include "package.h"
#include "standard.h"

class YitianPackage: public Package{
    Q_OBJECT

public:
    YitianPackage();
};

class Shit:public BasicCard{
    Q_OBJECT
public:
    Q_INVOKABLE Shit(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onMove(const CardMoveStruct &move) const;
};

#endif // YITIANPACKAGE_H
