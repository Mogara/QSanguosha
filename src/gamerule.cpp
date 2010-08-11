#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"

GameRule::GameRule()
    :PassiveSkill("game_rule")
{
}

bool GameRule::triggerable(const ServerPlayer *) const{
    return true;
}

int GameRule::getPriority(ServerPlayer *) const{
    return 0;
}

void GameRule::onPhaseChange(ServerPlayer *target){
    Room *room = qobject_cast<Room *>(target->parent());
    switch(target->getPhase()){
    case Player::Start: nextPhase(room, target); break;
    case Player::Judge:{
            // FIXME
            nextPhase(room, target);

            break;
        }
    case Player::Draw:{
            nextPhase(room, target);

            ActiveRecord *draw = new ActiveRecord;
            draw->method = "drawCards";
            draw->args << Q_ARG(ServerPlayer *, target) << Q_ARG(int, 2);
            room->pushActiveRecord(draw);
            break;
        }
    case Player::Play:{
            break;
        }
    case Player::Discard:{
            nextPhase(room, target);

            ActiveRecord *discard = new ActiveRecord;
            discard->method = "discardCards";
            discard->args << Q_ARG(ServerPlayer *, target);
            room->pushActiveRecord(discard);

            break;
        }
    case Player::Finish:{
            nextPhase(room, target);

            break;
        }

    case Player::NotActive:
        break;
    }
}

void GameRule::nextPhase(Room *room, ServerPlayer *target){
    ActiveRecord *next = new ActiveRecord;
    next->method = "nextPhase";
    next->args << Q_ARG(ServerPlayer *, target);
    room->pushActiveRecord(next);
}
