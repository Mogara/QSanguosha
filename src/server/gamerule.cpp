#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"
#include "standard.h"
#include "engine.h"
#include "settings.h"

#include <QTime>

GameRule::GameRule(QObject *parent)
    :TriggerSkill("game_rule")
{
    setParent(parent);

    events << GameStart << TurnStart << PhaseChange << CardUsed
            << CardEffected << HpRecover << HpLost << AskForPeachesDone
            << AskForPeaches << Death << Dying << GameOverJudge
            << SlashHit << SlashMissed << SlashEffected << SlashProceed
            << DamageDone << DamageComplete
            << StartJudge << FinishJudge << Pindian;
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
            QList<const DelayedTrick *> tricks = player->delayedTricks();
            while(!tricks.isEmpty() && player->isAlive()){
                const DelayedTrick *trick = tricks.takeLast();
                bool on_effect = room->cardEffect(trick, NULL, player);
                if(!on_effect)
                    trick->onNullified(player);
            }

            break;
        }
    case Player::Draw: {
            QVariant num = 2;
            if(room->getTag("FirstRound").toBool() && room->getMode() == "02_1v1"){
                room->setTag("FirstRound", false);
                num = 1;
            }

            room->getThread()->trigger(DrawNCards, player, num);
            int n = num.toInt();
            if(n > 0)
                player->drawCards(n, false);
            break;
        }

    case Player::Play: {
            player->clearHistory();

            while(player->isAlive()){
                CardUseStruct card_use;
                room->activate(player, card_use);
                if(card_use.isValid()){
                    room->useCard(card_use);
                }else
                    break;
            }
            break;
        }

    case Player::Discard:{
            int discard_num = player->getHandcardNum() - player->getMaxCards();
            if(player->hasFlag("jilei")){
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }

                if(jilei_cards.size() > player->getMaxCards()){
                    // show all his cards
                    room->showAllCards(player);

                    foreach(const Card *card, handcards.toSet() - jilei_cards){
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
            break;
        }

    case Player::NotActive:{
            if(player->hasFlag("drank")){
                LogMessage log;
                log.type = "#UnsetDrankEndOfTurn";
                log.from = player;
                room->sendLog(log);

                room->setPlayerFlag(player, "-drank");
            }

            player->clearFlags();

            return;
        }
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

    if(room->getTag("SkipGameRule").toBool()){
        room->removeTag("SkipGameRule");
        return false;
    }

    switch(event){
    case GameStart: {
        if(player->getGeneral()->getKingdom() == "god" && !Config.EnableBasara){
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

            if(room->getMode() == "02_1v1")
                room->setTag("FirstRound", true);

            break;
        }

    case TurnStart:{
            player = room->getCurrent();
            if(!player->faceUp())
                player->turnOver();
            else if(player->isAlive())
                player->play();

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

    case HpLost:{
            int lose = data.toInt();

            if(room->getCurrent()->hasSkill("jueqing"))
                return true;

            LogMessage log;
            log.type = "#LoseHp";
            log.from = player;
            log.arg = QString::number(lose);
            room->sendLog(log);

            room->setPlayerProperty(player, "hp", player->getHp() - lose);
            room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(-lose));

            if(player->getHp() <= 0)
                room->enterDying(player, NULL);

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

                room->useCard(use, false);
            }

            break;
        }

    case AskForPeachesDone:{
            if(player->getHp() <= 0){
                DyingStruct dying = data.value<DyingStruct>();
                room->killPlayer(player, dying.damage);
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
            if(room->getMode() == "02_1v1" && player->isDead()){
                QString new_general = player->tag["1v1ChangeGeneral"].toString();
                if(!new_general.isEmpty())
                    changeGeneral1v1(player);
            }

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
                        room->setPlayerProperty(chained_player, "chained", false);

                        LogMessage log;
                        log.type = "#IronChainDamage";
                        log.from = chained_player;
                        room->sendLog(log);

                        DamageStruct chain_damage = damage;
                        chain_damage.to = chained_player;
                        chain_damage.chain = true;

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

            QString slasher = effect.from->objectName();
            const Card *jink = room->askForCard(effect.to, "jink", "slash-jink:" + slasher);
            room->slashResult(effect, jink);

            break;
        }

    case SlashHit:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            DamageStruct damage;
            damage.card = effect.slash;

            damage.damage = 1;
            if(effect.drank)
                damage.damage ++;

            if(effect.to->hasSkill("jueqing") || effect.to->getGeneralName() == "zhangchunhua")
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

    case GameOverJudge:{
            if(room->getMode() == "02_1v1"){
                QStringList list = player->tag["1v1Arrange"].toStringList();

                if(!list.isEmpty())
                    return false;
            }

            QString winner = getWinner(player);
            if(!winner.isNull()){
                room->gameOver(winner);
                return true;
            }

            break;
        }

    case Death:{
            player->bury();

            if(room->getTag("SkipNormalDeathProcess").toBool())
                return false;

            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;
            if(killer){
                rewardAndPunish(killer, player);
            }

            setGameProcess(room);

            if(room->getMode() == "02_1v1"){
                QStringList list = player->tag["1v1Arrange"].toStringList();

                if(!list.isEmpty()){
                    player->tag["1v1ChangeGeneral"] = list.takeFirst();
                    player->tag["1v1Arrange"] = list;

                    DamageStar damage = data.value<DamageStar>();

                    if(damage == NULL){
                        changeGeneral1v1(player);
                        return false;
                    }
                }
            }

            break;
        }

    case StartJudge:{
            int card_id = room->drawCard();

            JudgeStar judge = data.value<JudgeStar>();
            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$InitialJudge";
            log.from = player;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);

            room->getThread()->delay();

            break;
        }

    case FinishJudge:{
            JudgeStar judge = data.value<JudgeStar>();
            room->throwCard(judge->card);

            LogMessage log;
            log.type = "$JudgeResult";
            log.from = player;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);

            room->getThread()->delay();

            break;
        }

    case Pindian:{
            PindianStar pindian = data.value<PindianStar>();

            LogMessage log;

            room->throwCard(pindian->from_card);
            log.type = "$PindianResult";
            log.from = pindian->from;
            log.card_str = pindian->from_card->getEffectIdString();
            room->sendLog(log);
            room->getThread()->delay();

            room->throwCard(pindian->to_card);
            log.type = "$PindianResult";
            log.from = pindian->to;
            log.card_str = pindian->to_card->getEffectIdString();
            room->sendLog(log);
            room->getThread()->delay();

            break;
        }

    default:
        ;
    }

    return false;
}

