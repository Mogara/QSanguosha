#include "room.h"
#include "engine.h"
#include "settings.h"
#include "standard.h"
#include "ai.h"
#include "scenario.h"
#include "gamerule.h"
#include "scenerule.h"
#include "contestdb.h"
#include "banpair.h"
#include "roomthread3v3.h"
#include "roomthread1v1.h"
#include "server.h"
#include "generalselector.h"
#include "protocol.h"
#include "jsonutils.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

using namespace QSanProtocol;
using namespace QSanProtocol::Utils;

Room::Room(QObject *parent, const QString &mode)
    :QThread(parent), mode(mode), current(NULL), reply_player(NULL), pile1(Sanguosha->getRandomCards()),
      draw_pile(&pile1), discard_pile(&pile2),
      game_started(false), game_finished(false), L(NULL), thread(NULL),
      thread_3v3(NULL), sem(new QSemaphore), provided(NULL), has_provided(false), _virtual(false)
{
    player_count = Sanguosha->getPlayerCount(mode);
    scenario = Sanguosha->getScenario(mode);

    initCallbacks();
}

void Room::initCallbacks(){
    // init request response pair
    m_requestResponsePair[S_COMMAND_PLAY_CARD] = S_COMMAND_USE_CARD;
    m_requestResponsePair[S_COMMAND_NULLIFICATION] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_SHOW_CARD] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_ASK_PEACH] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_PINDIAN] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_EXCHANGE_CARD] = S_COMMAND_DISCARD_CARD;
    m_requestResponsePair[S_COMMAND_CHOOSE_DIRECTION] = S_COMMAND_MULTIPLE_CHOICE;
    // init callback table
    //callbacks["useCardCommand"] = &Room::commonCommand;
    //callbacks["invokeSkillCommand"] = &Room::commonCommand;
    //callbacks["replyNullificationCommand"] = &Room::commonCommand;
    //callbacks["chooseCardCommand"] = &Room::commonCommand;
    //callbacks["responseCardCommand"] = &Room::commonCommand;
    //callbacks["discardCardsCommand"] = &Room::commonCommand;
    //callbacks["chooseSuitCommand"] = &Room::commonCommand;
    //callbacks["chooseKingdomCommand"] = &Room::commonCommand;
    //callbacks["replyYijiCommand"] = &Room::commonCommand;
    //callbacks["replyGuanxingCommand"] = &Room::commonCommand;
    //callbacks["replyGongxinCommand"] = &Room::commonCommand;
    //callbacks["chooseAGCommand"] = &Room::commonCommand;
    //callbacks["choosePlayerCommand"] = &Room::commonCommand;
    //callbacks["chooseGeneralCommand"] = &Room::commonCommand;
    //callbacks["chooseCommand"] = &Room::chooseCommand;
    //callbacks["choose2Command"] = &Room::choose2Command;
    //callbacks["selectChoiceCommand"] = &Room::commonCommand;    
    //callbacks["assignRolesCommand"] = &Room::commonCommand;

    callbacks["toggleReadyCommand"] = &Room::toggleReadyCommand;
    callbacks["addRobotCommand"] = &Room::addRobotCommand;
    callbacks["fillRobotsCommand"] = &Room::fillRobotsCommand;
    

    callbacks["arrangeCommand"] = &Room::arrangeCommand;
    callbacks["takeGeneralCommand"] = &Room::takeGeneralCommand;
    callbacks["selectOrderCommand"] = &Room::selectOrderCommand;
    callbacks["selectRoleCommand"] = &Room::selectRoleCommand;

    callbacks["speakCommand"] = &Room::speakCommand;
    callbacks["trustCommand"] = &Room::trustCommand;
    callbacks["kickCommand"] = &Room::kickCommand;
    callbacks["surrenderCommand"] = &Room::surrenderCommand;

    callbacks["networkDelayTestCommand"] = &Room::networkDelayTestCommand;
}

QString Room::createLuaState(){
    QString error_msg;
    L = Sanguosha->createLuaState(true, error_msg);
    return error_msg;
}

ServerPlayer *Room::getCurrent() const{
    return current;
}

void Room::setCurrent(ServerPlayer *current){
    this->current = current;
}

int Room::alivePlayerCount() const{
    return alive_players.count();
}

QList<ServerPlayer *> Room::getOtherPlayers(ServerPlayer *except) const{
    int index = alive_players.indexOf(except);
    QList<ServerPlayer *> other_players;
    int i;

    if(index == -1){
        // the "except" is dead
        index = players.indexOf(except);
        for(i=index+1; i<players.length(); i++){
            if(players.at(i)->isAlive())
                other_players << players.at(i);
        }

        for(i=0; i<index; i++){
            if(players.at(i)->isAlive())
                other_players << players.at(i);
        }

        return other_players;
    }

    for(i=index+1; i<alive_players.length(); i++)
        other_players << alive_players.at(i);

    for(i=0; i<index; i++)
        other_players << alive_players.at(i);

    return other_players;
}

QList<ServerPlayer *> Room::getPlayers() const{
    return players ;
}

QList<ServerPlayer *> Room::getAllPlayers() const{
    if(current == NULL)
        return alive_players;

    int index = alive_players.indexOf(current);

    if(index == -1)
        return alive_players;

    QList<ServerPlayer *> all_players;
    int i;
    for(i=index; i<alive_players.length(); i++)
        all_players << alive_players.at(i);

    for(i=0; i<index; i++)
        all_players << alive_players.at(i);

    return all_players;
}

QList<ServerPlayer *> Room::getAlivePlayers() const{
    return alive_players;
}

void Room::output(const QString &message){
    emit room_message(message);
}

void Room::outputEventStack(){
    QString msg;

    foreach(EventTriplet triplet, *thread->getEventStack()){
        msg.prepend(triplet.toString());
    }

    output(msg);
}

void Room::enterDying(ServerPlayer *player, DamageStruct *reason){
    player->setFlags("dying");

    QString sos_filename;
    if(player->getGeneral()->isMale())
        sos_filename = "male-sos";
    else{
        int r = qrand() % 2 + 1;
        sos_filename = QString("female-sos%1").arg(r);
    }
    broadcastInvoke("playAudio", sos_filename);

    QList<ServerPlayer *> savers;
    ServerPlayer *current = getCurrent();
    if(current->hasSkill("wansha") && current->isAlive()){
        playSkillEffect("wansha");

        savers << current;

        LogMessage log;
        log.from = current;
        log.arg = "wansha";
        if(current != player){
            savers << player;
            log.type = "#WanshaTwo";
            log.to << player;
        }else{
            log.type = "#WanshaOne";
        }

        sendLog(log);

    }else
        savers = getAllPlayers();

    DyingStruct dying;
    dying.who = player;
    dying.damage = reason;
    dying.savers = savers;

    QVariant dying_data = QVariant::fromValue(dying);
    thread->trigger(Dying, player, dying_data);
}

void Room::revivePlayer(ServerPlayer *player){
    player->setAlive(true);
    broadcastProperty(player, "alive");

    alive_players.clear();
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
            alive_players << player;
    }

    int i;
    for(i=0; i<alive_players.length(); i++){
        alive_players.at(i)->setSeat(i+1);
        broadcastProperty(alive_players.at(i), "seat");
    }

    broadcastInvoke("revivePlayer", player->objectName());
    updateStateItem();
}

static bool CompareByRole(ServerPlayer *player1, ServerPlayer *player2){
    int role1 = player1->getRoleEnum();
    int role2 = player2->getRoleEnum();

    if(role1 != role2)
        return role1 < role2;
    else
        return player1->isAlive();
}

void Room::updateStateItem(){
    QList<ServerPlayer *> players = this->players;
    qSort(players.begin(), players.end(), CompareByRole);
    QString roles;
    foreach(ServerPlayer *p, players){
        QChar c = "ZCFN"[p->getRoleEnum()];
        if(p->isDead())
            c = c.toLower();

        roles.append(c);
    }

    broadcastInvoke("updateStateItem", roles);
}

void Room::killPlayer(ServerPlayer *victim, DamageStruct *reason){
    ServerPlayer *killer = reason ? reason->from : NULL;
    if(Config.ContestMode && killer){
        killer->addVictim(victim);
    }

    victim->setAlive(false);

    int index = alive_players.indexOf(victim);
    int i;
    for(i=index+1; i<alive_players.length(); i++){
        ServerPlayer *p = alive_players.at(i);
        p->setSeat(p->getSeat() - 1);
        broadcastProperty(p, "seat");
    }

    alive_players.removeOne(victim);

    LogMessage log;
    log.to << victim;
    log.arg = Config.EnableHegemony ? victim->getKingdom() : victim->getRole();
    log.from = killer;

    updateStateItem();

    if(killer){
        if(killer == victim)
            log.type = "#Suicide";
        else
            log.type = "#Murder";
    }else{
        log.type = "#Contingency";
    }

    sendLog(log);

    broadcastProperty(victim, "alive");

    QVariant data = QVariant::fromValue(reason);
    thread->trigger(GameOverJudge, victim, data);


    broadcastProperty(victim, "role");
    thread->delay(300);
    broadcastInvoke("killPlayer", victim->objectName());

    thread->trigger(Death, victim, data);
    victim->loseAllSkills();

    if(Config.EnableAI){
        bool expose_roles = true;
        foreach(ServerPlayer *player, alive_players){
            if(player->getState() != "robot" && player->getState() != "offline"){
                expose_roles = false;
                break;
            }
        }

        if(expose_roles){
            foreach(ServerPlayer *player, alive_players){
                if(Config.EnableHegemony){
                    QString role = player->getKingdom();
                    if(role == "god")
                        role = Sanguosha->getGeneral(getTag(player->objectName()).toStringList().at(0))->getKingdom();
                    role = BasaraMode::getMappedRole(role);
                    broadcast(QString("#%1 role %2").arg(player->objectName()).arg(role));
                }
                else
                    broadcastProperty(player, "role");
            }
        }
    }


}

void Room::judge(JudgeStruct &judge_struct){
    Q_ASSERT(judge_struct.who != NULL);

    JudgeStar judge_star = &judge_struct;

    QVariant data = QVariant::fromValue(judge_star);

    thread->trigger(StartJudge, judge_star->who, data);

    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        thread->trigger(AskForRetrial, player, data);
    }

    thread->trigger(FinishJudge, judge_star->who, data);
}

void Room::sendJudgeResult(const JudgeStar judge){
    QString who = judge->who->objectName();
    QString result = judge->isGood() ? "good" : "bad";
    broadcastInvoke("judgeResult", QString("%1:%2").arg(who).arg(result));
}

QList<int> Room::getNCards(int n, bool update_pile_number){
    QList<int> card_ids;
    int i;
    for(i=0; i<n; i++){
        card_ids << drawCard();
    }

    if(update_pile_number)
        broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    return card_ids;
}

QStringList Room::aliveRoles(ServerPlayer *except) const{
    QStringList roles;
    foreach(ServerPlayer *player, alive_players){
        if(player != except)
            roles << player->getRole();
    }

    return roles;
}

void Room::gameOver(const QString &winner){
    QStringList all_roles;
    foreach(ServerPlayer *player, players)
        all_roles << player->getRole();

    game_finished = true;

    if(Config.ContestMode){
        foreach(ServerPlayer *player, players){
            QString screen_name = player->screenName().toUtf8().toBase64();
            broadcastInvoke("setScreenName", QString("%1:%2").arg(player->objectName()).arg(screen_name));
        }

        ContestDB *db = ContestDB::GetInstance();
        db->saveResult(players, winner);
    }

    broadcastInvoke("gameOver", QString("%1:%2").arg(winner).arg(all_roles.join("+")));

    // save records
    if(Config.ContestMode){
        bool only_lord = Config.value("Contest/OnlySaveLordRecord", true).toBool();
        QString start_time = tag.value("StartTime").toDateTime().toString(ContestDB::TimeFormat);

        if(only_lord)
            getLord()->saveRecord(QString("records/%1.txt").arg(start_time));
        else{
            foreach(ServerPlayer *player, players){
                QString filename = QString("records/%1-%2.txt").arg(start_time).arg(player->getGeneralName());
                player->saveRecord(filename);
            }
        }

        ContestDB *db = ContestDB::GetInstance();
        if(!Config.value("Contest/Sender").toString().isEmpty())
            db->sendResult(this);
    }

    emit game_over(winner);

    if(mode.contains("_mini_"))
    {
        ServerPlayer * playerWinner = NULL;
        QStringList winners =winner.split("+");
        foreach(ServerPlayer * sp, players)
        {
            if(sp->getState() != "robot" &&
                    (winners.contains(sp->getRole()) ||
                     winners.contains(sp->objectName()))
                    )
            {
                playerWinner = sp;
                break;
            }
        }

        if(playerWinner)
        {

            QString id = Config.GameMode;
            id.replace("_mini_","");
            int stage = Config.value("MiniSceneStage",1).toInt();
            int current = id.toInt();
            if((stage == current) && stage<21)
            {
                Config.setValue("MiniSceneStage",current+1);
                id = QString::number(stage+1).rightJustified(2,'0');
                id.prepend("_mini_");
                Config.setValue("GameMode",id);
                Config.GameMode = id;
            }
        }
    }


    if(QThread::currentThread() == thread)
        thread->end();
    //else
        //@todo: release some kind of semaphore?
        //mutex->unlock();
}

