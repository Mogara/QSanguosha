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

#ifndef _EXPPATTERN_H
#define _EXPPATTERN_H

#include "package.h"
#include "card.h"
#include "player.h"

class ExpPattern : public CardPattern {
public:
    ExpPattern(const QString &exp);
    virtual bool match(const Player *player, const Card *card) const;
    virtual QString getPatternString() const{
        return exp;
    }
private:
    QString exp;
    bool matchOne(const Player *player, const Card *card, QString exp) const;
};

#endif

