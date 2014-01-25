#ifndef _MOUNTAIN_H
#define _MOUNTAIN_H

#include "package.h"
#include "card.h"
#include "generaloverview.h"

class TiaoxinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TiaoxinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};



class MountainPackage: public Package {
    Q_OBJECT

public:
    MountainPackage();
};

#endif

