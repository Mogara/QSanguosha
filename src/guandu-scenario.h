#ifndef GUANDUSCENARIO_H
#define GUANDUSCENARIO_H

#include "scenario.h"
#include "card.h"
#include "standard.h"

class ZhanShuangxiongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhanShuangxiongCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
};

class SmallTuxiCard: public TuxiCard{
    Q_OBJECT

public:
    Q_INVOKABLE SmallTuxiCard();
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuanduScenario : public Scenario{
    Q_OBJECT

public:
    GuanduScenario();

    virtual void onTagSet(Room *room, const QString &key) const;
};

#endif // GUANDUSCENARIO_H
