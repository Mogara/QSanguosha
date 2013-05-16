#ifndef _YJCM2012_H
#define _YJCM2012_H

#include "package.h"
#include "card.h"
#include "wind.h"

#include <QMutex>
#include <QGroupBox>
#include <QAbstractButton>

class YJCM2012Package: public Package {
    Q_OBJECT

public:
    YJCM2012Package();
};

class QiceCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QiceCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
};

class GongqiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE GongqiCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JiefanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JiefanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AnxuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE AnxuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChunlaoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChunlaoWineCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoWineCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif

