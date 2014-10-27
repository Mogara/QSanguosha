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

#ifndef _STANDARD_H
#define _STANDARD_H

#include "card.h"
#include "structs.h"

class BasicCard : public Card {
    Q_OBJECT

public:
    BasicCard(Suit suit, int number) : Card(suit, number) { handling_method = Card::MethodUse; }
    virtual QString getType() const;
    virtual CardType getTypeId() const;
};

class TrickCard : public Card {
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

class EquipCard : public Card {
    Q_OBJECT
    Q_ENUMS(Location)

public:
    enum Location {
        WeaponLocation,
        ArmorLocation,
        DefensiveHorseLocation,
        OffensiveHorseLocation,
        TreasureLocation
    };

    EquipCard(Suit suit, int number) : Card(suit, number, true) { handling_method = MethodUse; }

    virtual QString getType() const;
    virtual CardType getTypeId() const;

    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual Location location() const = 0;
};

class GlobalEffect : public TrickCard {
    Q_OBJECT

public:
    Q_INVOKABLE GlobalEffect(Card::Suit suit, int number) : TrickCard(suit, number) { target_fixed = true; }
    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool isAvailable(const Player *player) const;
};

class AOE : public TrickCard {
    Q_OBJECT

public:
    AOE(Suit suit, int number) : TrickCard(suit, number) { target_fixed = true; }
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class SingleTargetTrick : public TrickCard {
    Q_OBJECT

public:
    SingleTargetTrick(Suit suit, int number) : TrickCard(suit, number) {}
    virtual QString getSubtype() const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class DelayedTrick : public TrickCard {
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

class Disaster : public DelayedTrick {
    Q_OBJECT

public:
    Disaster(Card::Suit suit, int number);
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool isAvailable(const Player *player) const;
};

class Weapon : public EquipCard {
    Q_OBJECT

public:
    Weapon(Suit suit, int number, int range);
    int getRange() const;

    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString getCommonEffectName() const;

protected:
    int range;
};

class Armor : public EquipCard {
    Q_OBJECT

public:
    Armor(Suit suit, int number) : EquipCard(suit, number) {}
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString getCommonEffectName() const;
};

class Horse : public EquipCard {
    Q_OBJECT

public:
    Horse(Suit suit, int number, int correct);
    int getCorrect() const;

    virtual Location location() const;
    /*virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;*/

    virtual QString getCommonEffectName() const;

private:
    int correct;
};

class OffensiveHorse : public Horse {
    Q_OBJECT

public:
    Q_INVOKABLE OffensiveHorse(Card::Suit suit, int number, int correct = -1, bool is_transferable = false);
    virtual QString getSubtype() const;
};

class DefensiveHorse : public Horse {
    Q_OBJECT

public:
    Q_INVOKABLE DefensiveHorse(Card::Suit suit, int number, int correct = +1);
    virtual QString getSubtype() const;
};

class Treasure : public EquipCard {
    Q_OBJECT

public:
    Treasure(Suit suit, int number) : EquipCard(suit, number) {}
    virtual QString getSubtype() const;

    virtual Location location() const;

    virtual QString getCommonEffectName() const;
};

#endif