void Room::slashEffect(const SlashEffectStruct &effect){
    effect.from->addMark("SlashCount");

    if(effect.from->getMark("SlashCount") > 1 && effect.from->hasSkill("paoxiao"))
        playSkillEffect("paoxiao");

    QVariant data = QVariant::fromValue(effect);

    if(effect.nature ==DamageStruct::Thunder)setEmotion(effect.from, "thunder_slash");
    else if(effect.nature == DamageStruct::Fire)setEmotion(effect.from, "fire_slash");
    else if(effect.slash->isBlack())setEmotion(effect.from, "slash_black");
    else if(effect.slash->isRed())setEmotion(effect.from, "slash_red");
    else setEmotion(effect.from, "killer");
    setEmotion(effect.to, "victim");

    setTag("LastSlashEffect", data);
    bool broken = thread->trigger(SlashEffect, effect.from, data);
    if(!broken)
        thread->trigger(SlashEffected, effect.to, data);
}

void Room::slashResult(const SlashEffectStruct &effect, const Card *jink){
    SlashEffectStruct result_effect = effect;
    result_effect.jink = jink;

    QVariant data = QVariant::fromValue(result_effect);

    if(jink == NULL)
        thread->trigger(SlashHit, effect.from, data);
    else{
        setEmotion(effect.to, "jink");
        thread->trigger(SlashMissed, effect.from, data);
    }
}

void Room::attachSkillToPlayer(ServerPlayer *player, const QString &skill_name){
    player->acquireSkill(skill_name);
    player->invoke("attachSkill", skill_name);
}

void Room::detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name){
    if(!player->hasSkill(skill_name))
        return;

    player->loseSkill(skill_name);
    broadcastInvoke("detachSkill",
                    QString("%1:%2").arg(player->objectName()).arg(skill_name));

    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill && skill->isVisible()){
        foreach(const Skill *skill, Sanguosha->getRelatedSkills(skill_name))
            detachSkillFromPlayer(player, skill->objectName());

        LogMessage log;
        log.type = "#LoseSkill";
        log.from = player;
        log.arg = skill_name;
        sendLog(log);
    }
}

bool Room::obtainable(const Card *card, ServerPlayer *player){
    if(card == NULL)
        return false;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        if(subcards.isEmpty())
            return false;
    }else{
        ServerPlayer *owner = getCardOwner(card->getId());
        Player::Place place = getCardPlace(card->getId());
        if(owner == player && place == Player::Hand)
            return false;
    }

    return true;
}

 bool Room::doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, 
                            bool broadcast, bool moveFocus)
 {
     QSanGeneralPacket packet(S_SERVER_REQUEST, command);
     packet.setMessageBody(arg);
     return executeCommand(player, &packet, broadcast, moveFocus);
 }
 
 bool Room::doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, time_t timeOut,
                            bool broadcast, bool moveFocus)
 {
     QSanGeneralPacket packet(S_SERVER_REQUEST, command);
     packet.setMessageBody(arg);
     return executeCommand(player, &packet, timeOut, broadcast, moveFocus);
 }
 
 bool Room::doNotify(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, bool broadcast)
 {
     QSanGeneralPacket packet(S_SERVER_NOTIFICATION, command);
     packet.setMessageBody(arg);
     if (broadcast)    
        broadcastInvoke(&packet);
    else
        player->invoke(&packet);
     return true;
 }

 bool Room::executeCommand(ServerPlayer* player, const QSanProtocol::QSanPacket* packet, bool broadcast, bool moveFocus)
{
    time_t timeOut = getCommandTimeout(packet->getCommandType());
    return executeCommand(player, packet, timeOut, broadcast, moveFocus);
}

bool Room::executeCommand(ServerPlayer* player, const QSanProtocol::QSanPacket* packet, time_t timeOut, bool broadcast, bool moveFocus)
{
    if (packet->getPacketType() == S_SERVER_REQUEST)
    {
        player->drainLock(ServerPlayer::SEMA_COMMAND);
        m_expectedReplyPlayer = player;
        CommandType command = packet->getCommandType();
        if (m_requestResponsePair.contains(command))
            m_expectedReplyCommand = m_requestResponsePair[command];
        else 
            m_expectedReplyCommand = command;
    }               

    if(moveFocus)
    {
        QSanGeneralPacket req(S_SERVER_NOTIFICATION, S_COMMAND_MOVE_FOCUS);
        req.setMessageBody(player->objectName().toAscii().constData());
        broadcastInvoke(&req, player);
    }   

    if (broadcast)    
        broadcastInvoke(packet);
    else
        player->invoke(packet);

    if (packet->getPacketType() == S_SERVER_REQUEST)
    {
        return getResult(timeOut);
    }
    else return true;
}

void Room::broadcastInvoke(const char *method, const QString &arg, ServerPlayer *except){
    broadcast(QString("%1 %2").arg(method).arg(arg), except);
}

void Room::broadcastInvoke(const QSanProtocol::QSanPacket* packet, ServerPlayer *except)
{
    broadcast(QString(packet->toString().c_str()), except);
}

bool Room::getResult(time_t timeOut){  
    QTime timer;
    timer.start();
    while(Config.OperationNoLimit || timeOut >= timer.elapsed())
    {
        if (Config.OperationNoLimit)
            m_expectedReplyPlayer->acquireLock(ServerPlayer::SEMA_COMMAND);
        else if (!m_expectedReplyPlayer->tryAcquireLock(ServerPlayer::SEMA_COMMAND, timeOut - timer.elapsed())) 
            return false;
    
        //@todo: ylin - release all locks when the client disconnects, perhaps writing it
        //into destructor of ServerPlayer
        //The lock might be acquired because the client disconnects
        if (!m_expectedReplyPlayer->isOnline())
            return false;

        if(game_finished)
        {
            thread->end();
            return false;
        }

        QSanGeneralPacket packet;
        bool success = packet.parse(m_clientResponseString.toAscii().constData());
        if (!success) return false;
        else if (packet.getPacketType() != S_CLIENT_REPLY 
            || packet.getCommandType() != m_expectedReplyCommand)
        {
            continue;
        }
        else
        {
            m_clientResponse = packet.getMessageBody();
            return true;
        }
    }
    return false;
}

bool Room::askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data){
    bool invoked;
    AI *ai = player->getAI();
    if(ai){
        invoked = ai->askForSkillInvoke(skill_name, data);
        if(invoked)
            thread->delay(Config.AIDelay);
    }else{

        Json::Value skillCommand;
        if(data.type() == QVariant::String)
            skillCommand = toJsonArray(skill_name, data.toString());
        else
            skillCommand = toJsonArray(skill_name, QString());

        if (!doRequest(player, S_COMMAND_INVOKE_SKILL, skillCommand))
        {            
            invoked = false;
        }
        else
        {
            if (m_clientResponse.isBool())
                invoked = m_clientResponse.asBool();
        }   
    }

    if(invoked)
    {        
        Json::Value msg = toJsonArray(skill_name, player->objectName());
        doNotify(player, S_COMMAND_INVOKE_SKILL, msg, true); 
    }

    QVariant decisionData = QVariant::fromValue("skillInvoke:"+skill_name+":"+(invoked ? "yes" : "no"));
    thread->trigger(ChoiceMade, player, decisionData);
    return invoked;
}

QString Room::askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices){
    AI *ai = player->getAI();
    QString answer;
    if(ai)
        answer = ai->askForChoice(skill_name, choices);
    else{
        bool success = doRequest(player, S_COMMAND_MULTIPLE_CHOICE, toJsonArray(skill_name, choices));
        if (!success || !m_clientResponse.isString())
        {            
            answer = ".";
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if(skill)
                return skill->getDefaultChoice(player);
        }
        else answer = toQString(m_clientResponse);
    }
    QVariant decisionData = QVariant::fromValue("skillChoice:"+skill_name+":"+answer);
    thread->trigger(ChoiceMade, player, decisionData);
    return answer;
}

void Room::obtainCard(ServerPlayer *target, const Card *card){
    if(card == NULL)
        return;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int card_id, subcards)
            obtainCard(target, card_id);
    }else
        obtainCard(target, card->getId());
}

void Room::obtainCard(ServerPlayer *target, int card_id){
    moveCardTo(Sanguosha->getCard(card_id), target, Player::Hand, true);
}

bool Room::isCanceled(const CardEffectStruct &effect){
    if(!effect.card->isCancelable(effect))
        return false;

    const TrickCard *trick = qobject_cast<const TrickCard *>(effect.card);
    if(trick){
        QVariant decisionData = QVariant::fromValue(effect.to);
        setTag("NullifyingTarget",decisionData);
        decisionData = QVariant::fromValue(effect.from);
        setTag("NullifyingSource",decisionData);
        decisionData = QVariant::fromValue(effect.card);
        setTag("NullifyingCard",decisionData);
        setTag("NullifyingTimes",0);
        return askForNullification(trick, effect.from, effect.to, true);
    }else
        return false;
}

bool Room::askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive){
    QString trick_name = trick->objectName();
    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        
        if(!player->hasNullification())
            continue;
        
        while(true)
        {
            AI *ai = player->getAI();
            const Card *card = NULL;
            if(ai){
                card = ai->askForNullification(trick, from, to, positive);
                if(card)
                    thread->delay(Config.AIDelay);
            }else{
                //@todo: does positive mean the card is not "nullifiction"? if so, chop the positive arg in
                // ai->askForNullification and change the trick_name instead.
                if(!positive) trick_name = "nullification";
                Json::Value ask_str = toJsonArray(trick_name, from ? from->objectName() : ".", to->objectName());
                    //ask_str = QString("%1:%2->%3").arg(trick_name)
                            //.arg(from ? from->objectName() : ".")
                            //.arg(to->objectName());                
                    //ask_str = QString("nullification:.->%1").arg(to->objectName());

                if(doRequest(player, S_COMMAND_NULLIFICATION, ask_str, false, false)
                    && m_clientResponse.isString())
                    card = Card::Parse(toQString(m_clientResponse));
            }

            if(card == NULL) break;

            bool continuable = false;
            card = card->validateInResposing(player, &continuable);
            if(card){
                CardUseStruct use;
                use.card = card;
                use.from = player;
                useCard(use);

                LogMessage log;
                log.type = "#NullificationDetails";
                log.from = from;
                log.to << to;
                log.arg = trick_name;
                sendLog(log);

                broadcastInvoke("animate", QString("nullification:%1:%2")
                                .arg(player->objectName()).arg(to->objectName()));

                QVariant decisionData = QVariant::fromValue("Nullification:"+QString(trick->metaObject()->className())+":"+to->objectName()+":"+(positive?"true":"false"));
                thread->trigger(ChoiceMade, player, decisionData);
                setTag("NullifyingTimes",getTag("NullifyingTimes").toInt()+1);

                return !askForNullification(trick, from, to, !positive);
            }else if(continuable)
                continue;
            break;
        }
    }

    return false;
}

int Room::askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason){
    
    //@todo: whoever wrote this had better but a explantory comment here
    if(!who->hasFlag("dongchaee") && who != player){
        if(flags == "h" || (flags == "he" && !who->hasEquip()))
            return who->getRandomHandCardId();
    }

    int card_id;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_id = ai->askForCardChosen(who, flags, reason);
    }else{
        bool success = doRequest(player, S_COMMAND_CHOOSE_CARD, toJsonArray(who->objectName(), flags, reason));
        //@todo: check if the card returned is valid
        if (!success || !m_clientResponse.isInt())
        {
            // randomly choose a card
            QList<const Card *> cards = who->getCards(flags);
            int r = qrand() % cards.length();
            return cards.at(r)->getId();
        }
        card_id = m_clientResponse.asInt();
    }

    if(card_id == -1)
        card_id = who->getRandomHandCardId();

    QVariant decisionData = QVariant::fromValue("cardChosen:"+reason+":"+QString::number(card_id));
    thread->trigger(ChoiceMade, player, decisionData);
    return card_id;
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt, const QVariant &data){
    const Card *card = NULL;

    QVariant asked = pattern;
    thread->trigger(CardAsked, player, asked);
    if(has_provided){
        card = provided;
        provided = NULL;
        has_provided = false;
    }else if(pattern.startsWith("@") || !player->isNude()){
        AI *ai = player->getAI();
        if(ai){
            card = ai->askForCard(pattern, prompt, data);
            if(card)
                thread->delay(Config.AIDelay);
        }else{            
            bool success = doRequest(player, S_COMMAND_RESPONSE_CARD, toJsonArray(pattern, prompt));
            if (success && !m_clientResponse.isNull())
                card = Card::Parse(toQString(m_clientResponse));
        }
    }

    if(card == NULL)
    {
        QVariant decisionData = QVariant::fromValue("cardResponsed:"+pattern+":"+prompt+":_"+"nil"+"_");
        thread->trigger(ChoiceMade, player, decisionData);
        return NULL;
    }

    bool continuable = false;
    CardUseStruct card_use;
    card_use.card = card;
    card_use.from = player;
    card = card->validateInResposing(player, &continuable);

    if(card){
        if(card->getTypeId() != Card::Skill){
            const CardPattern *card_pattern = Sanguosha->getPattern(pattern);
            if(card_pattern == NULL || card_pattern->willThrow())
                throwCard(card);
        }else if(card->willThrow())
            throwCard(card);

        QVariant decisionData = QVariant::fromValue("cardResponsed:"+pattern+":"+prompt+":_"+card->toString()+"_");
        thread->trigger(ChoiceMade, player, decisionData);

        CardStar card_ptr = card;
        QVariant card_star = QVariant::fromValue(card_ptr);

        if(!card->inherits("DummyCard") && !pattern.startsWith(".")){
            LogMessage log;
            log.card_str = card->toString();
            log.from = player;
            log.type = QString("#%1").arg(card->metaObject()->className());
            sendLog(log);

            player->playCardEffect(card);


            thread->trigger(CardResponsed, player, card_star);
        }else{
            thread->trigger(CardDiscarded, player, card_star);
        }

    }else if(continuable)
        return askForCard(player, pattern, prompt);

    return card;
}

bool Room::askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt){    
    CardUseStruct card_use;
    bool isCardUsed = false;
    AI *ai = player->getAI();
    if(ai){
        //@todo: update ai interface to use the new protocol
        QString answer = ai->askForUseCard(pattern, prompt);
        if(answer != ".")
        {            
            isCardUsed = true;
            card_use.from = player;
            card_use.parse(answer, this);
            thread->delay(Config.AIDelay);
        }
    }else{
        if (doRequest(player, S_COMMAND_USE_CARD, toJsonArray(pattern, prompt)))
            isCardUsed = !m_clientResponse.isNull();
        if (isCardUsed)
        {
            card_use.tryParse(m_clientResponse, this);
            card_use.from = player;
        }
    }

    if(isCardUsed){        
        if(card_use.isValid()){
            QVariant decisionData = QVariant::fromValue(card_use);
            thread->trigger(ChoiceMade, player, decisionData);
            useCard(card_use);
            return true;
        }
    }else{
        QVariant decisionData = QVariant::fromValue("askForUseCard:"+pattern+":"+prompt+":nil");
        thread->trigger(ChoiceMade, player, decisionData);
    }

    return false;
}

int Room::askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable, const QString &reason){
    
    Q_ASSERT(card_ids.length()>0);
    
    if(card_ids.length() == 1 && !refusable)
        return card_ids.first();

    int card_id;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_id = ai->askForAG(card_ids, refusable, reason);
    }else{
        
        player->invoke("disableAG", "false");        
        
        bool success = doRequest(player, S_COMMAND_AMAZING_GRACE, refusable);
        if (!success || !m_clientResponse.asInt() || !card_ids.contains(m_clientResponse.asInt()))
            card_id = card_ids.first();
        else card_id = m_clientResponse.asInt();
    }   

    QVariant decisionData = QVariant::fromValue("AGChosen:"+reason+":"+QString::number(card_id));
    thread->trigger(ChoiceMade, player, decisionData);

    return card_id;
}

const Card *Room::askForCardShow(ServerPlayer *player, ServerPlayer *requestor, const QString &reason){
    
    if(player->getHandcardNum() == 1){
        return player->getHandcards().first();
    }

    const Card *card = NULL;

    AI *ai = player->getAI();
    if(ai)
        card = ai->askForCardShow(requestor, reason);
    else{
        bool success = doRequest(player, S_COMMAND_SHOW_CARD, toJsonString(requestor->getGeneralName()));        
                        
        if (success && m_clientResponse.isString())
        {
            card = Card::Parse(toQString(m_clientResponse));
        }

        if (card == NULL)
            card = player->getRandomHandCard();        
    }

    QVariant decisionData = QVariant::fromValue("cardShow:" + reason + ":_" + card->toString() + "_");
    thread->trigger(ChoiceMade, player, decisionData);
    return card;
}

const Card *Room::askForSinglePeach(ServerPlayer *player, ServerPlayer *dying){

    //@todo: put this into AI!!!!!!!!!!!!!!!!!
    if(player->isKongcheng()){
        // jijiu special case
        if(player->hasSkill("jijiu") && player->getPhase() == Player::NotActive){
            bool has_red = false;
            foreach(const Card *equip, player->getEquips()){
                if(equip->isRed()){
                    has_red = true;
                    break;
                }
            }
            if(!has_red) return NULL;
        }else if(player->hasSkill("jiushi")){
            if(!player->faceUp())
                return NULL;
        }else if(player->hasSkill("longhun")){
            bool has_heart = false;
            foreach(const Card *equip, player->getEquips()){
                if(equip->getSuit() == Card::Heart){
                    has_heart = true;
                    break;
                }
            }
            if(!has_heart) return NULL;
        }else return NULL;
    }

    const Card * card;
    bool continuable = false;

    AI *ai = player->getAI();
    if(ai)
        card= ai->askForSinglePeach(dying);
    else{
        int peaches = 1 - dying->getHp();
        Json::Value arg(Json::arrayValue);
        arg[0] = toJsonString(dying->objectName());
        arg[1] = peaches;
        bool success = doRequest(player, S_COMMAND_ASK_PEACH, arg);
        
        if (!success || !m_clientResponse.isString()) return NULL;

        card = Card::Parse(toQString(m_clientResponse));

        if (card != NULL) 
            card = card->validateInResposing(player, &continuable);
    }
    if(card){
        QVariant decisionData = QVariant::fromValue("peach:"+
            QString("%1:%2:%3").arg(dying->objectName()).arg(1 - dying->getHp()).arg(card->toString()));
        thread->trigger(ChoiceMade, player, decisionData);
        return card;
    }else if(continuable)
        return askForSinglePeach(player, dying);
    else
        return NULL;
}

void Room::setPlayerFlag(ServerPlayer *player, const QString &flag){
    player->setFlags(flag);
    broadcast(QString("#%1 flags %2").arg(player->objectName()).arg(flag));
}

void Room::setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value){
    player->setProperty(property_name, value);
    broadcastProperty(player, property_name);

    if(strcmp(property_name, "hp") == 0){
        thread->trigger(HpChanged, player);
    }
}

void Room::setPlayerMark(ServerPlayer *player, const QString &mark, int value){
    player->setMark(mark, value);
    broadcastInvoke("setMark", QString("%1.%2=%3").arg(player->objectName()).arg(mark).arg(value));
}

void Room::setPlayerCardLock(ServerPlayer *player, const QString &name){
    player->setCardLocked(name);
    player->invoke("cardLock", name);
}

void Room::setPlayerStatistics(ServerPlayer *player, const QString &property_name, const QVariant &value){
    StatisticsStruct *statistics = player->getStatistics();
    if(!statistics->setStatistics(property_name, value))
        return;

    player->setStatistics(statistics);
    QString prompt = property_name + ":";

    bool ok;
    int add = value.toInt(&ok);
    if(ok)
        prompt += QString::number(add);
    else
        prompt += value.toString();

    player->invoke("setStatistics", prompt);
}

ServerPlayer *Room::addSocket(ClientSocket *socket){
    ServerPlayer *player = new ServerPlayer(this);
    player->setSocket(socket);
    players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processRequest(QString)));

    return player;
}

bool Room::isFull() const{
    return players.length() == player_count;
}

bool Room::isFinished() const{
    return game_finished;
}

int Room::getLack() const{
    return player_count - players.length();
}

QString Room::getMode() const{
    return mode;
}

const Scenario *Room::getScenario() const{
    return scenario;
}

void Room::broadcast(const QString &message, ServerPlayer *except){
    foreach(ServerPlayer *player, players){
        if(player != except){
            player->unicast(message);
        }
    }
}

void Room::swapPile(){
    if(discard_pile->isEmpty()){
        // the standoff
        gameOver(".");
    }

    int times = tag.value("SwapPile", 0).toInt();
    tag.insert("SwapPile", ++times);
    if(times == 6)
        gameOver(".");
    if(mode == "04_1v3"){
        int limit = Config.BanPackages.contains("maneuvering") ? 3 : 2;
        if(times == limit)
            gameOver(".");
    }

    qSwap(draw_pile, discard_pile);

    broadcastInvoke("clearPile");
    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    qShuffle(*draw_pile);

    foreach(int card_id, *draw_pile){
        setCardMapping(card_id, NULL, Player::DrawPile);
    }
}

QList<int> Room::getDiscardPile(){
    return *discard_pile;
}

QList<int> Room::getDrawPile(){
    return *draw_pile;
}

ServerPlayer *Room::findPlayer(const QString &general_name, bool include_dead) const{
    const QList<ServerPlayer *> &list = include_dead ? players : alive_players;

    if(general_name.contains("+")){
        QStringList names = general_name.split("+");
        foreach(ServerPlayer *player, list){
            if(names.contains(player->getGeneralName()))
                return player;
        }

        return NULL;
    }

    foreach(ServerPlayer *player, list){
        if(player->getGeneralName() == general_name)
            return player;
    }

    return NULL;
}

QList<ServerPlayer *>Room::findPlayersBySkillName(const QString &skill_name, bool include_dead) const{
    QList<ServerPlayer *> list;
    foreach(ServerPlayer *player, include_dead ? players : alive_players){
        if(player->hasSkill(skill_name))
            list << player;
    }
    return list;
}

ServerPlayer *Room::findPlayerBySkillName(const QString &skill_name, bool include_dead) const{
    const QList<ServerPlayer *> &list = include_dead ? players : alive_players;
    foreach(ServerPlayer *player, list){
        if(player->hasSkill(skill_name))
            return player;
    }

    return NULL;
}

void Room::installEquip(ServerPlayer *player, const QString &equip_name){
    if(player == NULL)
        return;

    int card_id = getCardFromPile(equip_name);
    if(card_id == -1)
        return;

    moveCardTo(Sanguosha->getCard(card_id), player, Player::Equip, true);

    thread->delay(800);
}

void Room::resetAI(ServerPlayer *player){
    AI *smart_ai = player->getSmartAI();
    if(smart_ai){
        ais.removeOne(smart_ai);
        smart_ai->deleteLater();
        player->setAI(cloneAI(player));
    }
}

void Room::transfigure(ServerPlayer *player, const QString &new_general, bool full_state, bool invoke_start, const QString &old_general){
    LogMessage log;
    log.type = "#Transfigure";
    log.from = player;
    log.arg = new_general;
    sendLog(log);

    QString transfigure_str = QString("%1:%2").arg(player->getGeneralName()).arg(new_general);
    player->invoke("transfigure", transfigure_str);

    if(Config.Enable2ndGeneral && !old_general.isEmpty() && player->getGeneral2Name() == old_general){
        setPlayerProperty(player, "general2", new_general);
        broadcastProperty(player, "general2");
    }
    else{
        setPlayerProperty(player, "general", new_general);
        broadcastProperty(player, "general");
    }
    thread->addPlayerSkills(player, invoke_start);

    player->setMaxHP(player->getGeneralMaxHP());
    broadcastProperty(player, "maxhp");

    if(full_state)
        player->setHp(player->getMaxHP());
    broadcastProperty(player, "hp");

    resetAI(player);
}

lua_State *Room::getLuaState() const{
    return L;
}

void Room::setFixedDistance(Player *from, const Player *to, int distance){
    QString a = from->objectName();
    QString b = to->objectName();
    QString set_str = QString("%1~%2=%3").arg(a).arg(b).arg(distance);
    from->setFixedDistance(to, distance);
    broadcastInvoke("setFixedDistance", set_str);
}

void Room::reverseFor3v3(const Card *card, ServerPlayer *player, QList<ServerPlayer *> &list){
    bool isClockwise = false;
    if(player->isOnline()){
        bool success = doRequest(player, S_COMMAND_CHOOSE_DIRECTION, Json::Value::null);
        //executeCommand(player, "askForDirection", "selectChoiceCommand", ".", ".");        
        if (!success || !m_clientResponse.isString())
        {
            isClockwise = (m_clientResponse.asString() == "cw");
        }        
    }else{
        //@todo: nice if this thing is encapsulated in AI
        const TrickCard *trick = qobject_cast<const TrickCard *>(card);
        if(trick->isAggressive()){
            if(AI::GetRelation3v3(player, player->getNextAlive()) == AI::Enemy)
                isClockwise = false;
            else
                isClockwise = true;
        }else{
            if(AI::GetRelation3v3(player, player->getNextAlive()) == AI::Friend)
                isClockwise = false;
            else
                isClockwise = true;
        }
    }

    LogMessage log;
    log.type = "#TrickDirection";
    log.from = player;
    log.arg = isClockwise ? "cw" : "ccw";
    log.arg2 = card->objectName();
    sendLog(log);

    if(isClockwise){
        QList<ServerPlayer *> new_list;

        while(!list.isEmpty())
            new_list << list.takeLast();

        if(card->inherits("GlobalEffect")){
            new_list.removeLast();
            new_list.prepend(player);
        }

        list = new_list;
    }
}

const ProhibitSkill *Room::isProhibited(const Player *from, const Player *to, const Card *card) const{
    return Sanguosha->isProhibited(from, to, card);
}

