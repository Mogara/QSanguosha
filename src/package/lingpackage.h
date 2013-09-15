#ifndef _LING_H
#define _LING_H

#include "package.h"
#include "card.h"
#include "yjcm-package.h"

class LingPackage: public Package {
    Q_OBJECT

public:
    LingPackage();
};

class Ling2013Package: public Package {
    Q_OBJECT

public:
    Ling2013Package();
};

class LuoyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LuoyiCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NeoFanjianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NeoFanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Neo2013XinzhanCard: public XinzhanCard{
    Q_OBJECT

public:
    Q_INVOKABLE Neo2013XinzhanCard();
};

class Neo2013FanjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE Neo2013FanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Neo2013PujiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE Neo2013PujiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif

