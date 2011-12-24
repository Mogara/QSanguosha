#ifndef SPPACKAGE_H
#define SPPACKAGE_H

#include "package.h"
#include "card.h"

class SPPackage: public Package{
    Q_OBJECT

public:
    SPPackage();
};

class WeidiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeidiCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class SPCardPackage: public Package{
    Q_OBJECT

public:
    SPCardPackage();
};

#endif // SPPACKAGE_H
