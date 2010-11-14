#ifndef FANCHENGSCENARIO_H
#define FANCHENGSCENARIO_H

#include "scenario.h"
#include "card.h"

class FanchengScenario: public Scenario{
    Q_OBJECT

public:
    FanchengScenario();

    virtual void onTagSet(Room *room, const QString &key) const;
};

class DujiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DujiangCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FloodCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FloodCard();

    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TaichenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TaichenCard();

    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // FANCHENGSCENARIO_H
