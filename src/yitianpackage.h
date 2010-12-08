#ifndef YITIANPACKAGE_H
#define YITIANPACKAGE_H

#include "package.h"
#include "standard.h"

class YitianPackage: public Package{
    Q_OBJECT

public:
    YitianPackage();
};


class ChengxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChengxiangCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JuejiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JuejiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // YITIANPACKAGE_H
