#ifndef SANDZHIMENGPACKAGE_H
#define SANDZHIMENGPACKAGE_H

#include "package.h"
#include "card.h"

class SanDZhimengPackage: public Package{
    Q_OBJECT

public:
    SanDZhimengPackage();
};

class DujiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DujiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class PengriCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PengriCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class XunguiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunguiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DaojuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DaojuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class Zha0xinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE Zha0xinCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChanxianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChanxianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QuanjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuanjianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SijieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SijieCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // SanDZHIMENGPACKAGE_H
