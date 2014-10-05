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

#ifndef _STANDARD_BASICS_H
#define _STANDARD_BASICS_H

#include "standard.h"

class Slash : public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Slash(Card::Suit suit, int number);
    DamageStruct::Nature getNature() const;
    void setNature(DamageStruct::Nature nature);

    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    static bool IsAvailable(const Player *player, const Card *slash = NULL, bool considerSpecificAssignee = true);
    static bool IsSpecificAssignee(const Player *player, const Player *from, const Card *slash);

protected:
    DamageStruct::Nature nature;
    mutable int drank;
};

class NatureSlash : public Slash {
    Q_OBJECT

public:
    NatureSlash(Suit suit, int number, DamageStruct::Nature nature);
    virtual bool match(const QString &pattern) const;
};

class ThunderSlash : public NatureSlash {
    Q_OBJECT

public:
    Q_INVOKABLE ThunderSlash(Card::Suit suit, int number, bool is_transferable = false);
};

class FireSlash : public NatureSlash {
    Q_OBJECT

public:
    Q_INVOKABLE FireSlash(Card::Suit suit, int number);
};

class Jink : public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Jink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *player) const;
};

class Peach : public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Peach(Card::Suit suit, int number, bool is_transferable = false);
    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class Analeptic : public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE Analeptic(Card::Suit suit, int number, bool is_transfer = false);
    virtual QString getSubtype() const;

    static bool IsAvailable(const Player *player, const Card *analeptic = NULL);

    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    virtual QStringList checkTargetModSkillShow(const CardUseStruct &use) const;
};

#endif

