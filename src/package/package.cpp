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

#include "package.h"
#include "skill.h"

void Package::insertRelatedSkills(const QString &main_skill, int n, ...) {
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; ++i) {
        QString c = va_arg(ap, const char *);
        related_skills.insertMulti(main_skill, c);
    }
    va_end(ap);
}

Package::~Package() {
    foreach(const Skill *skill, skills)
        delete skill;

    foreach(const QString key, patterns.keys())
        delete patterns[key];
}

Q_GLOBAL_STATIC(PackageHash, Packages)
PackageHash &PackageAdder::packages() {
    return *(::Packages());
}

