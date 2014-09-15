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

#ifndef _GAME_RULE_H
#define _GAME_RULE_H

#include "skill.h"

class GameRule : public TriggerSkill {
    Q_OBJECT

public:
    GameRule(QObject *parent);
    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const;
    virtual int getPriority() const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const;
    QString getWinner(ServerPlayer *victim) const;

private:
    void onPhaseProceed(ServerPlayer *player) const;
    void rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const;
};

#endif

