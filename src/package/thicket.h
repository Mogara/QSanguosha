#ifndef _THICKET_H
#define _THICKET_H

#include "package.h"
#include "card.h"

class ThicketPackage: public Package {
    Q_OBJECT

public:
    ThicketPackage();
};

class HaoshiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HaoshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DimengCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DimengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LuanwuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif

