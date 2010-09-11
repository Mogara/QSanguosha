#ifndef THICKET_H
#define THICKET_H

#include "package.h"
#include "card.h"

class ThicketPackage: public Package{
    Q_OBJECT

public:
    ThicketPackage();
};

class YinghunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YinghunCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DimengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DimengCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
};

class LuanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FangzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangzhuCard();
};

class ShenfenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenfenCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // THICKET_H
