#ifndef _STANDARD_QUN_H
#define _STANDARD_QUN_H

#include "standard-package.h"
#include "card.h"

class QingnangCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LijianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LijianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LuanwuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiongyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XiongyiCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuoshuiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuoshuiCard();
};

class QingchengCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingchengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif

