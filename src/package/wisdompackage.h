#ifndef WISDOM_H
#define WISDOM_H

#include "package.h"
#include "card.h"
#include "standard.h"

class JuaoCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JuaoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BawangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BawangCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WeidaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeidaiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HouyuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HouyuanCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShouyeCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouyeCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WisdomPackage: public Package{
    Q_OBJECT

public:
    WisdomPackage();
};

#endif // WISDOM_H
