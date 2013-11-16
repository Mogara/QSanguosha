#ifndef _YJCM_H
#define _YJCM_H

#include "package.h"
#include "card.h"
#include "skill.h"

class YJCMPackage: public Package {
    Q_OBJECT

public:
    YJCMPackage();
};

class Shangshi: public TriggerSkill {
    Q_OBJECT

public:
    Shangshi();
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhangchunhua, QVariant &data) const;

protected:
    virtual int getMaxLostHp(ServerPlayer *zhangchunhua) const;
};

class MingceCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE MingceCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GanluCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE GanluCard();
    void swapEquip(ServerPlayer *first, ServerPlayer *second) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XianzhenCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XianzhenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuanfengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XuanfengCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JujianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JujianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XinzhanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XinzhanCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PaiyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE PaiyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif

