#ifndef GOD_H
#define GOD_H

#include "package.h"
#include "card.h"

class GodPackage : public Package{
    Q_OBJECT

public:
    GodPackage();
};

class GongxinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GongxinCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YeyanCard: public SkillCard{
    Q_OBJECT

public:
    virtual void use(const QList<const ClientPlayer *> &targets) const;

    void damage(ServerPlayer *shenzhouyu, ServerPlayer *target, int point) const;
};

class GreatYeyanCard: public YeyanCard{
    Q_OBJECT

public:
    Q_INVOKABLE GreatYeyanCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MediumYeyanCard: public YeyanCard{
    Q_OBJECT

public:
    Q_INVOKABLE MediumYeyanCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SmallYeyanCard: public YeyanCard{
    Q_OBJECT

public:
    Q_INVOKABLE SmallYeyanCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShenfenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenfenCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // GOD_H
