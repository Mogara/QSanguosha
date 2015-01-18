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

#include "settings.h"
#include "engine.h"
#include "scenerule.h"

SceneRule::SceneRule(QObject *parent) : GameRule(parent) {
    events << GameStart;
}

int SceneRule::getPriority() const{
    return -2;
}

bool SceneRule::effect(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
    QStringList extensions = Sanguosha->getExtensions();
    QSet<QString> ban_packages = Config.BanPackages.toSet();

    if (!player && triggerEvent == GameStart){
        foreach (const QString &extension, extensions){
            bool forbid_package = Config.value("ForbidPackages").toStringList().contains(extension);
            if (ban_packages.contains(extension) || forbid_package) continue;

            QString skill = QString("#%1").arg(extension);
            if (extension.startsWith("scene") && Sanguosha->getSkill(skill)){
                foreach (ServerPlayer *p, room->getPlayers())
                    room->acquireSkill(p, skill);
            }
        }
    }

    return GameRule::effect(triggerEvent, room, player, data, player);
}
