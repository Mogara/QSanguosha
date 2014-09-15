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

#ifndef SCENERULE_H
#define SCENERULE_H

#include "gamerule.h"


class SceneRule : public GameRule {
public:
    SceneRule(QObject *parent);
    virtual int getPriority() const;
    virtual bool effect(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
};

#endif // SCENERULE_H
