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

#ifndef _UTIL_H
#define _UTIL_H

struct lua_State;
class QVariant;

#include <QList>
#include <QStringList>
#include <QVariant>

template<typename T>
void qShuffle(QList<T> &list) {
    int i, n = list.length();
    for (i = 0; i < n; i++) {
        int r = qrand() % (n - i) + i;
        list.swap(i, r);
    }
}

// lua interpreter related
lua_State *CreateLuaState();
void DoLuaScript(lua_State *L, const char *script);

QVariant GetValueFromLuaState(lua_State *L, const char *table_name, const char *key);

QStringList IntList2StringList(const QList<int> &intlist);
QList<int> StringList2IntList(const QStringList &stringlist);
QVariantList IntList2VariantList(const QList<int> &intlist);
QList<int> VariantList2IntList(const QVariantList &variantlist);

bool isNormalGameMode(const QString &mode);

static const int S_EQUIP_AREA_LENGTH = 5;

#endif

