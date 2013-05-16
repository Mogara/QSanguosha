#ifndef _STANDARD_SKILLCARDS_H
#define _STANDARD_SKILLCARDS_H

#include "skill.h"
#include "card.h"

class ZhihengCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class RendeCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JieyinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JieyinCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TuxiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TuxiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FanjianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KurouCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE KurouCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LijianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LijianCard(bool cancelable = true);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

private:
    bool duel_cancelable;
};

class QingnangCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LiuliCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LiuliCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JijiangCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JijiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

class JijiangViewAsSkill: public ZeroCardViewAsSkill {
    Q_OBJECT

public:
    JijiangViewAsSkill();

    virtual bool isEnabledAtPlay(const Player *player) const;
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    virtual const Card *viewAs() const;

private:
    static bool hasShuGenerals(const Player *player);
};

class Yiji: public MasochismSkill {
    Q_OBJECT

public:
    Yiji();
    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const;

protected:
    int n;
};

#endif
