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

#include "strategic-advantage.h"

JadeSeal::JadeSeal(Card::Suit suit, int number)
: Treasure(suit, number){
    setObjectName("JadeSeal");
}

StrategicAdvantagePackage::StrategicAdvantagePackage()
: Package("strategic-advantage", Package::CardPack){
    QList<Card *> cards;

    cards
        << new JadeSeal(Card::Spade, 2)
        << new JadeSeal(Card::Spade, 3)
        << new JadeSeal(Card::Spade, 4);

    foreach(Card *c, cards){
        c->setParent(this);
    }
}


ADD_PACKAGE(StrategicAdvantage)