int Room::drawCard(){
    if(draw_pile->isEmpty())
        swapPile();

    return draw_pile->takeFirst();
}

const Card *Room::peek(){
    if(draw_pile->isEmpty())
        swapPile();

    int card_id = draw_pile->first();
    return Sanguosha->getCard(card_id);
}

void Room::prepareForStart(){
    if(scenario){
        QStringList generals, roles;
        scenario->assign(generals, roles);

        bool expose_roles = scenario->exposeRoles();
        int i;
        for(i=0; i<players.length(); i++){
            ServerPlayer *player = players.at(i);
            if(generals.length()>0)
            {
                player->setGeneralName(generals.at(i));
                broadcastProperty(player, "general");
            }
            player->setRole(roles.at(i));

            if(player->isLord())
                broadcastProperty(player, "role");

            if(expose_roles)
                broadcastProperty(player, "role");
            else
                player->sendProperty("role");
        }
    }else if(mode == "06_3v3"){
        return;
    }else if(mode == "02_1v1"){
        if(qrand() % 2 == 0)
            players.swap(0, 1);

        players.at(0)->setRole("lord");
        players.at(1)->setRole("renegade");

        int i;
        for(i=0; i<2; i++){
            broadcastProperty(players.at(i), "role");
        }

    }else if(mode == "04_1v3"){
        ServerPlayer *lord = players.at(qrand() % 4);
        int i = 0;
        for(i=0; i<4; i++){
            ServerPlayer *player = players.at(i);
            if(player == lord)
                player->setRole("lord");
            else
                player->setRole("rebel");
            broadcastProperty(player, "role");
        }
    }else if(Config.value("FreeAssign", false).toBool()){
        ServerPlayer *owner = getOwner();
        if(owner && owner->isOnline()){            
            bool success = doRequest(owner, S_COMMAND_CHOOSE_ROLE, Json::Value::null);
            //executeCommand(owner, "askForAssign", "assignRolesCommand", ".", ".");
            if(!success || !m_clientResponse.isArray() || m_clientResponse.size() != 2)
                assignRoles();
            else if(Config.FreeAssignSelf){                
                QString name = toQString(m_clientResponse[0][0]);
                QString role = toQString(m_clientResponse[1][0]);
                ServerPlayer *player_self = findChild<ServerPlayer *>(name);
                setPlayerProperty(player_self, "role", role);
                if(role == "lord")
                    broadcastProperty(player_self, "role", "lord");

                QList<ServerPlayer *> all_players = players;
                all_players.removeOne(player_self);
                int n = all_players.count(), i;
                QStringList roles = Sanguosha->getRoleList(mode);
                roles.removeOne(role);
                qShuffle(roles);

                for(i = 0; i < n; i++){
                    ServerPlayer *player = all_players[i];
                    QString role = roles.at(i);

                    player->setRole(role);
                    if(role == "lord")
                        broadcastProperty(player, "role", "lord");
                    else
                        player->sendProperty("role");
                }
            }
            else{                
                for(unsigned int i = 0; i < m_clientResponse[0].size(); i++){                    
                    QString name = toQString(m_clientResponse[0][i]);
                    QString role = toQString(m_clientResponse[1][i]);

                    ServerPlayer *player = findChild<ServerPlayer *>(name);
                    setPlayerProperty(player, "role", role);

                    players.swap(i, players.indexOf(player));
                }
            }
        }else
            assignRoles();
    }else
        assignRoles();

    adjustSeats();
}

void Room::reportDisconnection(){
    ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());

    if(player == NULL)
        return;

    // send disconnection message to server log
    emit room_message(player->reportHeader() + tr("disconnected"));

    // the 4 kinds of circumstances
    // 1. Just connected, with no object name : just remove it from player list
    // 2. Connected, with an object name : remove it, tell other clients and decrease signup_count
    // 3. Game is not started, but role is assigned, give it the default general(general2) and others same with fourth case
    // 4. Game is started, do not remove it just set its state as offline
    // all above should set its socket to NULL

    player->setSocket(NULL);

    if(player->objectName().isEmpty()){
        // first case
        player->setParent(NULL);
        players.removeOne(player);
    }else if(player->getRole().isEmpty()){
        // second case
        if(players.length() < player_count){
            player->setParent(NULL);
            players.removeOne(player);

            if(player->getState() != "robot"){
                QString screen_name = Config.ContestMode ? tr("Contestant") : player->screenName();
                QString leaveStr = tr("<font color=#000000>Player <b>%1</b> left the game</font>").arg(screen_name);
                speakCommand(player, leaveStr.toUtf8().toBase64());
            }

            broadcastInvoke("removePlayer", player->objectName());
        }
    }else{
        if(!game_started){
            // third case
            if(!QRegExp("^\\d\\d_\\dv\\d$").exactMatch(mode)){                                
                player->releaseLock(ServerPlayer::SEMA_CHOOSE_GENERAL); 
                player->releaseLock(ServerPlayer::SEMA_CHOOSE_GENERAL2); 
            }
        }

        // fourth case
        setPlayerProperty(player, "state", "offline");

        bool someone_is_online = false;
        foreach(ServerPlayer *player, players){
            if(player->getState() == "online" || player->getState() == "trust"){
                someone_is_online = true;
                break;
            }
        }

        if(!someone_is_online){
            game_finished = true;
            return;
        }

        if(reply_player == player){
            reply_player = NULL;
            reply_func.clear();
            m_clientResponseString.clear();

            player->releaseLock(ServerPlayer::SEMA_COMMAND);
        }
    }

    if(player->isOwner()){
        foreach(ServerPlayer *p, players){
            if(p->getState() == "online"){
                p->setOwner(true);
                broadcastProperty(p, "owner");
                break;
            }
        }
    }
}

void Room::trustCommand(ServerPlayer *player, const QString &){
    if(player->getState() == "online"){
        player->setState("trust");

        if(reply_player == player){
            reply_player = NULL;
            reply_func.clear();
            m_clientResponseString.clear();

            player->releaseLock(ServerPlayer::SEMA_COMMAND);
        }
    }else
        player->setState("online");

    broadcastProperty(player, "state");
}

void Room::processRequest(const QString &request){
    QSanGeneralPacket packet;
    if (packet.parse(request.toAscii().constData()))
    {
        ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());        
        if (packet.getCommandType() == S_COMMAND_CHOOSE_GENERAL_1)
        {
            Json::Value generalName = packet.getMessageBody();
            if (!generalName.isString()) 
                chooseCommand(player, "");
            else
                chooseCommand(player, toQString(generalName));
        }
        else if (packet.getCommandType() == S_COMMAND_CHOOSE_GENERAL_2)
        {
            Json::Value generalName = packet.getMessageBody();
            if (!generalName.isString()) 
                choose2Command(player, "");
            else
                choose2Command(player, toQString(generalName));
        }
        else commonCommand(qobject_cast<ServerPlayer*>(sender()), request);
    }
    else
    {
        QStringList args = request.split(" ");
        QString command = args.first();
        ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());
        if(player == NULL)
            return;

        if(game_finished){
            player->invoke("warn", "GAME_OVER");
            return;
        }

        command.append("Command");
        Callback callback = callbacks.value(command, NULL);
        if(callback){
            if(callback == &Room::commonCommand){
                if(!reply_func.isEmpty() && reply_func != command){
                    // just report error message and do not block the game
                    emit room_message(tr("Reply function should be %1 instead of %2").arg(reply_func).arg(command));
                }

                if(reply_player && reply_player != player){
                    QString should_be = reply_player->objectName();
                    QString instead_of = player->objectName();

                    // just report error message and do not block the game
                    emit room_message(tr("Reply player should be %1 instead of %2").arg(should_be).arg(instead_of));
                }
            }

            (this->*callback)(player, args.at(1));

            #ifndef QT_NO_DEBUG
            // output client command only in debug version
            emit room_message(player->reportHeader() + request);
            #endif

        }else
            emit room_message(tr("%1: %2 is not invokable").arg(player->reportHeader()).arg(command));
    }
}

void Room::addRobotCommand(ServerPlayer *player, const QString &){
    if(player && !player->isOwner())
        return;

    if(isFull())
        return;

    int n = 0;
    foreach(ServerPlayer *player, players){
        if(player->getState() == "robot")
            n++;
    }

    ServerPlayer *robot = new ServerPlayer(this);
    robot->setState("robot");

    players << robot;

    const QString robot_name = tr("Computer %1").arg(QChar('A' + n));
    const QString robot_avatar = Sanguosha->getRandomGeneralName();
    signup(robot, robot_name, robot_avatar, true);

    QString greeting = tr("Hello, I'm a robot").toUtf8().toBase64();
    speakCommand(robot, greeting);

    broadcastProperty(robot, "state");
}

void Room::fillRobotsCommand(ServerPlayer *player, const QString &){
    int left = player_count - players.length();
    for(int i=0; i<left; i++){
        addRobotCommand(player, QString());
    }
}

ServerPlayer *Room::getOwner() const{
    foreach(ServerPlayer *player, players){
        if(player->isOwner())
            return player;
    }

    return NULL;
}

void Room::toggleReadyCommand(ServerPlayer *player, const QString &){
    if(game_started)
        return;

    setPlayerProperty(player, "ready", ! player->isReady());

    if(player->isReady() && isFull()){
        bool allReady = true;
        foreach(ServerPlayer *player, players){
            if(!player->isReady()){
                allReady = false;
                break;
            }
        }

        if(allReady){
            foreach(ServerPlayer *player, players)
                setPlayerProperty(player, "ready", false);

            start();
        }
    }
}

void Room::signup(ServerPlayer *player, const QString &screen_name, const QString &avatar, bool is_robot){
    player->setObjectName(generatePlayerName());
    player->setProperty("avatar", avatar);
    player->setScreenName(screen_name);

    if(Config.ContestMode)
        player->startRecord();

    if(!is_robot){
        player->sendProperty("objectName");

        ServerPlayer *owner = getOwner();
        if(owner == NULL){
            player->setOwner(true);
            broadcastProperty(player, "owner");
        }
    }

    // introduce the new joined player to existing players except himself
    player->introduceTo(NULL);

    if(!is_robot){
        QString greetingStr = tr("<font color=#EEB422>Player <b>%1</b> joined the game</font>")
                .arg(Config.ContestMode ? tr("Contestant") : screen_name);
        speakCommand(player, greetingStr.toUtf8().toBase64());
        player->startNetworkDelayTest();

        // introduce all existing player to the new joined
        foreach(ServerPlayer *p, players){
            if(p != player)
                p->introduceTo(player);
        }
    }else
        toggleReadyCommand(player, QString());
}

void Room::assignGeneralsForPlayers(const QList<ServerPlayer *> &to_assign){

    QSet<QString> existed;
    foreach(ServerPlayer *player, players){
        if(player->getGeneral())
            existed << player->getGeneralName();

        if(player->getGeneral2())
            existed << player->getGeneral2Name();
    }

    const int max_choice = (Config.EnableHegemony && Config.Enable2ndGeneral) ? 5
                                                                              : Config.value("MaxChoice", 5).toInt();
    const int total = Sanguosha->getGeneralCount();
    const int max_available = (total-existed.size()) / to_assign.length();
    const int choice_count = qMin(max_choice, max_available);

    QStringList choices = Sanguosha->getRandomGenerals(total-existed.size(), existed);

    if(Config.EnableHegemony)
    {
        if(to_assign.first()->getGeneral())
        {
            foreach(ServerPlayer *sp,players)
            {
                QStringList old_list = sp->getSelected();
                sp->clearSelected();
                QString choice;

                //keep legal generals
                foreach(QString name, old_list)
                {
                    if(Sanguosha->getGeneral(name)->getKingdom()
                            != sp->getGeneral()->getKingdom()
                            || sp->findReasonable(old_list,true)
                            == name)
                    {
                        sp->addToSelected(name);
                        old_list.removeOne(name);
                    }
                }

                //drop the rest and add new generals
                while(old_list.length())
                {
                    choice = sp->findReasonable(choices);
                    sp->addToSelected(choice);
                    old_list.pop_front();
                    choices.removeOne(choice);
                }

            }
            return;
        }
    }


    foreach(ServerPlayer *player, to_assign){
        player->clearSelected();

        for(int i=0; i<choice_count; i++){
            QString choice = player->findReasonable(choices);
            player->addToSelected(choice);
            choices.removeOne(choice);
        }
    }
}

time_t Room::getCommandTimeout(QSanProtocol::CommandType command)
{
    if (Config.OperationNoLimit) return UINT_MAX;
    else if (command == S_COMMAND_CHOOSE_GENERAL
             || command == S_COMMAND_CHOOSE_GENERAL_1
             || command == S_COMMAND_CHOOSE_GENERAL_2)
    {
        return (Config.S_CHOOSE_GENERAL_TIMEOUT + 5) * 1000;
    }
    else if (command == S_COMMAND_SKILL_GUANXING)
    {
        return (Config.S_GUANXING_TIMEOUT + 5) * 1000;
    }
    else
    {
        return Config.OperationTimeout * 2 * 1000;
    }
}

void Room::chooseGenerals(){

    // for lord.
    const int nonlord_prob = 5;
    if(!Config.EnableHegemony)
    {
        QStringList lord_list;
        ServerPlayer *the_lord = getLord();
        if(mode == "08same")
            lord_list = Sanguosha->getRandomGenerals(Config.value("MaxChoice", 5).toInt());
        else if(the_lord->getState() == "robot")
            if(qrand()%100 < nonlord_prob)
                lord_list = Sanguosha->getRandomGenerals(1);
            else
                lord_list = Sanguosha->getLords();
        else
            lord_list = Sanguosha->getRandomLords();
        QString general = askForGeneral(the_lord, lord_list);
        the_lord->setGeneralName(general);
        if (!Config.EnableBasara)
            broadcastProperty(the_lord, "general", general);

        if(mode == "08same"){
            foreach(ServerPlayer *p, players){
                if(!p->isLord())
                    p->setGeneralName(general);
            }

            Config.Enable2ndGeneral = false;
            return;
        }
    }
    QList<ServerPlayer *> to_assign = players;
    if(!Config.EnableHegemony)to_assign.removeOne(getLord());
    assignGeneralsForPlayers(to_assign);
    foreach(ServerPlayer *player, to_assign){
        askForGeneralAsync(player);
    }    
    QTime timer;
    time_t totalTime = getCommandTimeout(S_COMMAND_CHOOSE_GENERAL);
    time_t remainTime = totalTime;
    timer.start();
    foreach(ServerPlayer *player, to_assign){
        if (Config.OperationNoLimit)
            player->acquireLock(ServerPlayer::SEMA_CHOOSE_GENERAL);
        else
        {            
            if (remainTime < 0) remainTime = 0;
            player->tryAcquireLock(ServerPlayer::SEMA_CHOOSE_GENERAL, remainTime);
            remainTime = totalTime - timer.elapsed();
        }
        if(!player->getGeneral())
            chooseCommand(player, QString());
    }

    if(Config.Enable2ndGeneral){
        QList<ServerPlayer *> to_assign = players;
        assignGeneralsForPlayers(to_assign);
        foreach(ServerPlayer *player, to_assign){
            askForGeneralAsync(player);
        }       
        totalTime = getCommandTimeout(S_COMMAND_CHOOSE_GENERAL);
        remainTime = totalTime;
        timer.restart();
        foreach(ServerPlayer *player, to_assign){
            if (Config.OperationNoLimit)
                player->acquireLock(ServerPlayer::SEMA_CHOOSE_GENERAL2);
            else
            {                
                if (remainTime < 0) remainTime = 0;
                player->tryAcquireLock(ServerPlayer::SEMA_CHOOSE_GENERAL2, remainTime);
                remainTime = totalTime - timer.elapsed();
            }
            if(!player->getGeneral2())
                choose2Command(player, QString());        
        }
    }


    if(Config.EnableBasara)
    {
        foreach(ServerPlayer *player, players)
        {
            QStringList names;
            if(player->getGeneral())names.append(player->getGeneralName());
            if(player->getGeneral2() && Config.Enable2ndGeneral)names.append(player->getGeneral2Name());
            this->setTag(player->objectName(),QVariant::fromValue(names));
        }
    }
}

void Room::run(){
    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    prepareForStart();

    bool using_countdown = true;
    if(_virtual || !property("to_test").toString().isEmpty())
        using_countdown = false;

#ifndef QT_NO_DEBUG
    using_countdown = false;
#endif

    if(using_countdown){
        for(int i=Config.CountDownSeconds; i>=0; i--){
            broadcastInvoke("startInXs", QString::number(i));
            sleep(1);
        }
    }else
        broadcastInvoke("startInXs", "0");

    if(scenario && !scenario->generalSelection())
        startGame();
    else if(mode == "06_3v3"){
        thread_3v3 = new RoomThread3v3(this);
        thread_3v3->start();

        connect(thread_3v3, SIGNAL(finished()), this, SLOT(startGame()));
    }else if(mode == "02_1v1"){
        thread_1v1 = new RoomThread1v1(this);
        thread_1v1->start();

        connect(thread_1v1, SIGNAL(finished()), this, SLOT(startGame()));
    }else if(mode == "04_1v3"){
        ServerPlayer *lord = players.first();
        setPlayerProperty(lord, "general", "shenlvbu1");

        const Package *stdpack = Sanguosha->findChild<const Package *>("standard");
        const Package *windpack = Sanguosha->findChild<const Package *>("wind");

        QList<const General *> generals = stdpack->findChildren<const General *>();
        generals << windpack->findChildren<const General *>();

        QStringList names;
        foreach(const General *general, generals){
            names << general->objectName();
        }

        names.removeOne("yuji");

        foreach(ServerPlayer *player, players){

            //Ensure that the game starts with all player's mutex locked
            player->drainAllLocks();

            if(player == lord)
                continue;

            qShuffle(names);
            QStringList choices = names.mid(0, 3);
            QString name = askForGeneral(player, choices);

            setPlayerProperty(player, "general", name);
            names.removeOne(name);
        }

        startGame();
    }else{
        chooseGenerals();
        startGame();
    }
}

void Room::assignRoles(){
    int n = players.count(), i;

    QStringList roles = Sanguosha->getRoleList(mode);
    qShuffle(roles);

    for(i = 0; i < n; i++){
        ServerPlayer *player = players[i];
        QString role = roles.at(i);

        player->setRole(role);
        if(role == "lord")
            broadcastProperty(player, "role", "lord");
        else
            player->sendProperty("role");
    }
}

void Room::swapSeat(ServerPlayer *a, ServerPlayer *b){
    int seat1 = players.indexOf(a);
    int seat2 = players.indexOf(b);

    players.swap(seat1, seat2);

    QStringList player_circle;
    foreach(ServerPlayer *player, players)
        player_circle << player->objectName();
    broadcastInvoke("arrangeSeats", player_circle.join("+"));

    alive_players.clear();
    int i;
    for(i=0; i<players.length(); i++){
        ServerPlayer *player = players.at(i);
        if(player->isAlive()){
            alive_players << player;
            player->setSeat(alive_players.length());
        }else{
            player->setSeat(0);
        }

        broadcastProperty(player, "seat");

        player->setNext(players.at((i+1) % players.length()));
    }
}

void Room::adjustSeats(){
    int i;
    for(i=0; i<players.length(); i++){
        if(players.at(i)->getRoleEnum() == Player::Lord){
            players.swap(0, i);
            break;
        }
    }

    for(i=0; i<players.length(); i++)
        players.at(i)->setSeat(i+1);

    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach(ServerPlayer *player, players)
        player_circle << player->objectName();

    broadcastInvoke("arrangeSeats", player_circle.join("+"));
}

int Room::getCardFromPile(const QString &card_pattern){
    if(draw_pile->isEmpty())
        swapPile();

    if(card_pattern.startsWith("@")){
        if(card_pattern == "@duanliang"){
            foreach(int card_id, *draw_pile){
                const Card *card = Sanguosha->getCard(card_id);
                if(card->isBlack() && (card->inherits("BasicCard") || card->inherits("EquipCard")))
                    return card_id;
            }
        }
    }else{
        QString card_name = card_pattern;
        foreach(int card_id, *draw_pile){
            const Card *card = Sanguosha->getCard(card_id);
            if(card->objectName() == card_name)
                return card_id;
        }
    }

    return -1;
}

void Room::choose2Command(ServerPlayer *player, const QString &general_name){
    if (player->getGeneral2()) return;

    const General *general = Sanguosha->getGeneral(general_name);
    if(general == NULL){
        if(Config.EnableHegemony)
        {
            foreach(QString name,player->getSelected())
            {
                if(name == player->getGeneralName())continue;
                if(Sanguosha->getGeneral(name)->getKingdom()
                        == player->getGeneral()->getKingdom())
                    general = Sanguosha->getGeneral(name);
            }
        }else
        {
            GeneralSelector *selector = GeneralSelector::GetInstance();
            QString choice = selector->selectSecond(player, player->getSelected());
            general = Sanguosha->getGeneral(choice);
        }
    }

    player->setGeneral2Name(general->objectName());
    player->sendProperty("general2");

    player->releaseLock(ServerPlayer::SEMA_CHOOSE_GENERAL2);
}

//@todo: verify general name is contained in player->getSelected()!!!!!!
void Room::chooseCommand(ServerPlayer *player, const QString &general_name){
    
    if (player->getGeneral()) return;
    
    const General *general = Sanguosha->getGeneral(general_name);
    if(general == NULL){
        if(Config.EnableHegemony && Config.Enable2ndGeneral)
        {
            foreach(QString name, player->getSelected())
            {
                foreach(QString other,player->getSelected())
                {
                    if(name == other) continue;
                    if(Sanguosha->getGeneral(name)->getKingdom()
                            == Sanguosha->getGeneral(other)->getKingdom())
                        general = Sanguosha->getGeneral(name);
                }
            }
        }else{
            GeneralSelector *selector = GeneralSelector::GetInstance();
            QString choice = selector->selectFirst(player, player->getSelected());
            general = Sanguosha->getGeneral(choice);
        }
    }

    player->setGeneral(general);
    player->sendProperty("general");

    player->releaseLock(ServerPlayer::SEMA_CHOOSE_GENERAL);
}

void Room::speakCommand(ServerPlayer *player, const QString &arg){
    broadcastInvoke("speak", QString("%1:%2").arg(player->objectName()).arg(arg));
}

void Room::commonCommand(ServerPlayer *player, const QString &arg){
    m_clientResponseString = arg;

    reply_player = NULL;
    reply_func.clear();

    player->releaseLock(ServerPlayer::SEMA_COMMAND);
}

void Room::useCard(const CardUseStruct &card_use, bool add_history){
    const Card *card = card_use.card;

    if(card_use.from->getPhase() == Player::Play && add_history){
        QString key;
        if(card->inherits("LuaSkillCard"))
            key = "#" + card->objectName();
        else
            key = card->metaObject()->className();

        bool slash_record =
                key.contains("Slash") &&
                card_use.from->getSlashCount() > 0 &&
                card_use.from->hasWeapon("crossbow");

        if(!slash_record){
            card_use.from->addHistory(key);
            card_use.from->invoke("addHistory", key);
        }

        broadcastInvoke("addHistory","pushPile");
    }

    card = card_use.card->validate(&card_use);
    if(card == card_use.card)
        card_use.card->onUse(this, card_use);
    else if(card){
        CardUseStruct new_use = card_use;
        new_use.card = card;
        useCard(new_use);
    }

    /*
    if(card->isVirtualCard())
        delete card;
        */
}

void Room::loseHp(ServerPlayer *victim, int lose){
    QVariant data = lose;
    thread->trigger(HpLost, victim, data);
}

void Room::loseMaxHp(ServerPlayer *victim, int lose){
    int hp = victim->getHp();
    victim->setMaxHP(qMax(victim->getMaxHP() - lose, 0));

    broadcastProperty(victim, "maxhp");
    broadcastProperty(victim, "hp");

    LogMessage log;
    log.type = hp - victim->getHp() == 0 ? "#LoseMaxHp" : "#LostMaxHpPlus";
    log.from = victim;
    log.arg = QString::number(lose);
    log.arg2 = QString::number(hp - victim->getHp());
    sendLog(log);

    if(victim->getMaxHP() == 0)
        killPlayer(victim);
}

void Room::applyDamage(ServerPlayer *victim, const DamageStruct &damage){
    int new_hp = victim->getHp() - damage.damage;

    setPlayerProperty(victim, "hp", new_hp);
    QString change_str = QString("%1:%2").arg(victim->objectName()).arg(-damage.damage);
    switch(damage.nature){
    case DamageStruct::Fire: change_str.append("F"); break;
    case DamageStruct::Thunder: change_str.append("T"); break;
    default: break;
    }

    broadcastInvoke("hpChange", change_str);
}

void Room::recover(ServerPlayer *player, const RecoverStruct &recover, bool set_emotion){
    if(player->getLostHp() == 0 || player->isDead())
        return;

    QVariant data = QVariant::fromValue(recover);
    thread->trigger(HpRecover, player, data);

    if(set_emotion){
        setEmotion(player, "recover");
    }
}

bool Room::cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to){
    CardEffectStruct effect;

    effect.card = card;
    effect.from = from;
    effect.to = to;

    return cardEffect(effect);
}

bool Room::cardEffect(const CardEffectStruct &effect){
    if(effect.to->isDead())
        return false;

    QVariant data = QVariant::fromValue(effect);
    bool broken = false;
    if(effect.from)
        broken = thread->trigger(CardEffect, effect.from, data);

    if(broken)
        return false;

    return !thread->trigger(CardEffected, effect.to, data);
}

