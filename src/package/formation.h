#ifndef _H_FORMATION_H
#define _H_FORMATION_H

#include "package.h"
#include "card.h"
#include "skill.h"

class HuyuanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuyuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HeyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HeyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
}; 

class FormationPackage: public Package {
    Q_OBJECT

public:
    FormationPackage();
};

#endif