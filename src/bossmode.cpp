#include "bossmode.h"

static const int LoseHpTo1 = 1;
static const int ThrowAllCard = 2;

BossMode::BossMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("boss_mode");
}

bool BossMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event)
    {
    case Death:{
            const static QString evil = "lord+renegade";
            const static QString justice = "rebel+loyalist";

            player->throwAllCards();
            QStringList alive_roles = room->aliveRoles(player);
            if(!alive_roles.contains("rebel") && !alive_roles.contains("loyalist")){
                room->gameOver(evil);
                return true;
            }

            QString killer_name = data.toString();
            ServerPlayer *killer = NULL;
            if(!killer_name.isEmpty())
                killer = room->findChild<ServerPlayer *>(killer_name);

            switch(player->getRoleEnum()){
            case Player::Lord:{
                    QString winner;
                    if(!alive_roles.contains("renegade"))
                        winner = justice;
                    else{
                        if(killer == NULL || evil.contains(killer->getRole()))
                            winner = justice;
                        else
                            winner = evil;
                    }

                    room->gameOver(winner);
                    return true;
                }

            case Player::Loyalist:{
                    if(killer && killer->isLord()){
                        killer->throwAllEquips();
                        killer->throwAllHandCards();
                    }

                    break;
                }

            case Player::Rebel:{
                    if(killer)
                        killer->drawCards(3);
                    break;
                }

            default:
                break;
            }

            break;
        }

    case GameStart:{
            if(player->isLord()){
                // find guard
                QList<ServerPlayer *> players = room->getOtherPlayers(player);
                foreach(ServerPlayer *player, players){
                    if(player->getRoleEnum() == Player::Renegade){
                        room->broadcastProperty(player, "role");
                        break;
                    }
                }
            }

            break;
        }

    default:
        break;
    }

    return GameRule::trigger(event, player, data);
}
