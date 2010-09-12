#ifndef STANDARD_H
#define STANDARD_H

#include "package.h"
#include "card.h"
#include "roomthread.h"

class StandardPackage:public Package{
    Q_OBJECT

public:
    StandardPackage();
    void addCards();
    void addGenerals();
};

class BasicCard:public Card{
    Q_OBJECT

public:
    BasicCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class TrickCard:public Card{
    Q_OBJECT

public:
    TrickCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class EquipCard:public Card{
    Q_OBJECT

    Q_ENUMS(Location);

public:
    enum Location {
        WeaponLocation,
        ArmorLocation,
        DefensiveHorseLocation,
        OffensiveHorseLocation,
    };

    EquipCard(Suit suit, int number):Card(suit, number, true), skill(NULL), set_flag(false){}
    virtual QString getType() const;
    virtual int getTypeId() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;

    // should be pure virtual
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual Location location() const = 0;

protected:
    TriggerSkill *skill;
    bool set_flag;
};

class GlobalEffect:public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE GlobalEffect(Card::Suit suit, int number):TrickCard(suit, number){ target_fixed = true;}
    virtual QString getSubtype() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GodSalvation:public GlobalEffect{
    Q_OBJECT

public:
    Q_INVOKABLE GodSalvation(Card::Suit suit = Heart, int number = 1);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AOE:public TrickCard{
    Q_OBJECT

public:
    AOE(Suit suit, int number):TrickCard(suit, number){ target_fixed = true;}
    virtual QString getSubtype() const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SavageAssault:public AOE{
public:
    Q_INVOKABLE SavageAssault(Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ArcheryAttack:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE ArcheryAttack(Card::Suit suit = Heart, int number = 1);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SingleTargetTrick: public TrickCard{
    Q_OBJECT

public:
    SingleTargetTrick(Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Duel:public SingleTargetTrick{
public:
    Duel(Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DelayedTrick:public TrickCard{
    Q_OBJECT

public:
    DelayedTrick(Suit suit, int number, bool movable = false);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual QString getSubtype() const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    virtual bool judge(const Card *card) const = 0;
    virtual void takeEffect(ServerPlayer *target) const = 0;

    static const DelayedTrick *CastFrom(const Card *card);

private:
    bool movable;
};

class Indulgence:public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Indulgence(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;

    virtual bool judge(const Card *card) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class Lightning: public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Lightning(Card::Suit suit, int number);
    virtual void takeEffect(ServerPlayer *target) const;
    virtual bool judge(const Card *card) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Nullification:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Nullification(Card::Suit suit, int number);

    virtual bool isAvailable() const;
};

class Weapon:public EquipCard{
    Q_OBJECT

public:
    Weapon(Suit suit, int number, int range)
        :EquipCard(suit, number), range(range){}
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

protected:
    int range;
};

class IceSword: public Weapon{
    Q_OBJECT

public:
    IceSword(Suit suit, int number);
};

class Armor:public EquipCard{
    Q_OBJECT

public:
    Armor(Suit suit, int number):EquipCard(suit, number){}
    virtual QString getSubtype() const;

    virtual Location location() const;
};

class RenwangShield: public Armor{
    Q_OBJECT

public:
    RenwangShield(Suit suit, int number);
};

class Horse:public EquipCard{
    Q_OBJECT

public:
    Horse(const QString &name, Suit suit, int number, int correct);
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    int correct;
};

// cards of standard package

class Slash: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Slash(Card::Suit suit, int number);
    DamageStruct::Nature getNature() const;

    virtual QString getSubtype() const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source,  const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool isAvailable() const;

protected:
    DamageStruct::Nature nature;
};

class Jink: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Jink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool isAvailable() const;
};

class Peach: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Peach(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable() const;
    virtual bool match(const QString &pattern) const;
};

class Shit:public BasicCard{
    Q_OBJECT
public:
    Q_INVOKABLE Shit(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onMove(const CardMoveStruct &move) const;
};

class Snatch:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Snatch(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;    
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Dismantlement: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Dismantlement(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

// Skill cards

class ZhihengCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
};

class RendeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JieyinCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinCard();
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TuxiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuxiCard();
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class FanjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanjianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class KurouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KurouCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LijianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LijianCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QingnangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GuicaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuicaiCard();
};

class LiuliCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiuliCard();
    void setSlashSource(const QString &slash_source);
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;

private:
    QString slash_source;
};

#endif // STANDARD_H