void Room::damage(const DamageStruct &damage_data){
    if(damage_data.to == NULL)
        return;

    if(damage_data.to->isDead())
        return;

    QVariant data = QVariant::fromValue(damage_data);

    if(!damage_data.chain && damage_data.from){
        // predamage
        if(thread->trigger(Predamage, damage_data.from, data))
            return;
    }

    // DamagedProceed
    bool prevent = thread->trigger(DamagedProceed, damage_data.to, data);
    if(prevent)
        return;

    // DamageProceed
    if(damage_data.from){
        if(thread->trigger(DamageProceed, damage_data.from, data))
            return;
    }


    // predamaged
    bool broken = thread->trigger(Predamaged, damage_data.to, data);
    if(broken)
        return;

    // damage done, should not cause damage process broken
    thread->trigger(DamageDone, damage_data.to, data);

    // damage
    if(damage_data.from){
        bool broken = thread->trigger(Damage, damage_data.from, data);
        if(broken)
            return;
    }

    // damaged
    broken = thread->trigger(Damaged, damage_data.to, data);
    if(broken)
        return;

    thread->trigger(DamageComplete, damage_data.to, data);
}

void Room::sendDamageLog(const DamageStruct &data){
    LogMessage log;

    if(data.from){
        log.type = "#Damage";
        log.from = data.from;
    }else{
        log.type = "#DamageNoSource";
    }

    log.to << data.to;
    log.arg = QString::number(data.damage);

    switch(data.nature){
    case DamageStruct::Normal: log.arg2 = "normal_nature"; break;
    case DamageStruct::Fire: log.arg2 = "fire_nature"; break;
    case DamageStruct::Thunder: log.arg2 = "thunder_nature"; break;
    }

    sendLog(log);
}

bool Room::hasWelfare(const ServerPlayer *player) const{
    if(mode == "06_3v3")
        return player->isLord() || player->getRole() == "renegade";
    else if(mode == "04_1v3")
        return false;
    else if(Config.EnableHegemony)
        return false;
    else
        return player->isLord() && player_count > 4;
}

ServerPlayer *Room::getFront(ServerPlayer *a, ServerPlayer *b) const{
    ServerPlayer *p;

    for(p=current; true; p=p->getNext()){
        if(p == a)
            return a;
        else if(p == b)
            return b;
    }

    return a;
}

void Room::reconnect(ServerPlayer *player, ClientSocket *socket){
    player->setSocket(socket);
    player->setState("online");

    marshal(player);

    broadcastProperty(player, "state");
}

void Room::marshal(ServerPlayer *player){
    player->sendProperty("objectName");
    player->sendProperty("role");
    player->unicast(".flags marshalling");

    foreach(ServerPlayer *p, players){
        if(p != player)
            p->introduceTo(player);
    }

    QStringList player_circle;
    foreach(ServerPlayer *player, players)
        player_circle << player->objectName();

    player->invoke("arrangeSeats", player_circle.join("+"));
    player->invoke("startInXs", "0");

    foreach(ServerPlayer *p, players){
        player->sendProperty("general", p);

        if(p->getGeneral2())
            player->sendProperty("general2", p);
    }

    player->invoke("startGame");

    foreach(ServerPlayer *p, players){
        p->marshal(player);
    }

    player->unicast(".flags -marshalling");
    player->invoke("setPileNumber", QString::number(draw_pile->length()));
}

void Room::startGame(){
    if(Config.ContestMode)
        tag.insert("StartTime", QDateTime::currentDateTime());

    QString to_test = property("to_test").toString();
    if(!to_test.isEmpty()){
        bool found = false;

        foreach(ServerPlayer *p, players){
            if(p->getGeneralName() == to_test){
                found = true;
                break;
            }
        }

        if(!found){
            int r = qrand() % players.length();
            players.at(r)->setGeneralName(to_test);
        }
    }

    int i;
    if(true){
        int start_index = 1;
        if(mode == "06_3v3" || mode == "02_1v1")
            start_index = 0;

        if(!Config.EnableBasara)for(i = start_index; i < players.count(); i++){
            broadcastProperty(players.at(i), "general");
        }

        if(mode == "02_1v1"){
            foreach(ServerPlayer *player, players){
                broadcastInvoke("revealGeneral",
                                QString("%1:%2").arg(player->objectName()).arg(player->getGeneralName()),
                                player);
            }
        }
    }

    if((Config.Enable2ndGeneral) && mode != "02_1v1" && mode != "06_3v3" && mode != "04_1v3" && !Config.EnableBasara){
        foreach(ServerPlayer *player, players)
            broadcastProperty(player, "general2");
    }

    alive_players = players;
    for(i=0; i<player_count-1; i++)
        players.at(i)->setNext(players.at(i+1));
    players.last()->setNext(players.first());

    foreach(ServerPlayer *player, players){
        player->setMaxHP(player->getGeneralMaxHP());
        player->setHp(player->getMaxHP());

        broadcastProperty(player, "maxhp");
        broadcastProperty(player, "hp");

        if(mode == "06_3v3")
            broadcastProperty(player, "role");

        // setup AI
        AI *ai = cloneAI(player);
        ais << ai;
        player->setAI(ai);
    }

    broadcastInvoke("startGame");
    game_started = true;

    Server *server = qobject_cast<Server *>(parent());
    foreach(ServerPlayer *player, players){
        if(player->getState() == "online")
            server->signupPlayer(player);
    }

    current = players.first();

    // initialize the place_map and owner_map;
    foreach(int card_id, *draw_pile){
        setCardMapping(card_id, NULL, Player::DrawPile);
    }

    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    thread = new RoomThread(this);
    connect(thread, SIGNAL(started()), this, SIGNAL(game_start()));

    GameRule *game_rule;
    if(mode == "04_1v3")
        game_rule = new HulaoPassMode(this);
    else if(Config.EnableScene)	//changjing
        game_rule = new SceneRule(this);	//changjing
    else
        game_rule = new GameRule(this);

    thread->constructTriggerTable(game_rule);
    if(Config.EnableBasara)thread->addTriggerSkill(new BasaraMode(this));

    if(scenario){
        const ScenarioRule *rule = scenario->getRule();
        if(rule)
            thread->addTriggerSkill(rule);
    }

    if(!_virtual)thread->start();
}

void Room::broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value){
    if(value.isNull()){
        QString real_value = player->property(property_name).toString();
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(real_value));
    }else
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(value));
}

void Room::drawCards(ServerPlayer *player, int n, const QString &reason){
    if(n <= 0)
        return;

    QList<int> card_ids;
    QStringList cards_str;

    int i;
    for(i=0; i<n; i++){
        int card_id = drawCard();
        card_ids << card_id;
        const Card *card = Sanguosha->getCard(card_id);
        card->setFlags(reason);

        QVariant data = QVariant::fromValue(card_id);
        if(thread->trigger(CardDrawing, player, data))
            continue;

        player->drawCard(card);

        cards_str << QString::number(card_id);

        // update place_map & owner_map
        setCardMapping(card_id, player, Player::Hand);
    }
    if(cards_str.isEmpty())
        return;

    player->invoke("drawCards", cards_str.join("+"));

    QString draw_str = QString("%1:%2").arg(player->objectName()).arg(n);

    QString dongchaee = tag.value("Dongchaee").toString();
    if(player->objectName() == dongchaee){
        QString dongchaer_name = tag.value("Dongchaer").toString();
        ServerPlayer *dongchaer = findChild<ServerPlayer *>(dongchaer_name);

        CardMoveStruct move;
        foreach(int card_id, card_ids){
            move.card_id = card_id;
            move.from = NULL;
            move.from_place = Player::DrawPile;
            move.to = player;
            move.to_place = Player::Hand;

            dongchaer->invoke("moveCard", move.toString());
        }

        foreach(ServerPlayer *p, players){
            if(p != player && p != dongchaer)
                p->invoke("drawNCards", draw_str);
        }
    }else
        broadcastInvoke("drawNCards", draw_str, player);

    QVariant data = QVariant::fromValue(n);
    thread->trigger(CardDrawnDone, player, data);
}

void Room::throwCard(const Card *card, ServerPlayer *who){
    if(card == NULL)
        return;

    if (who) {
        LogMessage log;
        log.type = "$DiscardCard";
        log.from = who;
        QList<int> to_discard;
        if(card->isVirtualCard())
            to_discard.append(card->getSubcards());
        else
            to_discard << card->getEffectiveId();

        foreach(int card_id, to_discard){
            if(log.card_str.isEmpty())
                log.card_str = QString::number(card_id);
            else
                log.card_str += "+" + QString::number(card_id);
        }
        sendLog(log);
    }

    moveCardTo(card, NULL, Player::DiscardedPile);

    if(who){
        CardStar card_ptr = card;
        QVariant data = QVariant::fromValue(card_ptr);
        thread->trigger(CardDiscarded, who, data);
    }
}

void Room::throwCard(int card_id, ServerPlayer *who){
    throwCard(Sanguosha->getCard(card_id), who);
}

RoomThread *Room::getThread() const{
    return thread;
}

void Room::moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open){
    QSet<ServerPlayer *> scope;

    if(!open){
        int eid = card->getEffectiveId();
        ServerPlayer *from = getCardOwner(eid);
        Player::Place from_place= getCardPlace(eid);

        scope.insert(from);
        scope.insert(to);

        QString dongchaee_name = tag.value("Dongchaee").toString();
        if(!dongchaee_name.isEmpty()){
            ServerPlayer *dongchaee = findChild<ServerPlayer *>(dongchaee_name);
            bool invoke_dongcha = false;
            if(dongchaee == from)
                invoke_dongcha = (from_place == Player::Hand);
            else if(dongchaee == to)
                invoke_dongcha = (place == Player::Hand);

            if(invoke_dongcha){
                QString dongchaer_name = tag.value("Dongchaer").toString();
                ServerPlayer *dongchaer = findChild<ServerPlayer *>(dongchaer_name);
                scope.insert(dongchaer);
            }
        }

        QString from_str = from->objectName();
        if(from_place == Player::Special)
            from_str.append("@special");

        QString to_str = to->objectName();
        if(place == Player::Special)
            to_str.append("@special");

        int n = card->isVirtualCard() ? card->subcardsLength() : 1;
        QString private_move = QString("%1:%2->%3")
                .arg(n)
                .arg(from_str)
                .arg(to_str);

        foreach(ServerPlayer *player, players){
            if(!scope.contains(player))
                player->invoke("moveNCards", private_move);
        }
    }

    CardMoveStruct move;
    move.to = to;
    move.to_place = place;
    move.open = open;

    ServerPlayer *from = NULL;
    QVariant data;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards){
            move.card_id = subcard;
            move.from = getCardOwner(subcard);
            move.from_place = getCardPlace(subcard);

            if(to){
                data = QVariant::fromValue(move);
                thread->trigger(CardMoving, move.to, data);
            }
            doMove(move, scope);

            if(move.from)
                from = move.from;
        }
    }else{
        move.card_id = card->getId();
        move.from = getCardOwner(move.card_id);
        move.from_place = getCardPlace(move.card_id);

        if(to){
            data = QVariant::fromValue(move);
            thread->trigger(CardMoving, move.to, data);
        }
        doMove(move, scope);

        if(move.from)
            from = move.from;
    }

    if(from)
        thread->trigger(CardLostDone, from);
    if(to)
        thread->trigger(CardGotDone, to);
}

void Room::doMove(const CardMoveStruct &move, const QSet<ServerPlayer *> &scope){
    // avoid useless operation
    if(move.from == move.to && move.from_place == move.to_place)
        return;

    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from){
        if(move.from_place == Player::Special){
            QString pile_name = move.from->getPileName(move.card_id);
            
            //@todo: if (pile_name.isEmpty());
            
            QString pile_str = QString("%1:%2-%3")
                    .arg(move.from->objectName()).arg(pile_name).arg(move.card_id);

            if(move.open)
                broadcastInvoke("pile", pile_str);
            else{
                foreach(ServerPlayer *p, scope)
                    p->invoke("pile", pile_str);
            }
        }

        move.from->removeCard(card, move.from_place);
    }else{
        switch(move.from_place){
        case Player::DiscardedPile: discard_pile->removeOne(move.card_id); break;
        case Player::DrawPile: draw_pile->removeOne(move.card_id); break;
        case Player::Special: table_cards.removeOne(move.card_id); break;
        default:
            break;
        }
    }

    if(move.to){
        move.to->addCard(card, move.to_place);

        if(move.to_place == Player::Special){
            QString pile_name = move.to->getPileName(move.card_id);
            
            //@todo: if (pile_name.isEmpty());
            
            QString pile_str = QString("%1:%2+%3")
                    .arg(move.to->objectName()).arg(pile_name).arg(move.card_id);

            if(move.open)
                broadcastInvoke("pile", pile_str);
            else{
                foreach(ServerPlayer *p, scope)
                    p->invoke("pile", pile_str);
            }
        }
    }else{
        switch(move.to_place){
        case Player::DiscardedPile: discard_pile->prepend(move.card_id); break;
        case Player::DrawPile: draw_pile->prepend(move.card_id); break;
        case Player::Special: table_cards.append(move.card_id); break;
        default:
            break;
        }
    }

    setCardMapping(move.card_id, move.to, move.to_place);

    QString move_str = move.toString();
    if(!move.open){
        foreach(ServerPlayer *player, scope){
            player->invoke("moveCard", move_str);
        }
    }else{
        broadcastInvoke("moveCard", move_str);
    }

    if(move.from){
        CardMoveStar move_star = &move;
        QVariant data = QVariant::fromValue(move_star);
        thread->trigger(CardLost, move.from, data);
    }
    if(move.to && move.to!=move.from){
        CardMoveStar move_star = &move;
        QVariant data = QVariant::fromValue(move_star);
        thread->trigger(CardGot, move.to, data);
    }
    Sanguosha->getCard(move.card_id)->onMove(move);
}

QString CardMoveStruct::toString() const{
    static QMap<Player::Place, QString> place2str;
    if(place2str.isEmpty()){
        place2str.insert(Player::Hand, "hand");
        place2str.insert(Player::Equip, "equip");
        place2str.insert(Player::Judging, "judging");
        place2str.insert(Player::Special, "special");
        place2str.insert(Player::DiscardedPile, "_");
        place2str.insert(Player::DrawPile, "=");
    }

    QString from_str = from ? from->objectName() : "_";
    QString to_str = to ? to->objectName() : "_";

    return QString("%1:%2@%3->%4@%5")
            .arg(card_id)
            .arg(from_str).arg(place2str.value(from_place, "_"))
            .arg(to_str).arg(place2str.value(to_place, "_"));
}

void Room::playSkillEffect(const QString &skill_name, int index){
    broadcastInvoke("playSkillEffect", QString("%1:%2").arg(skill_name).arg(index));
}

void Room::startTest(const QString &to_test){
    fillRobotsCommand(NULL, ".");
    setProperty("to_test", to_test);
}

void Room::acquireSkill(ServerPlayer *player, const Skill *skill, bool open){
    QString skill_name = skill->objectName();
    if(player->hasSkill(skill_name))
        return;

    player->acquireSkill(skill_name);

    if(skill->inherits("TriggerSkill")){
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        thread->addTriggerSkill(trigger_skill);
    }

    if(skill->isVisible()){
        if(open){
            QString acquire_str = QString("%1:%2").arg(player->objectName()).arg(skill_name);
            broadcastInvoke("acquireSkill", acquire_str);
        }

        foreach(const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)){
            if(!related_skill->isVisible())
                acquireSkill(player, related_skill);
        }
    }
}

void Room::acquireSkill(ServerPlayer *player, const QString &skill_name, bool open){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill)
        acquireSkill(player, skill, open);
}

void Room::setTag(const QString &key, const QVariant &value){
    tag.insert(key, value);
    if(scenario)
        scenario->onTagSet(this, key);
}

QVariant Room::getTag(const QString &key) const{
    return tag.value(key);
}

void Room::removeTag(const QString &key){
    tag.remove(key);
}

void Room::setEmotion(ServerPlayer *target, const QString &emotion){
    broadcastInvoke("setEmotion",
                    QString("%1:%2").arg(target->objectName()).arg(emotion.isEmpty() ? "." : emotion));
}

#include <QElapsedTimer>

void Room::activate(ServerPlayer *player, CardUseStruct &card_use){
    AI *ai = player->getAI();
    if(ai){
        QElapsedTimer timer;
        timer.start();

        card_use.from = player;
        ai->activate(card_use);

        qint64 diff = Config.AIDelay - timer.elapsed();
        if(diff > 0)
            thread->delay(diff);
    }else{           
        bool success = doRequest(player, S_COMMAND_PLAY_CARD, toJsonString(player->objectName()), true);
        if (!success || m_clientResponse.isNull()) return;

        if(m_clientResponseString.startsWith(":")){
            makeCheat(m_clientResponseString);
            if(player->isAlive())
                return activate(player, card_use);
            return;
        }       

        card_use.from = player;
        if (!card_use.tryParse(m_clientResponse, this) || !card_use.isValid()){
            emit room_message(tr("Card can not parse:\n %1").arg(m_clientResponseString));
            return;
        }
    }

    QVariant data = QVariant::fromValue(card_use);
    thread->trigger(ChoiceMade, player, data);
}

Card::Suit Room::askForSuit(ServerPlayer *player, const QString& reason){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForSuit(reason);

    bool success = doRequest(player, S_COMMAND_CHOOSE_SUIT, Json::Value::null);    

    Card::Suit suit = Card::AllSuits[qrand() % 4];
    if (success)
    {
        QString suitStr = toQString(m_clientResponse); 
        if(suitStr == "spade")
            suit = Card::Spade;
        else if(suitStr == "club")
            suit = Card::Club;
        else if(suitStr == "heart")
            suit = Card::Heart;
        else if (suitStr == "diamond")
            suit = Card::Diamond;
    } 
        
    return suit;
}

QString Room::askForKingdom(ServerPlayer *player){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForKingdom();

    bool success = doRequest(player, S_COMMAND_CHOOSE_KINGDOM, Json::Value::null);
    
    //@todo: check if the result is valid before return!!
    //@todo: make kingdom a enum or static const instead of variable QString
    if (success && m_clientResponse.isString())
    {
        QString kingdom = toQString(m_clientResponse.asCString());
        if (kingdom == "wei" || kingdom == "shu" || kingdom == "wu" || kingdom == "qun")
            return kingdom;
    }    
    return "wei";    
}

bool Room::askForDiscard(ServerPlayer *player, const QString &reason, int discard_num, bool optional, bool include_equip){
    AI *ai = player->getAI();
    QList<int> to_discard;
    if (ai) {
        to_discard = ai->askForDiscard(reason, discard_num, optional, include_equip);
    }else{
        Json::Value ask_str(Json::arrayValue);
        ask_str[0] = discard_num;
        ask_str[1] = optional;
        ask_str[2] = include_equip;
        bool success = doRequest(player, S_COMMAND_DISCARD_CARD, ask_str);
                  
        //@todo: also check if the player does have that card!!!
        if(!success || !m_clientResponse.isArray() || (int)m_clientResponse.size() != discard_num
            || !tryParse(m_clientResponse, to_discard))
        {
            if(optional) return false;
            // time is up, and the server choose the cards to discard
            to_discard = player->forceToDiscard(discard_num, include_equip);
        }            
    }

    if (to_discard.isEmpty()) return false;

    DummyCard *dummy_card = new DummyCard;
    foreach(int card_id, to_discard)
        dummy_card->addSubcard(card_id);

    throwCard(dummy_card, player);

    QVariant data;
    data = QString("%1:%2").arg("cardDiscard").arg(dummy_card->toString());
    thread->trigger(ChoiceMade, player, data);

    dummy_card->deleteLater();

    return true;
}

const Card *Room::askForExchange(ServerPlayer *player, const QString &reason, int discard_num){
    AI *ai = player->getAI();
    QList<int> to_exchange;
    if(ai){
        // share the same callback interface
        to_exchange = ai->askForDiscard(reason, discard_num, false, false);
    }else{
        bool success = doRequest(player, S_COMMAND_EXCHANGE_CARD, discard_num);
        //@todo: also check if the player does have that card!!!
        if(!success || !m_clientResponse.isArray() || (int)m_clientResponse.size() != discard_num
           || !tryParse(m_clientResponse, to_exchange))
        {
            to_exchange = player->forceToDiscard(discard_num, false);
        }       
                   
    }

    DummyCard *card = new DummyCard;
    foreach(int card_id, to_exchange)
        card->addSubcard(card_id);

    return card;
}

void Room::setCardMapping(int card_id, ServerPlayer *owner, Player::Place place){
    owner_map.insert(card_id, owner);
    place_map.insert(card_id, place);
}

ServerPlayer *Room::getCardOwner(int card_id) const{
    return owner_map.value(card_id);
}

Player::Place Room::getCardPlace(int card_id) const{
    return place_map.value(card_id);
}

ServerPlayer *Room::getLord() const{
    ServerPlayer *the_lord = players.first();
    if(the_lord->getRole() == "lord")
        return the_lord;

    foreach(ServerPlayer *player, players){
        if(player->getRole() == "lord")
            return player;
    }

    return NULL;
}

void Room::askForGuanxing(ServerPlayer *zhuge, const QList<int> &cards, bool up_only){
    QList<int> top_cards, bottom_cards;

    AI *ai = zhuge->getAI();
    if(ai){
        ai->askForGuanxing(cards, top_cards, bottom_cards, up_only);
    }else if(up_only && cards.length() == 1){
        top_cards = cards;
    }else{
        Json::Value guanxingArgs(Json::arrayValue);
        guanxingArgs[0] = toJsonIntArray(cards);        
        guanxingArgs[1] = up_only;
        bool success = doRequest(zhuge, S_COMMAND_SKILL_GUANXING, guanxingArgs);
        
        //@todo: sanity check if this logic is correct
        if(!success){
            // the method "askForGuanxing" without any arguments
            // means to clear all the guanxing items
            zhuge->invoke("doGuanxing");
            foreach(int card_id, cards)
                draw_pile->prepend(card_id);
            return;
        }
        if (m_clientResponse.isArray() && m_clientResponse.size() == 2)
        {
            success &= tryParse(m_clientResponse[0], top_cards);
            success &= tryParse(m_clientResponse[1], bottom_cards);
        }        
    }


    bool length_equal = top_cards.length() + bottom_cards.length() == cards.length();
    bool result_equal = top_cards.toSet() + bottom_cards.toSet() == cards.toSet();
    if(!length_equal || !result_equal){
        top_cards = cards;
        bottom_cards.clear();
    }

    LogMessage log;
    log.type = "#GuanxingResult";
    log.from = zhuge;
    log.arg = QString::number(top_cards.length());
    log.arg2 = QString::number(bottom_cards.length());
    sendLog(log);

    QListIterator<int> i(top_cards);
    i.toBack();
    while(i.hasPrevious())
        draw_pile->prepend(i.previous());

    i = bottom_cards;
    while(i.hasNext())
        draw_pile->append(i.next());
}

void Room::doGongxin(ServerPlayer *shenlumeng, ServerPlayer *target){    
    //@todo: this thing should be put in AI!!!!!!!!!!
    if(!shenlumeng->isOnline()){
        // throw the first card whose suit is Heart
        QList<const Card *> cards = target->getHandcards();
        foreach(const Card *card, cards){
            if(card->getSuit() == Card::Heart && !card->inherits("Shit")){
                showCard(target, card->getEffectiveId());
                thread->delay();
                throwCard(card, target);
                return;
            }
        }
        return;
    }

    Json::Value gongxinArgs(Json::arrayValue);    
    gongxinArgs[0] = toJsonString(target->objectName());
    gongxinArgs[1] = true;
    gongxinArgs[2] = toJsonIntArray(target->handCards());
    bool success = doRequest(shenlumeng, S_COMMAND_SKILL_GONGXIN, gongxinArgs);
    
    if (!success || !m_clientResponse.isInt() 
        || !target->handCards().contains(m_clientResponse.asInt()))
        return;

    int card_id = m_clientResponse.asInt();
    showCard(target, card_id);

    QString result = askForChoice(shenlumeng, "gongxin", "discard+put");
    if(result == "discard")
        throwCard(card_id, target);
    else
        moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DrawPile, true);    
}

const Card *Room::askForPindian(ServerPlayer *player, ServerPlayer *from, ServerPlayer *to, const QString &reason)
{
    if(player->getHandcardNum() == 1){
        return player->getHandcards().first();
    }

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        return ai->askForPindian(from, reason);
    }

    bool success = doRequest(player, S_COMMAND_PINDIAN, toJsonArray(from->objectName(), to->objectName()));
    
    //executeCommand(player, "askForPindian", "responseCardCommand", ask_str, ".");
    
    if(!success || !m_clientResponse.isString()){
        int card_id = player->getRandomHandCardId();
        return Sanguosha->getCard(card_id);
    }else{        
        const Card *card = Card::Parse(toQString(m_clientResponse));
        if(card->isVirtualCard()){
            const Card *real_card = Sanguosha->getCard(card->getEffectiveId());
            delete card;
            return real_card;
        }else
            return card;
    }
}

ServerPlayer *Room::askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const QString &skillName){
    if(targets.isEmpty())
        return NULL;
    else if(targets.length() == 1)
        return targets.first();

    AI *ai = player->getAI();
    ServerPlayer* choice;
    if(ai)
        choice = ai->askForPlayerChosen(targets, skillName);
    else{
        Json::Value req;
        req[0] = Json::Value(Json::arrayValue);
        req[1] = toJsonString(skillName);
        foreach(ServerPlayer *target, targets)
            req[0].append(toJsonString(target->objectName()));
        bool success = doRequest(player, S_COMMAND_CHOOSE_PLAYER, req);
        
        //executeCommand(player, "askForPlayerChosen", "choosePlayerCommand", ask_str, ".");
        choice = NULL;
        if (success && m_clientResponse.isString())
        {
            choice = findChild<ServerPlayer *>(m_clientResponse.asCString());
        }           
    }
    if(choice){
        QVariant data=QString("%1:%2:%3").arg("playerChosen").arg(skillName).arg(choice->objectName());
        thread->trigger(ChoiceMade, player, data);
    }
    return choice;
}

void Room::askForGeneralAsync(ServerPlayer *player){
    if(!player->isOnline())
    {
        if(player->getGeneral())
            choose2Command(player, QString());
        else
            chooseCommand(player, QString());
    }
    else
    {
        Json::Value options = toJsonStringArray(player->getSelected());
        if(!Config.EnableBasara) 
            options.append(toJsonString(QString("%1(lord)").arg(getLord()->getGeneralName())));
        else 
            options.append("anjiang(lord)");
        CommandType command = player->getGeneral() ? S_COMMAND_CHOOSE_GENERAL_2 : S_COMMAND_CHOOSE_GENERAL_1;
        QSanGeneralPacket packet(S_SERVER_REQUEST, command);
        packet.setMessageBody(options);
        player->invoke(&packet);
    }
}

//@todo: merge this function with chooseCommand and choose2Command
QString Room::askForGeneral(ServerPlayer *player, const QStringList &generals, QString default_choice){
    if(default_choice.isEmpty())
        default_choice = generals.at(qrand() % generals.length());

    if(player->isOnline())
    {
        Json::Value options = toJsonStringArray(generals);
        bool success = doRequest(player, S_COMMAND_CHOOSE_GENERAL, options);
        //executeCommand(player, "askForGeneral", "chooseGeneralCommand", generals.join("+"), ".");

        if(!success || !m_clientResponse.isString() || !generals.contains(m_clientResponse.asCString()))
            return default_choice;
        else
            return toQString(m_clientResponse);
    }

    return default_choice;
}

void Room::kickCommand(ServerPlayer *player, const QString &arg){
    // kicking is not allowed at contest mode
    if(Config.ContestMode)
        return;

    // only the lord can kick others
    if(player != getLord())
        return;

    ServerPlayer *to_kick = findChild<ServerPlayer *>(arg);
    if(to_kick == NULL)
        return;

    to_kick->kick();
}

void Room::makeCheat(const QString &cheat_str){
    QRegExp damage_rx(":(.+)->(\\w+):([NTFRL])(\\d+)");
    QRegExp killing_rx(":KILL:(.+)->(\\w+)");
    QRegExp revive_rx(":REVIVE:(.+)");
    QRegExp doscript_rx(":SCRIPT:(.+)");

    if(damage_rx.exactMatch(cheat_str))
        makeDamage(damage_rx.capturedTexts());
    else if(killing_rx.exactMatch(cheat_str)){
        makeKilling(killing_rx.capturedTexts());
    }else if(revive_rx.exactMatch(cheat_str)){
        makeReviving(revive_rx.capturedTexts());
    }else if(doscript_rx.exactMatch(cheat_str)){
        QString script = doscript_rx.capturedTexts().value(1);
        if(!script.isEmpty()){
            QByteArray data = QByteArray::fromBase64(script.toAscii());
            data = qUncompress(data);
            script = data;
            doScript(script);
        }
    }
}

void Room::makeDamage(const QStringList &texts){
    int point = texts.at(4).toInt();

    // damage
    DamageStruct damage;
    if(texts.at(1) != ".")
        damage.from = findChild<ServerPlayer *>(texts.at(1));

    damage.to = findChild<ServerPlayer *>(texts.at(2));

    char nature = texts.at(3).toAscii().at(0);
    switch(nature){
    case 'N': damage.nature = DamageStruct::Normal; break;
    case 'T': damage.nature = DamageStruct::Thunder; break;
    case 'F': damage.nature = DamageStruct::Fire; break;
    case 'L': loseHp(damage.to, point); return;
    case 'R':{
        RecoverStruct recover;
        if(texts.at(1) != ".")
            recover.who = findChild<ServerPlayer *>(texts.at(1));
        ServerPlayer *player = findChild<ServerPlayer *>(texts.at(2));

        recover.recover = point;

        this->recover(player, recover);

        return;
    }
    }

    damage.damage = point;

    this->damage(damage);
}

void Room::makeKilling(const QStringList &texts){
    ServerPlayer *killer = NULL, *victim = NULL;

    if(texts.at(1) != ".")
        killer = findChild<ServerPlayer *>(texts.at(1));

    victim = findChild<ServerPlayer *>(texts.at(2));

    Q_ASSERT(victim);

    if(killer == NULL)
        return killPlayer(victim);

    DamageStruct damage;
    damage.from = killer;
    damage.to = victim;
    killPlayer(victim, &damage);
}

void Room::makeReviving(const QStringList &texts){
    ServerPlayer *player = findChild<ServerPlayer *>(texts.at(1));
    Q_ASSERT(player);
    revivePlayer(player);
    setPlayerProperty(player, "maxhp", player->getGeneralMaxHP());
    setPlayerProperty(player, "hp", player->getMaxHP());
}

void Room::surrenderCommand(ServerPlayer *player, const QString &){
    if(!player->isLord())
        return;

    if(alivePlayerCount() <= 2)
        return;

    QStringList roles = aliveRoles(player);
    bool can_surrender = true;
    foreach(QString role, roles){
        if(role == "loyalist" || role == "renegade"){
            can_surrender = false;
            break;
        }
    }

    if(can_surrender){
        gameOver("rebel");
    }
}

void Room::fillAG(const QList<int> &card_ids, ServerPlayer *who){
    QStringList card_str;
    foreach(int card_id, card_ids)
        card_str << QString::number(card_id);

    if(who)
        who->invoke("fillAG", card_str.join("+"));
    else{
        broadcastInvoke("fillAG", card_str.join("+"));
        broadcastInvoke("disableAG", "true");
    }
}

void Room::takeAG(ServerPlayer *player, int card_id){
    if(player){
        player->addCard(Sanguosha->getCard(card_id), Player::Hand);
        setCardMapping(card_id, player, Player::Hand);
        broadcastInvoke("takeAG", QString("%1:%2").arg(player->objectName()).arg(card_id));
        player->invoke("disableAG", "true");
        CardMoveStruct move;
        move.from = NULL;
        move.from_place = Player::DrawPile;
        move.to = player;
        move.to_place = Player::Hand;
        move.card_id = card_id;
        CardMoveStar move_star = &move;
        QVariant data = QVariant::fromValue(move_star);
        thread->trigger(CardGot, player, data);
        thread->trigger(CardGotDone, player);
    }else{
        discard_pile->prepend(card_id);
        setCardMapping(card_id, NULL, Player::DiscardedPile);
        broadcastInvoke("takeAG", QString(".:%1").arg(card_id));
    }
}

void Room::provide(const Card *card){
    Q_ASSERT(provided == NULL);
    Q_ASSERT(!has_provided);

    provided = card;
    has_provided = true;
}

QList<ServerPlayer *> Room::getLieges(const QString &kingdom, ServerPlayer *lord) const{
    QList<ServerPlayer *> lieges;
    foreach(ServerPlayer *player, alive_players){
        if(player != lord && player->getKingdom() == kingdom)
            lieges << player;
    }

    return lieges;
}

void Room::sendLog(const LogMessage &log){
    if(log.type.isEmpty())
        return;

    broadcastInvoke("log", log.toString());
}

void Room::showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer){
    Json::Value show_str;
    show_str[0] = toJsonString(player->objectName());
    show_str[1] = card_id;
    if(only_viewer)
        doNotify(player, S_COMMAND_SHOW_CARD, show_str);
        //only_viewer->invoke("showCard", show_str);
    else
        doNotify(player, S_COMMAND_SHOW_CARD, show_str, true);
        //broadcastInvoke("showCard", show_str);
}

void Room::showAllCards(ServerPlayer *player, ServerPlayer *to){
    QStringList handcards_str;
    foreach(const Card *card, player->getHandcards())
        handcards_str << QString::number(card->getId());
    QString gongxin_str = QString("%1!:%2").arg(player->objectName()).arg(handcards_str.join("+"));

    if(to)
        to->invoke("doGongxin", gongxin_str);
    else
        broadcastInvoke("doGongxin", gongxin_str, player);
}

bool Room::askForYiji(ServerPlayer *guojia, QList<int> &cards){
    if(cards.isEmpty())
        return false;

    AI *ai = guojia->getAI();
    if(ai){
        int card_id;
        ServerPlayer *who = ai->askForYiji(cards, card_id);
        if(who){
            cards.removeOne(card_id);
            moveCardTo(Sanguosha->getCard(card_id), who, Player::Hand, false);
            return true;
        }else
            return false;
    }else{
        
        bool success = doRequest(guojia, S_COMMAND_SKILL_YIJI, toJsonIntArray(cards));         

        //Validate client response
        if(!success || !m_clientResponse.isArray()
            || m_clientResponse.size() != 2)
            return false;
        
        QList<int> ids;
        if (!tryParse(m_clientResponse[0], ids)
            || !m_clientResponse[1].isString()) return false;

        foreach (int id, ids)        
            if (!cards.contains(id)) return false;
        
        ServerPlayer *who = findChild<ServerPlayer *>(toQString(m_clientResponse[1]));

        if (!who) return false;

        DummyCard *dummy_card = new DummyCard;
        foreach(int card_id, ids){
            cards.removeOne(card_id);
            dummy_card->addSubcard(card_id);
        }

        moveCardTo(dummy_card, who, Player::Hand, false);
        delete dummy_card;

        setEmotion(who, "draw-card");

        return true;
        
    }
}

QString Room::generatePlayerName(){
    static int id = 0;
    id ++;
    return QString("sgs%1").arg(id);
}

void Room::arrangeCommand(ServerPlayer *player, const QString &arg){
    if(mode == "06_3v3")
        thread_3v3->arrange(player, arg.split("+"));
    else if(mode == "02_1v1")
        thread_1v1->arrange(player, arg.split("+"));
}

void Room::takeGeneralCommand(ServerPlayer *player, const QString &arg){
    if(mode == "06_3v3")
        thread_3v3->takeGeneral(player, arg);
    else if(mode == "02_1v1")
        thread_1v1->takeGeneral(player, arg);
}

QString Room::askForOrder(ServerPlayer *player){
    QString reason;
    if(thread_3v3->isFinished())
        reason = "turn";
    else
        reason = "select";

    if(player->getState() == "online"){
        player->invoke("askForOrder", reason);
        reply_player = player;
        reply_func = "selectOrderCommand";
        reply_player->drainLock(ServerPlayer::SEMA_COMMAND);
        reply_player->acquireLock(ServerPlayer::SEMA_COMMAND);
    }else{
        m_clientResponseString = qrand() % 2 == 0 ? "warm" : "cool";
    }

    return m_clientResponseString;
}

void Room::selectOrderCommand(ServerPlayer *player, const QString &arg){
    m_clientResponseString = arg;
    player->releaseLock(ServerPlayer::SEMA_COMMAND);
}

QString Room::askForRole(ServerPlayer *player, const QStringList &roles, const QString &scheme){
    QStringList squeezed = roles.toSet().toList();
    player->invoke("askForRole", QString("%1:%2").arg(scheme).arg(squeezed.join("+")));
    reply_player = player;
    reply_func = "selectRoleCommand";
    
    reply_player->acquireLock(ServerPlayer::SEMA_CHOOSE_ROLE);

    return m_clientResponseString;
}

void Room::selectRoleCommand(ServerPlayer *player, const QString &arg){
    m_clientResponseString = arg;
    if(m_clientResponseString.isEmpty())
        m_clientResponseString = "abstained";

    player->releaseLock(ServerPlayer::SEMA_CHOOSE_ROLE);
}

void Room::networkDelayTestCommand(ServerPlayer *player, const QString &){
    qint64 delay = player->endNetworkDelayTest();
    QString reportStr = tr("<font color=#EEB422>The network delay of player <b>%1</b> is %2 milliseconds.</font>")
            .arg(Config.ContestMode ? tr("Contestant") : player->screenName()).arg(QString::number(delay));
    speakCommand(player, reportStr.toUtf8().toBase64());
}

bool Room::isVirtual()
{
    return _virtual;
}

void Room::setVirtual()
{
    _virtual = true;
}

void Room::copyFrom(Room* rRoom)
{
    QMap<ServerPlayer*, ServerPlayer*> player_map;

    for(int i=0; i<players.length(); i++)
    {

        ServerPlayer* a = rRoom->players.at(i);
        ServerPlayer* b = players.at(i);
        player_map.insert(a, b);

        transfigure(b, a->getGeneralName(), false);

        b->copyFrom(a);
    }
    for(int i=0; i<players.length(); i++)
    {
        ServerPlayer* a = rRoom->players.at(i);
        ServerPlayer* b = players.at(i);
        b->setNext(player_map.value(a->getNext()));
    }

    foreach(ServerPlayer* a,alive_players)
    {
        if(!a->isAlive())alive_players.removeOne(a);
    }
    current = player_map.value(rRoom->getCurrent());

    pile1 = QList<int> (rRoom->pile1);
    pile2 = QList<int> (rRoom->pile2);
    table_cards = QList<int> (rRoom->table_cards);
    draw_pile = &pile1;
    discard_pile = &pile2;

    place_map = QMap<int, Player::Place> (rRoom->place_map);
    owner_map = QMap<int, ServerPlayer*>();

    QList<int> keys = rRoom->owner_map.keys();

    foreach(int i, keys)
        owner_map.insert(i, rRoom->owner_map.value(i));

    provided = rRoom->provided;
    has_provided = rRoom->has_provided;

    tag = QVariantMap(rRoom->tag);

}

Room* Room::duplicate()
{
    Server* svr = qobject_cast<Server *> (parent());
    Room* room = svr->createNewRoom();
    room->setVirtual();
    room->fillRobotsCommand(NULL, 0);
    room->copyFrom(this);
    return room;
}
