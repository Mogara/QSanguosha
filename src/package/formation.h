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

#ifndef _H_FORMATION_H
#define _H_FORMATION_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class ZiliangCard : public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZiliangCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuyuanCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuyuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HeyiSummon : public ArraySummonCard {
    Q_OBJECT

public:
    Q_INVOKABLE HeyiSummon();
};

class TiaoxinCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TiaoxinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TianfuSummon : public ArraySummonCard {
    Q_OBJECT

public:
    Q_INVOKABLE TianfuSummon();
};

class ShangyiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShangyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NiaoxiangSummon : public ArraySummonCard {
    Q_OBJECT

public:
    Q_INVOKABLE NiaoxiangSummon();
};

class QianhuanCard : public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QianhuanCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FormationPackage : public Package {
    Q_OBJECT

public:
    FormationPackage();
};

class DragonPhoenix : public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE DragonPhoenix(Card::Suit suit = Spade, int number = 2);
};

class FormationEquipPackage : public Package{
    Q_OBJECT

public:
    FormationEquipPackage();
};

#endif
