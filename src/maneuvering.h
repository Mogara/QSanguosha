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
    ThunderSlash(Suit suit, int number);
};

class FireSlash: public NatureSlash{
    Q_OBJECT

public:
    FireSlash(Suit suit, int number);
};

class Analeptic: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Analeptic(Card::Suit suit, int number);
    virtual QString getSubtype() const;

    virtual bool isAvailable() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool match(const QString &pattern) const;
};

class Fan: public Weapon{
    Q_OBJECT

public:
    Fan(Suit suit, int number);
};

class GudingBlade: public Weapon{
    Q_OBJECT

public:
    GudingBlade(Suit suit, int number);
};

class Vine: public Armor{
    Q_OBJECT

public:
    Vine(Suit suit, int number);
};

class SilverLion: public Armor{
    Q_OBJECT

public:
    SilverLion(Suit suit, int number);

    virtual void onUninstall(ServerPlayer *player) const;
};

class IronChain: public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE IronChain(Card::Suit suit, int number);

    virtual QString getSubtype() const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FireAttack: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE FireAttack(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SupplyShortage: public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE SupplyShortage(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select);
    virtual bool judge(const Card *card) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class ManeuveringPackage: public Package{
    Q_OBJECT

public:
    ManeuveringPackage();
};

#endif // MANEUVERING_H
