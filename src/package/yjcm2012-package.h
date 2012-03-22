#ifndef YJCM2012PACKAGE_H
#define YJCM2012PACKAGE_H

#include "package.h"
#include "card.h"

class YJCM2012Package: public Package{
    Q_OBJECT

public:
    YJCM2012Package();
};

class ZhenlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhenlieCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // YJCM2012PACKAGE_H
