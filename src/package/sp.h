#ifndef SPPACKAGE_H
#define SPPACKAGE_H

#include "package.h"
#include "card.h"

class SPPackage: public Package{
    Q_OBJECT

public:
    SPPackage();
};

class WeidiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeidiCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class YuanhuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE YuanhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuejiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XuejiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BifaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BifaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SongciCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE SongciCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MizhaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MizhaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DuwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MouzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MouzhuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
};

class SPCardPackage: public Package{
    Q_OBJECT

public:
    SPCardPackage();
};

#endif // SPPACKAGE_H
