#ifndef WIND_H
#define WIND_H

#include "package.h"
#include "card.h"

class GuidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuidaoCard();
};

class LeijiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LeijiCard();
};

class HuangtianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuangtianCard();

    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GongxinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GongxinCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class ShensuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShensuCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WindPackage: public Package{
    Q_OBJECT

public:
    WindPackage();
};



#endif // WIND_H
