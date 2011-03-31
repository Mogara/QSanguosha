#ifndef YJCMPACKAGE_H
#define YJCMPACKAGE_H

#include "package.h"
#include "card.h"

class YJCMPackage: public Package{
    Q_OBJECT

public:
    YJCMPackage();
};

class JujianCard: public SkillCard{
    Q_OBJECT

public:
    JujianCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // YJCMPACKAGE_H
