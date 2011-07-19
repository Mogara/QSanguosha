#ifndef MOUNTAINPACKAGE_H
#define MOUNTAINPACKAGE_H

#include "package.h"
#include "card.h"

class QiaobianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiaobianCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TiaoxinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TiaoxinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhijianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhijianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MountainPackage : public Package
{
    Q_OBJECT

public:
    MountainPackage();
};

#endif // MOUNTAINPACKAGE_H
