/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Rara
*********************************************************************/

%{

#include "standard.h"
#include "standard-basics.h"

%}

class BasicCard: public Card {
public:
    BasicCard(Suit suit, int number): Card(suit, number), will_throw(false) {}
    virtual QString getType() const;
    virtual CardType getTypeId() const;
};

class TrickCard: public Card {
public:
    TrickCard(Suit suit, int number);
    void setCancelable(bool cancelable);

    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

private:
    bool cancelable;
};

class DelayedTrick: public TrickCard {
public:
    DelayedTrick(Suit suit, int number, bool movable = false);

private:
    bool movable;
};

class EquipCard: public Card {
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
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual Location location() const = 0;
};

%extend EquipCard {
    void equipOnInstall(ServerPlayer *player) const{
        $self->EquipCard::onInstall(player);
    }

    void equipOnUninstall(ServerPlayer *player) const{
        $self->EquipCard::onUninstall(player);
    }
};

class Weapon: public EquipCard {
public:
    Weapon(Suit suit, int number, int range);
    int getRange();
    virtual QString getSubtype() const;

    virtual Location location() const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

protected:
    int range;
};

class Armor: public EquipCard {
public:
    Armor(Suit suit, int number): EquipCard(suit, number) {}
    virtual QString getSubtype() const;

    virtual Location location() const;
};

class Horse: public EquipCard {
public:
    Horse(Suit suit, int number, int correct);

    virtual Location location() const;
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

private:
    int correct;
};

class OffensiveHorse: public Horse {
public:
    OffensiveHorse(Card::Suit suit, int number, int correct = -1);
    virtual QString getSubtype() const;
};

class DefensiveHorse: public Horse {
public:
    DefensiveHorse(Card::Suit suit, int number, int correct = +1);
    virtual QString getSubtype() const;
};

class Treasure : public EquipCard {
public:
    Treasure(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual Location location() const;
    virtual QString getCommonEffectName() const;
};

class Slash: public BasicCard {
public:
    Slash(Card::Suit suit, int number);
    DamageStruct::Nature getNature() const;
    void setNature(DamageStruct::Nature nature);

    static bool IsAvailable(const Player *player, const Card *slash = NULL, bool considerSpecificAssignee = true);
    static bool IsSpecificAssignee(const Player *player, const Player *from, const Card *slash);

protected:
    DamageStruct::Nature nature;
    mutable int drank;
};

class Analeptic: public BasicCard {
public:
    Analeptic(Card::Suit suit, int number);

    static bool IsAvailable(const Player *player, const Card *analeptic = NULL);
};
