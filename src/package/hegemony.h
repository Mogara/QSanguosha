#ifndef HEGEMONY_H
#define HEGEMONY_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class AllyFarAttackNear:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE AllyFarAttackNear(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class EaseVSFatigue: public GlobalEffect{
    Q_OBJECT

public:
    Q_INVOKABLE EaseVSFatigue(Card::Suit suit, int number);

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KnowThyself: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE KnowThyself(Card::Suit suit, int number);

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WuLiuJian:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE WuLiuJian(Card::Suit suit, int number);
};

class TriDouble:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE TriDouble(Card::Suit suit, int number);
};

class HegemonyCardPackage: public Package{
    Q_OBJECT

public:
    HegemonyCardPackage();
};

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
