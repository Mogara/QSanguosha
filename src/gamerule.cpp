#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"
#include "standard.h"

GameRule::GameRule(QObject *parent)
    :TriggerSkill("game_rule")
{
    setParent(parent);

    events << GameStart << PhaseChange << CardUsed
            << Predamaged << CardEffected << HpRecover
            << AskForPeaches << Death << Dying
            << SlashResult << SlashEffected << SlashProceed;
}

bool GameRule::triggerable(const ServerPlayer *) const{
    return true;
}

int GameRule::getPriority() const{
    return 0;
}

void GameRule::onPhaseChange(ServerPlayer *player) const{
    Room *room = player->getRoom();
    switch(player->getPhase()){
    case Player::Start: break;
    case Player::Judge: {
            QStack<const DelayedTrick *> tricks = player->delayedTricks();
            while(!tricks.isEmpty() && player->isAlive()){
                const DelayedTrick *trick = tricks.pop();
                bool on_effect = room->cardEffect(trick, NULL, player);
                if(!on_effect)
                    trick->onNullified(player);
            }

            break;
        }
    case Player::Draw: {
            QVariant num = 2;
            room->getThread()->trigger(DrawNCards, player, num);
            int n = num.toInt();
            if(n > 0)
                player->drawCards(n, false);
            break;
        }

    case Player::Play: {
            while(player->isAlive()){
                CardUseStruct card_use;
                room->activate(player, card_use);
                if(card_use.isValid())
                    room->useCard(card_use);
                else
                    break;
            }
            break;
        }

    case Player::Discard:{
            int discard_num = player->getHandcardNum() - player->getMaxCards();
            if(player->hasFlag("jilei")){
                QList<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(card->inherits("BasicCard")){
                        if(player->hasFlag("jileiB"))
                            jilei_cards << card;
                    }else if(card->inherits("EquipCard")){
                        if(player->hasFlag("jileiE"))
                            jilei_cards << card;
                    }else if(card->inherits("TrickCard")){
                        if(player->hasFlag("jileiT"))
                            jilei_cards << card;
                    }
                }

                if(jilei_cards.length() > player->getMaxCards()){
                    // show all his cards
                    QStringList handcards_str;
                    foreach(const Card *card, handcards)
                        handcards_str << QString::number(card->getId());
                    QString gongxin_str = QString("%1!:%2").arg(player->objectName()).arg(handcards_str.join("+"));
                    room->broadcastInvoke("doGongxin", gongxin_str, player);

                    QList<const Card *> other_cards = handcards.toSet().subtract(jilei_cards.toSet()).toList();
                    foreach(const Card *card, other_cards){
                        room->throwCard(card);
                    }

                    return;
                }
            }


            if(discard_num > 0)
                room->askForDiscard(player, discard_num);
            break;
        }
    case Player::Finish: {
            if(player->hasFlag("drank"))
                room->setPlayerFlag(player, "-drank");
            break;
        }
    case Player::NotActive: return;
    }
}

bool GameRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart: {
            if(player->getKingdom() == "god"){
                QString new_kingdom = room->askForKingdom(player);
                if(new_kingdom != "god")
                    room->setPlayerProperty(player, "kingdom", new_kingdom);

                LogMessage log;
                log.type = "#ChooseKingdom";
                log.from = player;
                log.arg = new_kingdom;
                room->sendLog(log);
            }

            player->drawCards(4, false);
            break;
        }

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

    case HpRecover:{
            int recover = data.toInt();

            int new_hp = qMin(player->getHp() + recover, player->getMaxHP());
            room->setPlayerProperty(player, "hp", new_hp);
            room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(recover));

            break;
        }

    case AskForPeaches:{
            DyingStruct dying = data.value<DyingStruct>();
            int got = room->askForPeach(player, dying.who, dying.peaches);
            dying.peaches -= got;

            data = QVariant::fromValue(dying);

            break;
        }

    case Dying:{
            DyingStruct dying = data.value<DyingStruct>();
            QList<ServerPlayer *> players = room->getAllPlayers();
            room->askForPeaches(dying, players);

            break;
        }

    case Predamaged:{
            DamageStruct damage = data.value<DamageStruct>();
            room->sendDamageLog(damage);

            bool chained = player->isChained();

            int new_hp = player->getHp() - damage.damage;
            room->damage(player, damage.damage);
            if(new_hp <= 0){
                DyingStruct dying;
                dying.who = player;
                dying.damage = &damage;
                dying.peaches = 1 - new_hp;

                QVariant dying_data = QVariant::fromValue(dying);
                room->getThread()->trigger(Dying, player, dying_data);
            }

            if(chained && damage.nature != DamageStruct::Normal){
                room->setPlayerProperty(player, "chained", false);

                // iron chain effect
                QList<ServerPlayer *> chained_players = room->getAllPlayers();
                chained_players.removeOne(player);

                foreach(ServerPlayer *chained_player, chained_players){
                    if(chained_player->isChained()){
                        DamageStruct chain_damage = damage;
                        chain_damage.to = chained_player;
                        chain_damage.chain = true;

                        room->setPlayerProperty(chained_player, "chained", false);
                        room->damage(chain_damage);
                    }
                }
            }

            break;
        }        

    case CardEffected:{
            if(data.canConvert<CardEffectStruct>()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(room->isCanceled(effect))
                    return true;

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
            player->throwAllCards();

            if(room->getTag("SkipNormalDeathProcess").toBool())
                return false;

            QString winner;
            QStringList alive_roles = room->aliveRoles(player);

            switch(player->getRoleEnum()){
            case Player::Lord:{
                    if(alive_roles.length() == 1 && alive_roles.first() == "renegade")
                        winner = "renegade";
                    else
                        winner = "rebel";
                    break;
                }

            case Player::Rebel:
            case Player::Renegade:
                {
                    if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade"))
                        winner = "lord+loyalist";
                    break;
                }

            default:
                break;
            }

            if(winner.isNull()){
                QString killer_name = data.toString();
                if(!killer_name.isEmpty()){
                    ServerPlayer *killer = room->findChild<ServerPlayer *>(killer_name);

                    if(player->getRole() == "rebel" && killer != player)
                        killer->drawCards(3);
                    else if(player->getRole() == "loyalist" && killer->getRole() == "lord"){
                        killer->throwAllEquips();
                        killer->throwAllHandCards();
                    }
                }
            }else{
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


