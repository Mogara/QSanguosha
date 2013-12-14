#ifndef _H_MOMENTUM_H
#define _H_MOMENTUM_H

#include "package.h"
#include "card.h"
#include "skill.h"

class GuixiuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE GuixiuCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class CunsiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE CunsiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HMomentumPackage: public Package {
    Q_OBJECT

public:
    HMomentumPackage();
};

#endif
