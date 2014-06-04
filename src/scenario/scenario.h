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

#ifndef _SCENARIO_H
#define _SCENARIO_H

#include "package.h"
#include "ai.h"

class Room;
class ScenarioRule;

#include <QMap>

class Scenario : public Package {
    Q_OBJECT

public:
    explicit Scenario(const QString &name);
    ScenarioRule *getRule() const;

    virtual bool exposeRoles() const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
    virtual void onTagSet(Room *room, const QString &key) const = 0;
    virtual bool generalSelection() const;

protected:
    QString lord;
    QStringList loyalists, rebels, renegades;
    ScenarioRule *rule;
};

#endif

