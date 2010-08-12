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

int GameRule::getPriority(ServerPlayer *, ServerPlayer *) const{
    return 0;
}

void GameRule::onPhaseChange(ServerPlayer *target) const{
    Room *room = qobject_cast<Room *>(target->parent());
    switch(target->getPhase()){
    case Player::Start: {
            nextPhase(room, target);

            break;
        }
    case Player::Judge:{
            // FIXME
            nextPhase(room, target);

            break;
        }
    case Player::Draw:{
            nextPhase(room, target);

            ActiveRecord *draw = new ActiveRecord;
            draw->method = "drawCards";
            draw->target = target;
            draw->data = 2;
            room->pushActiveRecord(draw);
            break;
        }
    case Player::Play:{
            ActiveRecord *activate = new ActiveRecord;
            activate->method = "activate";
            activate->target = target;

            room->pushActiveRecord(activate);

            break;
        }
    case Player::Discard:{
            nextPhase(room, target);

            ActiveRecord *discard = new ActiveRecord;
            discard->method = "discardCards";
            discard->target = target;
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

void GameRule::nextPhase(Room *room, ServerPlayer *target) const{
    ActiveRecord *next = new ActiveRecord;
    next->method = "nextPhase";
    next->target = target;

    room->pushActiveRecord(next);
}
