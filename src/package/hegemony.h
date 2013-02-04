#ifndef _HEGEMONY_H
#define _HEGEMONY_H

#include "standard.h"

class HegemonyPackage: public Package {
    Q_OBJECT

public:
    HegemonyPackage();
};

class DuoshiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DuoshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShushenCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShushenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FenxunCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShuangrenCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShuangrenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiongyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XiongyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SijianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE SijianCard();

    void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QingchengCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingchengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif
