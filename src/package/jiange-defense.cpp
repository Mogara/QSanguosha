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

#include "jiange-defense.h"
#include "skill.h"
#include "engine.h"

class JGJizhen : public PhaseChangeSkill{
public:
    JGJizhen() : PhaseChangeSkill("jgjizhen") {
        frequency = Compulsory;
    }
    
    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent /*triggerEvent*/, Room *room, ServerPlayer *player, QVariant &/*data*/, ServerPlayer* &/*ask_who*/) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish) {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->isFriendWith(player) && p->isWounded())
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent /*triggerEvent*/, Room * /*room*/, ServerPlayer *player, QVariant &/*data*/, ServerPlayer * /*ask_who*/ /* = NULL */) const{
        return player->hasShownSkill(this) || player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();

        room->notifySkillInvoked(target, objectName());

        QList<ServerPlayer *> draw_list;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (p->isFriendWith(target) && p->isWounded())
                draw_list << p;
        }
        room->sortByActionOrder(draw_list);
        room->drawCards(draw_list, 1);
        return false;
    }
};

class JGLingfeng : public PhaseChangeSkill{
public:
    JGLingfeng() : PhaseChangeSkill("jglingfeng") {
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent /*triggerEvent*/, Room * /*room*/, ServerPlayer *player, QVariant &/*data*/, ServerPlayer* &/*ask_who*/) const{
        return (TriggerSkill::triggerable(player) && player->getPhase() == Player::Draw) ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent /*triggerEvent*/, Room * /*room*/, ServerPlayer *player, QVariant &/*data*/, ServerPlayer * /*ask_who*/ /* = NULL */) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        QList<int> cardids = room->getNCards(2);
        CardsMoveStruct move(cardids, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_SHOW, target->objectName(), objectName(), QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        DummyCard dummy(cardids);
        room->obtainCard(target, &dummy);

        if (Sanguosha->getCard(cardids.first())->getColor() != Sanguosha->getCard(cardids.last())->getColor()) {
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *p, room->getOtherPlayers(target)) {
                if (!p->isFriendWith(target))
                    players << p;
            }
            if (players.isEmpty())
                return true;

            ServerPlayer *victim = room->askForPlayerChosen(target, players, objectName() + "-losehp", "@jglingfeng");
            if (victim == NULL)
                victim = players.at(qrand() % players.length());

            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, target->objectName(), victim->objectName());
            room->loseHp(victim, 1);
        }

        return true;
    }
};

JiangeDefensePackage::JiangeDefensePackage()
    : Package("jiange-defense", Package::MixedPack){

}


ADD_PACKAGE(JiangeDefense)

