#ifndef _H_FORMATION_H
#define _H_FORMATION_H

#include "package.h"
#include "card.h"
#include "skill.h"

class JixiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JixiCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class JixiSnatchCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JixiSnatchCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class HuyuanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuyuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TiaoxinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TiaoxinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FormationPackage: public Package {
    Q_OBJECT

public:
    FormationPackage();
};

#endif