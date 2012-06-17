#ifndef MANEUVERING_H
#define MANEUVERING_H

#include "standard.h"

class NatureSlash: public Slash{
    Q_OBJECT

public:
    NatureSlash(Suit suit, int number, DamageStruct::Nature nature);
    virtual bool match(const QString &pattern) const;
};

class ThunderSlash: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE ThunderSlash(Card::Suit suit, int number);
};

class FireSlash: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE FireSlash(Card::Suit suit, int number);
};

class Analeptic: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Analeptic(Card::Suit suit, int number);
    virtual QString getSubtype() const;

    static bool IsAvailable(const Player *player);

    virtual bool isAvailable(const Player *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Fan: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Fan(Card::Suit suit, int number);
};

class GudingBlade: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE GudingBlade(Card::Suit suit, int number);
};

class Vine: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Vine(Card::Suit suit, int number);
};

class SilverLion: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE SilverLion(Card::Suit suit, int number);

    virtual void onUninstall(ServerPlayer *player) const;
};

class IronChain: public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE IronChain(Card::Suit suit, int number);

    virtual QString getSubtype() const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FireAttack: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE FireAttack(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SupplyShortage: public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE SupplyShortage(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class ManeuveringPackage: public Package{
    Q_OBJECT

public:
    ManeuveringPackage();
};

#endif // MANEUVERING_H
