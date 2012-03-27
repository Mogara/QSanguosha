#ifndef SPECIAL3V3_H
#define SPECIAL3V3_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class HuanshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanshiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Special3v3Package : public Package{
    Q_OBJECT

public:
    Special3v3Package();
};

#endif // SPECIAL3V3_H
