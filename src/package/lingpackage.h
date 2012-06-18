#ifndef LINGPACKAGE_H
#define LINGPACKAGE_H

#include "package.h"
#include "card.h"

class LingPackage : public Package{
    Q_OBJECT

public:
    LingPackage();
};

class LuoyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuoyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class NeoFanjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NeoFanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // LINGPACKAGE_H
