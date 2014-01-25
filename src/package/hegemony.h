#ifndef _HEGEMONY_H
#define _HEGEMONY_H

#include "standard.h"

class HegemonyPackage: public Package {
    Q_OBJECT

public:
    HegemonyPackage();
};

class FenxunCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif

