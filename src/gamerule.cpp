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

void GameRule::getTriggerEvents(QList<TriggerEvent> &events) const{
    events << GameStart << PhaseChange << CardUsed << Damaged;
}

void GameRule::onPhaseChange(ServerPlayer *player) const{
    Room *room = player->getRoom();
    switch(player->getPhase()){
    case Player::Start: room->nextPhase(player); break;
    case Player::Judge: room->nextPhase(player); break;
    case Player::Draw: room->drawCards(player, 2); room->nextPhase(player); break;
    case Player::Play: {
            forever{
                QString card = room->activate(player);
                if(card == ".")
                    break;

                room->useCard(player, card);
            }
            room->nextPhase(player);
            break;
        }
    case Player::Discard:
    default:
        ;
    }
}

bool GameRule::trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart: room->drawCards(player, 4); break;
    case PhaseChange: onPhaseChange(player); break;
    case CardUsed: {
            if(data.canConvert<CardUseStruct>()){
                CardUseStruct card_use = data.value<CardUseStruct>();
                const Card *card = card_use.card;

                card->use(room, card_use.from, card_use.to);
            }

            break;
        }
    case Damaged: {
            if(data.canConvert<DamageStruct>()){
                DamageStruct damage = data.value<DamageStruct>();
                room->damage(damage.to, damage.damage);
            }
            break;
        }

    default:
        ;
    }

    return false;
}


