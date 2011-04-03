#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"
#include "standard.h"

GameRule::GameRule(QObject *parent)
    :TriggerSkill("game_rule")
{
    setParent(parent);

    events << GameStart << PhaseChange << CardUsed
            << CardEffected << HpRecover << AskForPeachesDone
            << AskForPeaches << Death << Dying
            << SlashHit << SlashMissed << SlashEffected << SlashProceed
            << DamageDone << DamageComplete;
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
    case Player::Start: {
            player->setMark("SlashCount", 0);
            break;
        }
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
                    room->showAllCards(player);

                    QList<const Card *> other_cards = handcards.toSet().subtract(jilei_cards.toSet()).toList();
                    foreach(const Card *card, other_cards){
                        room->throwCard(card);
                    }

                    return;
                }
            }


            if(discard_num > 0)
                room->askForDiscard(player, "gamerule", discard_num);
            break;
        }
    case Player::Finish: {
            player->clearFlags();
            break;
        }

    case Player::NotActive: return;
    }
}

void GameRule::setGameProcess(Room *room) const{
    int good = 0, bad = 0;
    QList<ServerPlayer *> players = room->getAlivePlayers();
    foreach(ServerPlayer *player, players){
        switch(player->getRoleEnum()){
        case Player::Lord:
        case Player::Loyalist: good ++; break;
        case Player::Rebel: bad++; break;
        case Player::Renegade: break;
        }
    }

    QString process;
    if(good == bad)
        process = "Balance";
    else if(good > bad)
        process = "LordSuperior";
    else
        process = "RebelSuperior";

    room->setTag("GameProcess", process);
}

bool GameRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart: {
            if(player->getKingdom() == "god"){
                QString new_kingdom = room->askForKingdom(player);
                room->setPlayerProperty(player, "kingdom", new_kingdom);

                LogMessage log;
                log.type = "#ChooseKingdom";
                log.from = player;
                log.arg = new_kingdom;
                room->sendLog(log);
            }

            if(player->isLord())
                setGameProcess(room);

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
            RecoverStruct recover_struct = data.value<RecoverStruct>();
            int recover = recover_struct.recover;

            int new_hp = qMin(player->getHp() + recover, player->getMaxHP());
            room->setPlayerProperty(player, "hp", new_hp);
            room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(recover));

            break;
        }

    case Dying:{
            if(player->getHp() > 0){
                player->setFlags("-dying");
                break;
            }

            QList<ServerPlayer *> savers;
            ServerPlayer *current = room->getCurrent();
            if(current->hasSkill("wansha") && current->isAlive()){
                room->playSkillEffect("wansha");

                savers << current;

                LogMessage log;
                log.from = current;
                if(current != player){
                    savers << player;
                    log.type = "#WanshaTwo";
                    log.to << player;
                }else{
                    log.type = "#WanshaOne";
                }

                room->sendLog(log);

            }else
                savers = room->getAllPlayers();

            LogMessage log;
            log.type = "#AskForPeaches";
            log.from = player;
            log.to = savers;
            log.arg = QString::number(1 - player->getHp());
            room->sendLog(log);

            RoomThread *thread = room->getThread();
            foreach(ServerPlayer *saver, savers){
                if(player->getHp() > 0)
                    break;

                thread->trigger(AskForPeaches, saver, data);
            }

            player->setFlags("-dying");
            thread->trigger(AskForPeachesDone, player, data);

            break;
        }

    case AskForPeaches:{
            DyingStruct dying = data.value<DyingStruct>();

            while(dying.who->getHp() <= 0){
                const Card *peach = room->askForSinglePeach(player, dying.who);
                if(peach == NULL)
                    break;

                CardUseStruct use;
                use.card = peach;
                use.from = player;
                if(player != dying.who)
                    use.to << dying.who;
                room->useCard(use);
            }

            break;
        }

    case AskForPeachesDone:{
            if(player->getHp() <= 0){
                DyingStruct dying = data.value<DyingStruct>();
                ServerPlayer *killer = NULL;
                if(dying.damage)
                    killer = dying.damage->from;

                room->killPlayer(player, killer);
            }

            break;
        }

    case DamageDone:{
            DamageStruct damage = data.value<DamageStruct>();
            room->sendDamageLog(damage);

            room->applyDamage(player, damage);
            if(player->getHp() <= 0){
                room->enterDying(player, &damage);
            }

            break;
        }        

    case DamageComplete:{
            bool chained = player->isChained();
            if(!chained)
                break;

            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal){
                room->setPlayerProperty(player, "chained", false);

                // iron chain effect
                QList<ServerPlayer *> chained_players = room->getOtherPlayers(player);
                foreach(ServerPlayer *chained_player, chained_players){
                    if(chained_player->isChained()){
                        room->getThread()->delay();

                        DamageStruct chain_damage = damage;
                        chain_damage.to = chained_player;
                        chain_damage.chain = true;

                        room->damage(chain_damage);

                        break;
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
            bool hit = !room->askForCard(effect.to, "jink", "slash-jink:" + slasher);
            room->slashResult(effect, hit);

            break;
        }

    case SlashHit:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            DamageStruct damage;
            damage.card = effect.slash;

            damage.damage = 1;
            if(effect.drank)
                damage.damage ++;

            damage.from = effect.from;
            damage.to = effect.to;
            damage.nature = effect.nature;

            room->damage(damage);

            effect.to->removeMark("qinggang");

            break;
        }

    case SlashMissed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            effect.to->removeMark("qinggang");

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
                    if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade")){
                        winner = "lord+loyalist";
                        if(player->getRole() == "renegade" && !alive_roles.contains("loyalist"))
                            room->setTag("RenegadeInFinalPK", true);
                    }
                    break;
                }

            default:
                break;
            }

            if(winner.isNull()){
                QString killer_name = data.toString();
                if(!killer_name.isEmpty()){
                    ServerPlayer *killer = room->findChild<ServerPlayer *>(killer_name);

                    if(player->getRole() == "rebel" && killer != player){
                        if(killer->isAlive())
                            killer->drawCards(3);
                    }else if(player->getRole() == "loyalist" && killer->getRole() == "lord"){
                        killer->throwAllEquips();
                        killer->throwAllHandCards();
                    }
                }

                setGameProcess(room);
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


