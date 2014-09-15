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

#ifndef _BAN_PAIR_H
#define _BAN_PAIR_H

#include <QDialog>
#include <QPair>
#include <QListWidget>

struct BanPair : public QPair < QString, QString > {
    BanPair();
    BanPair(const QString &first, const QString &second);

    static void loadBanPairs();
    static void saveBanPairs();
    static bool isBanned(const QString &general);
    static bool isBanned(const QString &first, const QString &second);
    static const QSet<BanPair> getBanPairSet();
    static const QSet<QString> getAllBanSet();
    static const QSet<QString> getSecondBanSet();
};

#endif