void GameRule::changeGeneral1v1(ServerPlayer *player) const{
    Room *room = player->getRoom();
    QString new_general = player->tag["1v1ChangeGeneral"].toString();
    player->tag.remove("1v1ChangeGeneral");
    room->transfigure(player, new_general, true, true);
    room->revivePlayer(player);

    if(player->getKingdom() != player->getGeneral()->getKingdom())
        room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());

    room->broadcastInvoke("revealGeneral",
                          QString("%1:%2").arg(player->objectName()).arg(new_general),
                          player);

    if(!player->faceUp())
        player->turnOver();

    if(player->isChained())
        room->setPlayerProperty(player, "chained", false);

    player->drawCards(4);
}

void GameRule::rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const{
    if(killer->isDead())
        return;

    if(killer->getRoom()->getMode() == "06_3v3"){
        killer->drawCards(3);
    }else{
        if(victim->getRole() == "rebel" && killer != victim){
            killer->drawCards(3);
        }else if(victim->getRole() == "loyalist" && killer->getRole() == "lord"){
            killer->throwAllEquips();
            killer->throwAllHandCards();
        }
    }
}

QString GameRule::getWinner(ServerPlayer *victim) const{
    Room *room = victim->getRoom();
    QString winner;

    if(room->getMode() == "06_3v3"){
        switch(victim->getRoleEnum()){
        case Player::Lord: winner = "renegade+rebel"; break;
        case Player::Renegade: winner = "lord+loyalist"; break;
        default:
            break;
        }
    }else{
        QStringList alive_roles = room->aliveRoles(victim);
        switch(victim->getRoleEnum()){
        case Player::Lord:{
                if(alive_roles.length() == 1 && alive_roles.first() == "renegade")
                    winner = room->getAlivePlayers().first()->objectName();
                else
                    winner = "rebel";
                break;
            }

        case Player::Rebel:
        case Player::Renegade:
            {
                if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade")){
                    winner = "lord+loyalist";
                    if(victim->getRole() == "renegade" && !alive_roles.contains("loyalist"))
                        room->setTag("RenegadeInFinalPK", true);
                }
                break;
            }

        default:
            break;
        }
    }

    return winner;
}


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

    case GameOverJudge:{
            const static QString evil = "lord+renegade";
            const static QString justice = "rebel+loyalist";

            QStringList alive_roles = room->aliveRoles(player);
            if(!alive_roles.contains("rebel") && !alive_roles.contains("loyalist")){
                room->gameOver(evil);
                return true;
            }

            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = NULL;
            if(damage)
                killer = damage->from;

            if(player->isLord()){
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

HulaoPassMode::HulaoPassMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("hulaopass_mode");

    events << HpChanged;
}

static int Transfiguration = 1;

bool HulaoPassMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart:{
            if(player->isLord()){
                if(setjmp(env) == Transfiguration){
                    player = room->getLord();
                    room->transfigure(player, "shenlvbu2", true, true);

                    QList<const Card *> tricks = player->getJudgingArea();
                    foreach(const Card *trick, tricks)
                        room->throwCard(trick);

                }else{
                    player->drawCards(8, false);
                }
            }else
                player->drawCards(player->getSeat() + 1, false);

            if(player->getGeneralName() == "zhangchunhua"){
                if(qrand() % 3 == 0)
                    room->killPlayer(player);
            }

            return false;
        }

    case CardUsed:{
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("Weapon") && player->askForSkillInvoke("weapon_recast", data)){
                room->throwCard(use.card);
                player->drawCards(1, false);
                return false;
            }

            break;
        }

    case HpChanged:{
            if(player->getGeneralName() == "shenlvbu1" && player->getHp() <= 4){
                longjmp(env, Transfiguration);
            }

            return false;
        }

    case Death:{
            if(player->isLord()){
                room->gameOver("rebel");
            }else{
                if(room->aliveRoles(player).length() == 1)
                    room->gameOver("lord");

                LogMessage log;
                log.type = "#Reforming";
                log.from = player;
                room->sendLog(log);

                player->bury();
                room->setPlayerProperty(player, "hp", 0);

                foreach(ServerPlayer *player, room->getOtherPlayers(room->getLord())){
                    if(player->askForSkillInvoke("draw_1v3"))
                        player->drawCards(1, false);
                }
            }

            return false;
        }

    case TurnStart:{
            if(player->isLord()){
                if(!player->faceUp())
                    player->turnOver();
                else
                    player->play();
            }else{
                if(player->isDead()){
                    if(player->getHp() + player->getHandcardNum() == 6){
                        LogMessage log;
                        log.type = "#ReformingRevive";
                        log.from = player;
                        room->sendLog(log);

                        room->revivePlayer(player);
                    }else if(player->isWounded()){
                        LogMessage log;
                        log.type = "#ReformingRecover";
                        log.from = player;
                        room->sendLog(log);

                        room->setPlayerProperty(player, "hp", player->getHp() + 1);
                    }else
                        player->drawCards(1, false);
                }else if(!player->faceUp())
                    player->turnOver();
                else
                    player->play();
            }

            return false;
        }

    default:
        break;
    }

    return GameRule::trigger(event, player, data);
}

