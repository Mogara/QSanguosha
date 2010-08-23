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

void GameRule::getTriggerEvents(QList<Room::TriggerEvent> &events) const{
    events << Room::PhaseChange << Room::CardUsed;
}

void GameRule::onPhaseChange(ServerPlayer *player) const{
    Room *room = player->getRoom();
    switch(player->getPhase()){
    case Player::Start: nextPhase(room, player); break;
    case Player::Judge: nextPhase(room, player); break;
    case Player::Draw:{
            ActiveRecord *draw = new ActiveRecord;
            draw->method = "drawCards";
            draw->target = player;
            draw->data = 2;

            room->enqueueRecord(draw);

            nextPhase(room, player);
            break;
        }
    case Player::Play:{
            ActiveRecord *activate = new ActiveRecord;
            activate->method = "activate";
            activate->target = player;

            room->enqueueRecord(activate);

            break;
        }
    case Player::Discard:{
            ActiveRecord *discard = new ActiveRecord;
            discard->method = "discardCards";
            discard->target = player;

            room->enqueueRecord(discard);

            nextPhase(room,player);
            break;
        }
    default:
        ;
    }
}

bool GameRule::trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case Room::PhaseChange: onPhaseChange(player); break;
    default:
        ;
    }

    return false;
}

void GameRule::nextPhase(Room *room, ServerPlayer *target) const{
    ActiveRecord *next = new ActiveRecord;
    next->method = "nextPhase";
    next->target = target;

    room->enqueueRecord(next);
}
