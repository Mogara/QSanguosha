/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#ifndef _STRATEGIC_ADVANTAGE_PACKAGE_H
#define _STRATEGIC_ADVANTAGE_PACKAGE_H

#include "package.h"
#include "standard.h"

class Blade : public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Blade(Card::Suit suit, int number);
};

class Halberd : public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Halberd(Card::Suit suit, int number);
};

class JadeSeal : public Treasure{
    Q_OBJECT

public:
    Q_INVOKABLE JadeSeal(Card::Suit suit, int number);
};

class Breastplate : public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Breastplate(Card::Suit suit = Card::Club, int number = 2);
};

class Drowning: public SingleTargetTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Drowning(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;

    virtual QStringList checkTargetModSkillShow(const CardUseStruct &use) const;
};

class StrategicAdvantagePackage : public Package{
    Q_OBJECT

public:
    StrategicAdvantagePackage();
};

#endif