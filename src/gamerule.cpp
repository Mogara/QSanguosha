#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"
#include "standard.h"

GameRule::GameRule()
    :TriggerSkill("game_rule")
{
    events << GameStart << PhaseChange << CardUsed << Predamaged
            << CardEffected << Death << Dying
            << SlashResult << SlashEffected << SlashProceed;
}

bool GameRule::triggerable(const ServerPlayer *) const{
    return true;
}

int GameRule::getPriority(ServerPlayer *) const{
    return 0;
}

void GameRule::onPhaseChange(ServerPlayer *player) const{
    Room *room = player->getRoom();
    switch(player->getPhase()){
    case Player::Start: {
            if(player->hasFlag("drank"))
                room->setPlayerFlag(player, "-drank");
            break;
        }
    case Player::Judge: {
            QStack<const DelayedTrick *> tricks = player->delayedTricks();
            while(!tricks.isEmpty()){
                const DelayedTrick *trick = tricks.pop();
                bool on_effect = room->cardEffect(trick, NULL, player);
                if(!on_effect)
                    trick->onNullified(player);
            }

            break;
        }
    case Player::Draw: {
            player->drawCards(2);
            break;
        }

    case Player::Play: {
            forever{
                QString card = room->activate(player);
                if(card == ".")
                    break;

                room->useCard(player, card);
            }
            break;
        }

    case Player::Discard:{
            int discard_num = player->getHandcardNum() - player->getMaxCards();
            if(discard_num > 0)
                room->askForDiscard(player, discard_num);
            break;
        }
    case Player::Finish: break;
    case Player::NotActive: return;
    }
}

bool GameRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart: player->drawCards(4); break;
    case PhaseChange: onPhaseChange(player); break;
    case CardUsed: {
            if(data.canConvert<CardUseStruct>()){
                CardUseStruct card_use = data.value<CardUseStruct>();
                const Card *card = card_use.card;

                card_use.from->playCardEffect(card);
                card->use(room, card_use.from, card_use.to);
            }

            break;
        }

    case Dying:{
            DyingStruct dying = data.value<DyingStruct>();
            bool saved = room->askForSave(player, dying.peaches);
            if(saved)
                room->setPlayerProperty(player, "hp", 1);
            else{
                QVariant killer_name;
                if(dying.damage && dying.damage->from)
                    killer_name = dying.damage->from->objectName();
                room->getThread()->trigger(Death, player, killer_name);
            }
            break;
        }

    case Predamaged:{
            if(data.canConvert<DamageStruct>()){
                DamageStruct damage = data.value<DamageStruct>();
                bool chained = player->isChained();
                int new_hp = player->getHp() - damage.damage;
                room->damage(player, damage.damage);
                if(new_hp <= 0){
                    DyingStruct dying;
                    dying.damage = &damage;
                    dying.peaches = 1 - new_hp;

                    QVariant dying_data = QVariant::fromValue(dying);
                    room->getThread()->trigger(Dying, player, dying_data);
                }

                if(damage.nature != DamageStruct::Normal && chained){
                    room->setPlayerProperty(player, "chained", false);

                    // iron chain effect
                    QList<ServerPlayer *> chained_players = room->getAllPlayers();
                    chained_players.removeOne(player);

                    foreach(ServerPlayer *chained_player, chained_players){
                        if(chained_player->isChained()){
                            DamageStruct chain_damage = damage;
                            chain_damage.to = chained_player;

                            room->setPlayerProperty(chained_player, "chained", false);
                            room->damage(chain_damage);
                        }
                    }
                }
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

    case SlashEffected:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            QVariant data = QVariant::fromValue(effect);
            room->getThread()->trigger(SlashProceed, effect.from, data);

            break;
        }

    case SlashProceed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            QString slasher = effect.from->getGeneralName();
            bool success = !room->askForCard(effect.to, "jink", "slash-jink:" + slasher);

            SlashResultStruct result;
            result.fill(effect, success);
            room->slashResult(result);

            break;
        }

    case SlashResult:{
            SlashResultStruct result = data.value<SlashResultStruct>();
            if(result.success){
                DamageStruct damage;
                damage.card = result.slash;

                damage.damage = 1;
                if(result.drank)
                    damage.damage ++;                

                damage.from = result.from;
                damage.to = result.to;
                damage.nature = result.nature;

                room->damage(damage);
            }

            if(result.to->hasFlag("armor_nullified"))
                room->setPlayerFlag(result.to, "-armor_nullified");

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


