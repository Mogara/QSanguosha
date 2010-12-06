#ifndef NOSTALGIA_H
#define NOSTALGIA_H

#include "package.h"
#include "card.h"

class NostalgiaPackage: public Package{
    Q_OBJECT

public:
    NostalgiaPackage();
};

class TianxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianxiangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // NOSTALGIA_H