BasaraMode::BasaraMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("basara_mode");

    events << CardLost << Predamaged;
}

int BasaraMode::getPriority() const
{
    return 5;
}

void BasaraMode::playerShowed(ServerPlayer *player) const{
    Room *room = player->getRoom();
    QStringList names = room->getTag(player->objectName()).toStringList();
    if(names.isEmpty())
        return;

    QString answer = room->askForChoice(player, "RevealGeneral", "yes+no");
    if(answer == "yes"){

        QString general_name = room->askForGeneral(player,names);

        generalShowed(player,general_name);
        playerShowed(player);
    }
}

void BasaraMode::generalShowed(ServerPlayer *player, QString general_name) const
{
    Room * room = player->getRoom();
    QStringList names = room->getTag(player->objectName()).toStringList();
    if(names.isEmpty())return;

    if(player->getGeneralName() == "anjiang")
    {
        QString transfigure_str = QString("%1:%2").arg(player->getGeneralName()).arg(general_name);
        player->invoke("transfigure", transfigure_str);
        room->setPlayerProperty(player,"general",general_name);
    }else
    {
        QString transfigure_str = QString("%1:%2").arg(player->getGeneral2Name()).arg(general_name);
        player->invoke("transfigure", transfigure_str);
        room->setPlayerProperty(player,"general2",general_name);
    }

        room->setPlayerProperty(player,"maxhp",player->getGeneralMaxHP());
        if(player->getHp() > player->getMaxHP())
            room->setPlayerProperty(player,"hp",player->getMaxHP());

        room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());

        names.removeOne(general_name);
        room->setTag(player->objectName(),QVariant::fromValue(names));

        LogMessage log;
        log.type = "#BasaraReveal";
        log.from = player;
        log.arg  = player->getGeneralName();
        log.arg2 = player->getGeneral2Name();

        room->sendLog(log);
        room->broadcastInvoke("playAudio","choose-item");
}

