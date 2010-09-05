#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"

GameRule::GameRule()
    :TriggerSkill("game_rule")
{
}

bool GameRule::triggerable(const ServerPlayer *) const{
    return true;
}

int GameRule::getPriority(ServerPlayer *) const{
    return 0;
}

void GameRule::getTriggerEvents(QList<TriggerEvent> &events) const{
    events << GameStart << PhaseChange << CardUsed << Predamaged << Damaged << CardEffected << Dying << Death;
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
    case Player::Discard:{
            int discard_num = player->getMaxCards() - player->getHandcardNum();
            if(discard_num > 0){
                QList<int> card_ids = room->askForDiscard(player, discard_num);
                foreach(int card_id, card_ids)
                    room->throwCard(card_id);
            }

            room->nextPhase(player);
            break;
        }
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
    case Predamaged:{
            if(data.canConvert<DamageStruct>()){
                DamageStruct damage = data.value<DamageStruct>();
                if(player->getHp() - damage.damage <= 0){
                    QString killer_name;
                    if(damage.from)
                        killer_name = damage.from->objectName();
                    room->getThread()->invokePassiveSkills(Death, player, killer_name);
                }
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

    case CardEffected:{
            if(data.canConvert<CardEffectStruct>()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                effect.card->onEffect(effect);
            }

            break;
        }
    case Dying:{
            // FIXME

            break;
        }

    case Death:{
            QString winner;
            QStringList alive_roles = room->aliveRoles(player);

            if(player->getRole() == "lord"){
                if(alive_roles.length() == 1 && alive_roles.first() == "renegade")
                    winner = "renegade";
                else
                    winner = "rebel";
            }else if(player->getRole() == "rebel" || player->getRole() == "renegade"){
                if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade"))
                    winner = "lord";
            }

            QString killer_name = data.toString();
            ServerPlayer *killer = NULL;
            if(!killer_name.isEmpty())
                killer = room->findChild<ServerPlayer *>(killer_name);
            room->obit(player, killer);

            if(winner.isNull()){
                room->bury(player);
                if(killer){
                    if(player->getRole() == "rebel" && killer != player)
                        killer->drawCards(3);
                    else if(player->getRole() == "loyalist" && killer->getRole() == "lord")
                        killer->throwAllCards();
                }
            }else{
                player->throwAllCards();
                room->gameOver(winner);
                return true;
            }

            break;
        }
    default:
        ;
    }

    return false;
}


