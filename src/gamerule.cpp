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
    events << GameStart << PhaseChange << CardUsed << Damaged << CardEffected;
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

    case CardEffected:{
            if(data.canConvert<CardEffectStruct>()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                QString card_name = effect.card->objectName();

                if(card_name == "slash"){
                    const Card *card = room->requestForCard(effect.to, "jink");
                    if(!card){
                        DamageStruct damage;
                        damage.card = effect.card;
                        damage.damage = 1;
                        damage.from = effect.from;
                        damage.to = effect.to;

                        if(effect.flags.contains("fire"))
                            damage.nature = DamageStruct::Fire;
                        else if(effect.flags.contains("thunder"))
                            damage.nature = DamageStruct::Thunder;
                        else
                            damage.nature = DamageStruct::Normal;

                        room->damage(damage);
                    }
                }
            }


            break;
        }

    default:
        ;
    }

    return false;
}


