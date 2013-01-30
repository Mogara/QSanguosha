#ifndef HEGEMONY_H
#define HEGEMONY_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class HegemonyPackage: public Package{
    Q_OBJECT

public:
    HegemonyPackage();
};

class ShushenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShushenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DuoshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuoshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LirangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LirangCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class MingshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MingshiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FenxunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiongyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiongyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

typedef Skill SkillClass;
class QingchengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingchengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShuangrenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShuangrenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // HEGEMONY_H
