%{

#include "standard.h"

%}

class BasicCard:public Card{
public:
    BasicCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual CardType getTypeId() const;
};

class TrickCard:public Card{
public:
    TrickCard(Suit suit, int number, bool aggressive);
    bool isAggressive() const;
    void setCancelable(bool cancelable);

    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

private:
    bool aggressive;
    bool cancelable;
};

class EquipCard:public Card{
public:
    enum Location {
        WeaponLocation,
        ArmorLocation,
        DefensiveHorseLocation,
        OffensiveHorseLocation,
    };

    EquipCard(Suit suit, int number):Card(suit, number, true), skill(NULL){}
    TriggerSkill *getSkill() const;    

    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;

    // should be pure virtual
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual Location location() const = 0;
    virtual QString label() const = 0;

protected:
    TriggerSkill *skill;
};

class Weapon:public EquipCard{
public:
    Weapon(Suit suit, int number, int range);
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString label() const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

protected:
    int range;
    bool attach_skill;
};

class Armor:public EquipCard{
public:
    Armor(Suit suit, int number):EquipCard(suit, number){}
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString label() const;
};

class Horse:public EquipCard{
public:
    Horse(Suit suit, int number, int correct);
    virtual QString getEffectPath(bool is_male) const;

    virtual Location location() const;
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual QString label() const;

private:
    int correct;
};

class OffensiveHorse: public Horse{
public:
    OffensiveHorse(Card::Suit suit, int number);
    virtual QString getSubtype() const;
};

class DefensiveHorse: public Horse{
public:
    DefensiveHorse(Card::Suit suit, int number);
    virtual QString getSubtype() const;
};