bool BasaraMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart:{
        if(player->isLord()){

            QSet<QString> selected_set;
            const Package *godpack = Sanguosha->findChild<const Package *>("god");
            foreach(const General *general, godpack->findChildren<const General *>())
                selected_set.insert(general->objectName());

            foreach(ServerPlayer *p, room->getAllPlayers()){
                QStringList choices = Sanguosha->getRandomGenerals(5, selected_set), selected;
                QList<const General *> choices_generals, selected_generals;
                foreach(QString n, choices)
                    choices_generals << Sanguosha->getGeneral(n);

                rechoose:
                QString first_name = room->askForGeneral(p, choices);
                const General *first = Sanguosha->getGeneral(first_name);

                foreach(const General *g, choices_generals)
                    if(g->getKingdom() == first->getKingdom() && g != first){
                        selected_generals << g;
                        selected << g->objectName();
                    }
                if(selected.isEmpty())
                    goto rechoose;

                QString second_name = room->askForGeneral(p, selected);
                selected_set.insert(first_name);
                selected_set.insert(second_name);

                QStringList roles;
                roles << first_name << second_name;
                QVariant player_roles;
                player_roles.setValue(roles);
                room->setTag(p->objectName(), player_roles);

                const General * gen = Sanguosha->getGeneral(first_name);
                foreach(const TriggerSkill *skill, gen->getTriggerSkills())
                    room->getThread()->addTriggerSkill(skill);

                gen = Sanguosha->getGeneral(second_name);
                foreach(const TriggerSkill *skill, gen->getTriggerSkills())
                    room->getThread()->addTriggerSkill(skill);
            }
        }

        break;
    }
    case CardLost:{
        if(player->getPhase() == Player::NotActive){
            CardMoveStar cms = data.value<CardMoveStar>();
            if(cms->from_place == Player::Hand)
                playerShowed(player);
            else if(cms->from_place == Player::Equip)
                playerShowed(player);
        }
        break;
    }

    case PhaseChange:{
        if(player->getPhase() == Player::Start)
            playerShowed(player);

        break;
    }
    case Predamaged:{
        playerShowed(player);
        break;
    }

    default:
        break;
    }

    return false;
}
