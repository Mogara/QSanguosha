#ifndef WIND_H
#define WIND_H

#include "package.h"
#include "card.h"

class GuidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuidaoCard();
};

class WindPackage: public Package{
    Q_OBJECT

public:
    WindPackage();
};



#endif // WIND_H
