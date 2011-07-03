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

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TaichenFightCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TaichenFightCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhiyuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiyuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // FANCHENGSCENARIO_H
