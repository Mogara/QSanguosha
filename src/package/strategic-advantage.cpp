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
#include "skill.h"

Blade::Blade(Card::Suit suit, int number)
: Weapon(suit, number, 3){
    setObjectName("Blade");
}

class BladeSkill : public WeaponSkill{
public:
    BladeSkill(): WeaponSkill("Blade"){
        events << TargetChosen << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (triggerEvent == TargetChosen){
            if (!WeaponSkill::triggerable(use.from))
                return QStringList();

            if (use.to.contains(player) && use.card->isKindOf("Slash")){
                ask_who = use.from;
                return QStringList(objectName());
            }
        }
        else {
            if (use.card->isKindOf("Slash")){
                foreach(ServerPlayer *p, use.to){
                    QStringList blade_use = p->property("blade_use").toStringList();
                    if (!blade_use.contains(use.card->toString()))
                        return QStringList();

                    blade_use.removeOne(use.card->toString());
                    room->setPlayerProperty(p, "blade_use", blade_use);

                    if (blade_use.isEmpty())
                        room->setPlayerMark(p, "@blade", 0);
                }
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        QStringList blade_use = player->property("blade_use").toStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (blade_use.contains(use.card->toString()))
            return false;

        blade_use << use.card->toString();
        room->setPlayerProperty(player, "blade_use", blade_use);
        
        if (!player->hasShownAllGenerals())
            room->setPlayerMark(player, "@blade", 1);

        return false;
    }
};

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
        << new JadeSeal(Card::Spade, 4)
        << new Blade(Card::Spade, 5);

    foreach(Card *c, cards){
        c->setParent(this);
    }

    skills << new BladeSkill;
}


ADD_PACKAGE(StrategicAdvantage)

