#ifndef _STANDARD_H
#define _STANDARD_H

#include "package.h"
#include "card.h"
#include "roomthread.h"
#include "skill.h"

class Mashu: public DistanceSkill {
public:
    Mashu(const QString &);

    virtual int getCorrect(const Player *from, const Player *) const;
private:
    QString owner;
};

class TuxiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TuxiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShensuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShensuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class RendeCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhihengCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class KurouCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE KurouCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FanjianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LiuliCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LiuliCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JieyinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JieyinCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QingnangCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LijianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LijianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LuanwuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiongyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XiongyiCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShuangrenCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShuangrenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuoshuiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuoshuiCard();
};

class QingchengCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingchengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class StandardPackage: public Package {
    Q_OBJECT

public:
    StandardPackage();
    void addWeiGenerals();
    void addShuGenerals();
    void addWuGenerals();
    void addQunGenerals();
};

class TestPackage: public Package {
    Q_OBJECT

public:
    TestPackage();
};

class BasicCard: public Card {
    Q_OBJECT

public:
    BasicCard(Suit suit, int number): Card(suit, number) { handling_method = Card::MethodUse;}
    virtual QString getType() const;
    virtual CardType getTypeId() const;
};

class TrickCard:public Card {
    Q_OBJECT

public:
    TrickCard(Suit suit, int number);
    void setCancelable(bool cancelable);

    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

private:
    bool cancelable;
};

class EquipCard: public Card {
    Q_OBJECT
    Q_ENUMS(Location)

public:
    enum Location {
        WeaponLocation,
        ArmorLocation,
        DefensiveHorseLocation,
        OffensiveHorseLocation
    };

    EquipCard(Suit suit, int number): Card(suit, number, true) { handling_method = MethodUse; }

    virtual QString getType() const;
    virtual CardType getTypeId() const;

    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual Location location() const = 0;
};

class GlobalEffect: public TrickCard {
    Q_OBJECT

public:
    Q_INVOKABLE GlobalEffect(Card::Suit suit, int number): TrickCard(suit, number) { target_fixed = true; }
    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool isAvailable(const Player *player) const;
};

class GodSalvation: public GlobalEffect {
    Q_OBJECT

public:
    Q_INVOKABLE GodSalvation(Card::Suit suit = Heart, int number = 1);
    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AmazingGrace: public GlobalEffect {
    Q_OBJECT

public:
    Q_INVOKABLE AmazingGrace(Card::Suit suit = Heart, int number = 3);
    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

private:
    void clearRestCards(Room *room) const;
};

class AOE: public TrickCard {
    Q_OBJECT

public:
    AOE(Suit suit, int number): TrickCard(suit, number) { target_fixed = true; }
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class SavageAssault:public AOE {
    Q_OBJECT

public:
    Q_INVOKABLE SavageAssault(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ArcheryAttack: public AOE {
    Q_OBJECT

public:
    Q_INVOKABLE ArcheryAttack(Card::Suit suit = Heart, int number = 1);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SingleTargetTrick: public TrickCard {
    Q_OBJECT

public:
    SingleTargetTrick(Suit suit, int number): TrickCard(suit, number) {}
    virtual QString getSubtype() const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class Collateral: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Collateral(Card::Suit suit = Club, int number = 12);
    virtual bool isAvailable(const Player *player) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

private:
    bool doCollateral(Room *room, ServerPlayer *killer, ServerPlayer *victim, const QString &prompt) const;
};

class ExNihilo: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE ExNihilo(Card::Suit suit, int number);
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class Duel: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Duel(Card::Suit suit, int number);
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DelayedTrick: public TrickCard {
    Q_OBJECT

public:
    DelayedTrick(Suit suit, int number, bool movable = false);
    virtual void onNullified(ServerPlayer *target) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual QString getSubtype() const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void takeEffect(ServerPlayer *target) const = 0;

protected:
    JudgeStruct judge;

private:
    bool movable;
};

class Indulgence: public DelayedTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Indulgence(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class SupplyShortage: public DelayedTrick {
    Q_OBJECT

public:
    Q_INVOKABLE SupplyShortage(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class Disaster: public DelayedTrick {
    Q_OBJECT

public:
    Disaster(Card::Suit suit, int number);
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool isAvailable(const Player *player) const;
};

class Lightning: public Disaster {
    Q_OBJECT

public:
    Q_INVOKABLE Lightning(Card::Suit suit = Spade, int number = 1);

    virtual void takeEffect(ServerPlayer *target) const;
};

class Nullification: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Nullification(Card::Suit suit = Spade, int number = 11);

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
};

class HegNullification: public Nullification {
    Q_OBJECT

public:
    Q_INVOKABLE HegNullification(Card::Suit suit, int number);
};

class Weapon: public EquipCard {
    Q_OBJECT

public:
    Weapon(Suit suit, int number, int range);
    int getRange() const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString getCommonEffectName() const;
    virtual bool isAvailable(const Player *player) const;

protected:
    int range;
};

class Armor: public EquipCard {
    Q_OBJECT

public:
    Armor(Suit suit, int number): EquipCard(suit, number) {}
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString getCommonEffectName() const;
};

class Horse: public EquipCard {
    Q_OBJECT

public:
    Horse(Suit suit, int number, int correct);
    int getCorrect() const;

    virtual Location location() const;
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual QString getCommonEffectName() const;

private:
    int correct;
};

class OffensiveHorse: public Horse {
    Q_OBJECT

public:
    Q_INVOKABLE OffensiveHorse(Card::Suit suit, int number, int correct = -1);
    virtual QString getSubtype() const;
};

class DefensiveHorse: public Horse {
    Q_OBJECT

public:
    Q_INVOKABLE DefensiveHorse(Card::Suit suit, int number, int correct = +1);
    virtual QString getSubtype() const;
};

// cards of standard package

class Slash: public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Slash(Card::Suit suit, int number);
    DamageStruct::Nature getNature() const;
    void setNature(DamageStruct::Nature nature);

    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    static bool IsAvailable(const Player *player, const Card *slash = NULL, bool considerSpecificAssignee = true);
    static bool IsSpecificAssignee(const Player *player, const Player *from, const Card *slash);

protected:
    DamageStruct::Nature nature;
    mutable int drank;
};

class NatureSlash: public Slash {
    Q_OBJECT

public:
    NatureSlash(Suit suit, int number, DamageStruct::Nature nature);
    virtual bool match(const QString &pattern) const;
};

class ThunderSlash: public NatureSlash {
    Q_OBJECT

public:
    Q_INVOKABLE ThunderSlash(Card::Suit suit, int number);
};

class FireSlash: public NatureSlash {
    Q_OBJECT

public:
    Q_INVOKABLE FireSlash(Card::Suit suit, int number);
};

class Jink: public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Jink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *player) const;
};

class Peach: public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Peach(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class Analeptic: public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Analeptic(Card::Suit suit, int number);
    virtual QString getSubtype() const;

    static bool IsAvailable(const Player *player, const Card *analeptic = NULL);

    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Snatch:public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Snatch(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Dismantlement: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Dismantlement(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class IronChain: public TrickCard {
    Q_OBJECT

public:
    Q_INVOKABLE IronChain(Card::Suit suit, int number);

    virtual QString getSubtype() const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FireAttack: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE FireAttack(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AwaitExhausted: public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE AwaitExhausted(Card::Suit suit, int number);

    virtual QString getSubtype() const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KnownBoth: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE KnownBoth(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BefriendAttacking: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE BefriendAttacking(Card::Suit suit = Heart, int number = 9);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

#endif

