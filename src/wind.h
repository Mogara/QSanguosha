#ifndef WIND_H
#define WIND_H

#include "package.h"
#include "card.h"

class GuidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuidaoCard();
};

class GongxinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GongxinCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class WindPackage: public Package{
    Q_OBJECT

public:
    WindPackage();
};



#endif // WIND_H
