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
#include "engine.h"

Blade::Blade(Card::Suit suit, int number)
    : Weapon(suit, number, 3){
    setObjectName("Blade");
}

class BladeSkill : public WeaponSkill{
public:
    BladeSkill() : WeaponSkill("Blade"){
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
                        room->removePlayerDisableShow(p, "Blade");
                }
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        QStringList blade_use = player->property("blade_use").toStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (blade_use.contains(use.card->toString()))
            return false;

        blade_use << use.card->toString();
        room->setPlayerProperty(player, "blade_use", blade_use);

        if (!player->hasShownAllGenerals())
            room->setPlayerDisableShow(player, "hd", "Blade");

        return false;
    }
};

Halberd::Halberd(Card::Suit suit, int number)
    : Weapon(suit, number, 4){
    setObjectName("Halberd");
}

JadeSeal::JadeSeal(Card::Suit suit, int number)
    : Treasure(suit, number){
    setObjectName("JadeSeal");
}

Breastplate::Breastplate(Card::Suit suit, int number)
    : Armor(suit, number){
    setObjectName("Breastplate");
}

Drowning::Drowning(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("drowning");
}

bool Drowning::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    return to_select->hasEquip() && to_select != Self;
}

void Drowning::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (!effect.to->getEquips().isEmpty()
        && room->askForChoice(effect.to, objectName(), "throw+damage", QVariant::fromValue(effect)) == "throw")
        effect.to->throwAllEquips();
    else
        room->damage(DamageStruct(this, effect.from->isAlive() ? effect.from : NULL, effect.to));
}

bool Drowning::isAvailable(const Player *player) const{
    bool canUse = false;
    QList<const Player *> players = player->getAliveSiblings();
    foreach(const Player *p, players) {
        if (player->isProhibited(p, this))
            continue;
        if (!p->hasEquip())
            continue;
        canUse = true;
        break;
    }

    return canUse && TrickCard::isAvailable(player);
}

QStringList Drowning::checkTargetModSkillShow(const CardUseStruct &use) const{
    if (use.to.length() >= 2){
        const ServerPlayer *from = use.from;
        QList<const Skill *> skills = from->getSkillList(false, false);
        QList<const TargetModSkill *> tarmods;

        foreach(const Skill *skill, skills){
            if (from->hasSkill(skill->objectName()) && skill->inherits("TargetModSkill")) {
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }

        if (tarmods.isEmpty())
            return QStringList();

        int n = use.to.length() - 1;
        QList<const TargetModSkill *> tarmods_copy = tarmods;

        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            if (from->hasShownSkill(main_skill)){
                tarmods.removeOne(tarmod);
                n -= tarmod->getExtraTargetNum(from, this);
            }
        }

        if (tarmods.isEmpty() || n <= 0)
            return QStringList();

        tarmods_copy = tarmods;

        QStringList shows;
        foreach(const TargetModSkill *tarmod, tarmods_copy){
            const Skill *main_skill = Sanguosha->getMainSkill(tarmod->objectName());
            shows << main_skill->objectName();
        }
        return shows;
    }
    return QStringList();
}

AllianceFeast::AllianceFeast(Card::Suit suit, int number)
    : AOE(suit, number)
{
    setObjectName("alliance_feast");
    target_fixed = false;
}

StrategicAdvantagePackage::StrategicAdvantagePackage()
    : Package("strategic_advantage", Package::CardPack){
    QList<Card *> cards;

    cards
        << new JadeSeal(Card::Spade, 2)
        << new Blade(Card::Spade, 5)
        << new Halberd(Card::Spade, 6)
        << new Breastplate()
        << new Drowning(Card::Club, 2)
        << new Drowning(Card::Club, 3)
        << new Drowning(Card::Club, 4)
        << new AllianceFeast();

    foreach(Card *c, cards){
        c->setParent(this);
    }

    skills << new BladeSkill;
}

ADD_PACKAGE(StrategicAdvantage)
