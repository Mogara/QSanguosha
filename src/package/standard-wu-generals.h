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

#ifndef _STANDARD_WU_H
#define _STANDARD_WU_H

#include "standard-package.h"
#include "card.h"
#include "skill.h"

class Yingzi : public DrawCardsSkill {
public:
    Yingzi();

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual int getDrawNum(ServerPlayer *player, int n) const;
};

class Yinghun : public PhaseChangeSkill {
public:
    Yinghun();

    virtual bool canPreshow() const;
    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const;
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual bool onPhaseChange(ServerPlayer *target) const;
};

class ZhihengCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class KurouCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE KurouCard();
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FanjianCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LiuliCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LiuliCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JieyinCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JieyinCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TianxiangCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TianxiangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class TianyiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TianyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HaoshiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HaoshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DimengCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DimengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhijianCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhijianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FenxunCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif
