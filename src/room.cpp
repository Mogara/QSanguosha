#include "room.h"
#include "engine.h"
#include "settings.h"
#include "standard.h"
#include "ai.h"
#include "scenario.h"
#include "gamerule.h"
#include "contestdb.h"
#include "banpairdialog.h"
#include "roomthread3v3.h"
#include "roomthread1v1.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>
#include <QDateTime>

Room::Room(QObject *parent, const QString &mode)
    :QObject(parent), mode(mode), owner(NULL), current(NULL), reply_player(NULL), pile1(Sanguosha->getRandomCards()),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(Config.CountDownSeconds),
    chosen_generals(0), game_started(false), game_finished(false), signup_count(0),
    L(NULL), thread(NULL), thread_3v3(NULL), sem(new QSemaphore), provided(NULL)
{
    player_count = Sanguosha->getPlayerCount(mode);
    scenario = Sanguosha->getScenario(mode);

    initCallbacks();
}

void Room::initCallbacks(){
    // init callback table
    callbacks["useCardCommand"] = &Room::commonCommand;
    callbacks["invokeSkillCommand"] = &Room::commonCommand;
    callbacks["replyNullificationCommand"] = &Room::commonCommand;
    callbacks["chooseCardCommand"] = &Room::commonCommand;
    callbacks["responseCardCommand"] = &Room::commonCommand;
    callbacks["discardCardsCommand"] = &Room::commonCommand;
    callbacks["chooseSuitCommand"] = &Room::commonCommand;
    callbacks["chooseKingdomCommand"] = &Room::commonCommand;
    callbacks["chooseAGCommand"] = &Room::commonCommand;
    callbacks["choosePlayerCommand"] = &Room::commonCommand;
    callbacks["chooseGeneralCommand"] = &Room::commonCommand;
    callbacks["selectChoiceCommand"] = &Room::commonCommand;
    callbacks["replyYijiCommand"] = &Room::commonCommand;
    callbacks["replyGuanxingCommand"] = &Room::commonCommand;
    callbacks["replyGongxinCommand"] = &Room::commonCommand;

    callbacks["addRobotCommand"] = &Room::addRobotCommand;
    callbacks["fillRobotsCommand"] = &Room::fillRobotsCommand;
    callbacks["signupCommand"] = &Room::signupCommand;
    callbacks["chooseCommand"] = &Room::chooseCommand;
    callbacks["choose2Command"] = &Room::choose2Command;

    callbacks["arrangeCommand"] = &Room::arrangeCommand;
    callbacks["takeGeneralCommand"] = &Room::takeGeneralCommand;
    callbacks["selectOrderCommand"] = &Room::selectOrderCommand;

    callbacks["speakCommand"] = &Room::speakCommand;
    callbacks["trustCommand"] = &Room::trustCommand;
    callbacks["kickCommand"] = &Room::kickCommand;
    callbacks["surrenderCommand"] = &Room::surrenderCommand;
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

void Room::enterDying(ServerPlayer *player, DamageStruct *reason){
    DyingStruct dying;
    dying.who = player;
    dying.damage = reason;

    player->setFlags("dying");

    QVariant dying_data = QVariant::fromValue(dying);
    thread->trigger(Dying, player, dying_data);
}

void Room::revivePlayer(ServerPlayer *player, const General *general){
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

    if(general){
        transfigure(player, general->objectName(), true, true);
    }
}

void Room::killPlayer(ServerPlayer *victim, ServerPlayer *killer){
    if(Config.ContestMode && killer){
        killer->addVictim(victim);
    }

    victim->setAlive(false);
    broadcastProperty(victim, "alive");

    broadcastProperty(victim, "role");
    broadcastInvoke("killPlayer", victim->objectName());

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
    log.arg = victim->getRole();
    log.from = killer;

    QVariant killer_name;
    if(killer){
        killer_name = killer->objectName();

        if(killer == victim)
            log.type = "#Suicide";
        else
            log.type = "#Murder";
    }else{
        log.type = "#Contingency";
    }

    sendLog(log);

    thread->trigger(Death, victim, killer_name);
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

    if(QThread::currentThread() == thread)
        thread->end();
    else
        sem->release();
}

void Room::slashEffect(const SlashEffectStruct &effect){
    effect.from->addMark("SlashCount");

    if(effect.from->getMark("SlashCount") > 1 && effect.from->hasSkill("paoxiao"))
        playSkillEffect("paoxiao");

    QVariant data = QVariant::fromValue(effect);

    setEmotion(effect.from, Killer);
    setEmotion(effect.to, Victim);

    bool broken = thread->trigger(SlashEffect, effect.from, data);
    if(!broken)
        thread->trigger(SlashEffected, effect.to, data);
}

void Room::slashResult(const SlashEffectStruct &effect, bool hit){
    QVariant data = QVariant::fromValue(effect);

    if(hit)
        thread->trigger(SlashHit, effect.from, data);
    else
        thread->trigger(SlashMissed, effect.from, data);
}

void Room::attachSkillToPlayer(ServerPlayer *player, const QString &skill_name){
    player->invoke("attachSkill", skill_name);
}

void Room::detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name){
    player->invoke("detachSkill", skill_name);
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

bool Room::askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data){
    bool invoked;
    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        invoked = ai->askForSkillInvoke(skill_name, data);
    }else{
        player->invoke("askForSkillInvoke", skill_name);
        getResult("invokeSkillCommand", player);

        if(result.isEmpty())
            return askForSkillInvoke(player, skill_name); // recursive call;

        // result should be "yes" or "no"
        invoked =  result == "yes";
    }

    if(invoked)
        broadcastInvoke("skillInvoked", QString("%1:%2").arg(player->objectName()).arg(skill_name));

    return invoked;
}

QString Room::askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForChoice(skill_name, choices);

    QString ask_str = QString("%1:%2").arg(skill_name).arg(choices);
    player->invoke("askForChoice", ask_str);
    getResult("selectChoiceCommand", player);

    if(result.isEmpty())
        return askForChoice(player, skill_name, choices);    

    if(result == "."){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill)
            return skill->getDefaultChoice(player);
    }

    return result;
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

    return askForNullification(effect.card->objectName(), effect.from, effect.to);
}

bool Room::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to){
    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(!player->hasNullification())
            continue;

        trust:
        AI *ai = player->getAI();
        const Card *card = NULL;
        if(ai){
            card = ai->askForNullification(trick_name, from, to);
        }else{
            QString ask_str = QString("%1:%2->%3")
                              .arg(trick_name)
                              .arg(from ? from->objectName() : ".")
                              .arg(to->objectName());

            player->invoke("askForNullification", ask_str);
            getResult("responseCardCommand", player, false);

            if(result.isEmpty())
                goto trust;

            if(result != ".")
                card = Card::Parse(result);
        }

        if(card == NULL)
            continue;

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

        return !askForNullification("nullification", player, to);
    }

    return false;
}

int Room::askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason){
    if(flags == "h" && !who->hasFlag("dongchaee"))
        return who->getRandomHandCardId();

    int card_id;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_id = ai->askForCardChosen(who, flags, reason);
    }else{
        player->invoke("askForCardChosen", QString("%1:%2:%3").arg(who->objectName()).arg(flags).arg(reason));
        getResult("chooseCardCommand", player);

        if(result.isEmpty())
            return askForCardChosen(player, who, flags, reason);

        if(result == "."){
            // randomly choose a card
            QList<const Card *> cards = who->getCards(flags);
            int r = qrand() % cards.length();
            return cards.at(r)->getId();
        }

        card_id = result.toInt();
    }

    if(card_id == -1)
        card_id = who->getRandomHandCardId();

    return card_id;
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt, bool throw_it){
    const Card *card = NULL;

    QVariant asked = pattern;
    thread->trigger(CardAsked, player, asked);
    if(provided){
        card = provided;
        provided = NULL;
    }else{
        AI *ai = player->getAI();
        if(ai){
            thread->delay(Config.AIDelay);
            card = ai->askForCard(pattern, prompt);
        }else{
            player->invoke("askForCard", QString("%1:%2").arg(pattern).arg(prompt));
            getResult("responseCardCommand", player);

            if(result.isEmpty())
                return askForCard(player, pattern, prompt);

            if(result != ".")
                card = Card::Parse(result);
        }
    }

    if(card){
        if(throw_it)
            throwCard(card);

        if(!card->inherits("DummyCard") && !pattern.startsWith(".")){
            LogMessage log;
            log.card_str = card->toString();
            log.from = player;
            log.type = QString("#%1").arg(card->metaObject()->className());
            sendLog(log);

            player->playCardEffect(card);

            CardStar card_ptr = card;
            QVariant card_star = QVariant::fromValue(card_ptr);
            thread->trigger(CardResponsed, player, card_star);
        }
    }

    return card;
}

bool Room::askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt){
    QString answer;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        answer = ai->askForUseCard(pattern, prompt);
    }else{
        player->invoke("askForUseCard", QString("%1:%2").arg(pattern).arg(prompt));
        getResult("useCardCommand", player);

        if(result.isEmpty())
            return askForUseCard(player, pattern, prompt);

        answer = result;
    }

    if(answer != "."){
        CardUseStruct card_use;
        card_use.from = player;
        card_use.parse(answer, this);
        if(card_use.isValid()){
            useCard(card_use);
            return true;
        }
    }

    return false;
}

int Room::askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable){
    if(card_ids.length() == 1)
        return card_ids.first();

    int card_id;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_id = ai->askForAG(card_ids, refusable);
    }else{
        player->invoke("askForAG", refusable ? "?" : ".");
        getResult("chooseAGCommand", player);

        if(result.isEmpty())
            return askForAG(player, card_ids, refusable);

        card_id = result.toInt();
    }

    return card_id;
}

const Card *Room::askForCardShow(ServerPlayer *player, ServerPlayer *requestor){
    if(player->getHandcardNum() == 1){
        return player->getHandcards().first();
    }

    AI *ai = player->getAI();
    if(ai)
        return ai->askForCardShow(requestor);

    player->invoke("askForCardShow", requestor->getGeneralName());
    getResult("responseCardCommand", player);

    if(result.isEmpty())
        return askForCardShow(player, requestor);
    else if(result == ".")
        return player->getRandomHandCard();

    return Card::Parse(result);
}

const Card *Room::askForSinglePeach(ServerPlayer *player, ServerPlayer *dying){
    if(player->isKongcheng()){
        // jijiu special case
        if(player->hasSkill("jijiu") && player->getPhase() == Player::NotActive){
            ServerPlayer *huatuo = player;
            QList<const Card *> equips = huatuo->getEquips();
            bool has_red = false;
            foreach(const Card *equip, equips){
                if(equip->isRed()){
                    has_red = true;
                    break;
                }
            }

            if(!has_red)
                return NULL;
        }else if(player->hasSkill("jiushi")){
            if(!player->faceUp())
                return NULL;
        }else
            return NULL;
    }

    AI *ai = player->getAI();
    if(ai)
        return ai->askForSinglePeach(dying);

    int peaches = 1 - dying->getHp();
    player->invoke("askForSinglePeach", QString("%1:%2").arg(dying->objectName()).arg(peaches));
    getResult("responseCardCommand", player);

    if(result.isEmpty())
        return askForSinglePeach(player, dying);

    if(result == ".")
        return NULL;

    return Card::Parse(result);
}

void Room::setPlayerFlag(ServerPlayer *player, const QString &flag){
    player->setFlags(flag);
    broadcast(QString("#%1 flags %2").arg(player->objectName()).arg(flag));
}

void Room::setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value){
    player->setProperty(property_name, value);
    broadcastProperty(player, property_name);
}

void Room::setPlayerMark(ServerPlayer *player, const QString &mark, int value){
    player->setMark(mark, value);
    broadcastInvoke("setMark", QString("%1.%2=%3").arg(player->objectName()).arg(mark).arg(value));
}

void Room::addSocket(ClientSocket *socket){
    ServerPlayer *player = new ServerPlayer(this);
    player->setSocket(socket);
    players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processRequest(QString)));
}

bool Room::isFull() const
{
    return signup_count == player_count;
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

    qSwap(draw_pile, discard_pile);

    broadcastInvoke("clearPile");
    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    qShuffle(*draw_pile);

    foreach(int card_id, *draw_pile){
        setCardMapping(card_id, NULL, Player::DrawPile);
    }
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

void Room::transfigure(ServerPlayer *player, const QString &new_general, bool full_state, bool invoke_start){
    LogMessage log;
    log.type = "#Transfigure";
    log.from = player;
    log.arg = new_general;
    sendLog(log);

    player->invoke("detachAllSkills");

    thread->removePlayerSkills(player);
    setPlayerProperty(player, "general", new_general);
    thread->addPlayerSkills(player, invoke_start);

    player->setMaxHP(player->getGeneralMaxHP());
    broadcastProperty(player, "maxhp");

    if(full_state)
        player->setHp(player->getMaxHP());
    broadcastProperty(player, "hp");
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
    QString choice;

    if(player->getState() == "online"){
        player->invoke("askForDirection");
        getResult("selectChoiceCommand", player);

        if(result.isEmpty() || result == ".")
            choice = "ccw";
        else
            choice = result;
    }else{
        const TrickCard *trick = qobject_cast<const TrickCard *>(card);
        if(trick->isAggressive()){
            if(AI::GetRelation3v3(player, player->getNextAlive()) == AI::Enemy)
                choice = "ccw";
            else
                choice = "cw";
        }else{
            if(AI::GetRelation3v3(player, player->getNextAlive()) == AI::Friend)
                choice = "ccw";
            else
                choice = "cw";
        }
    }

    LogMessage log;
    log.type = "#TrickDirection";
    log.from = player;
    log.arg = choice;
    log.arg2 = card->objectName();
    sendLog(log);

    if(choice == "cw"){
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

void Room::addProhibitSkill(const ProhibitSkill *skill){
    if(!prohibit_skills.contains(skill)){
        prohibit_skills << skill;

        broadcastInvoke("addProhibitSkill", skill->objectName());
    }
}

const ProhibitSkill *Room::isProhibited(Player *from, Player *to, const Card *card) const{
    foreach(const ProhibitSkill *skill, prohibit_skills){
        if(to->hasSkill(skill->objectName()) && skill->isProhibited(from, to, card))
            return skill;
    }

    return NULL;
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

void Room::timerEvent(QTimerEvent *event){
    if(left_seconds > 0){
        left_seconds --;
        broadcastInvoke("startInXs", QString::number(left_seconds));
    }else{
        killTimer(event->timerId());

        if(scenario)
            startGame();
        else if(mode == "06_3v3"){
            thread_3v3 = new RoomThread3v3(this);
            thread_3v3->start();

            connect(thread_3v3, SIGNAL(finished()), this, SLOT(startGame()));
        }else if(mode == "02_1v1"){
            thread_1v1 = new RoomThread1v1(this);
            thread_1v1->start();

            connect(thread_1v1, SIGNAL(finished()), this, SLOT(startGame()));
        }else{
            QStringList lord_list = Sanguosha->getRandomLords();
            QString default_lord = lord_list[qrand() % lord_list.length()];
            ServerPlayer *the_lord = getLord();
            if(the_lord->getState() != "online") {
                chooseCommand(the_lord, default_lord);
            }else{
                the_lord->setProperty("default_general", default_lord);
                the_lord->invoke("doChooseGeneral", lord_list.join("+"));
            }
        }
    }
}

void Room::prepareForStart(){
    if(scenario){
        QStringList generals, roles;
        scenario->assign(generals, roles);

        bool expose_roles = scenario->exposeRoles();
        int i;
        for(i=0; i<players.length(); i++){
            ServerPlayer *player = players.at(i);
            player->setGeneralName(generals.at(i));
            player->setRole(roles.at(i));

            broadcastProperty(player, "general");

            if(player->isLord())
                broadcastProperty(player, "role");

            if(expose_roles)
                broadcastProperty(player, "role");
            else
                player->sendProperty("role");
        }
    }else if(mode == "06_3v3"){
        qShuffle(players);

        ServerPlayer *online = NULL;
        int online_num = 0;
        foreach(ServerPlayer *player, players){
            if(player->getState() == "online"){
                online = player;
                online_num ++;
            }
        }

        if(online_num == 1){
            players.removeOne(online);

            int r = qrand() % 2;
            if(r == 1)
                players.prepend(online);
            else
                players.insert(3, online);
        }

        static QStringList roles;
        if(roles.isEmpty()){
            roles << "lord" << "loyalist" << "rebel"
                    << "renegade" << "rebel" << "loyalist";
        }

        int i;
        for(i=0; i<6; i++){
            players.at(i)->setRole(roles.at(i));
            broadcastProperty(players.at(i), "role");
        }
    }else if(mode == "02_1v1"){
        if(qrand() % 2 == 0)
            players.swap(0, 1);

        players.at(0)->setRole("lord");
        players.at(1)->setRole("renegade");

        int i;
        for(i=0; i<2; i++){
            broadcastProperty(players.at(i), "role");
        }

    }else{
        assignRoles();
    }

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
        if(signup_count < player_count){
            player->setParent(NULL);
            players.removeOne(player);

            broadcastInvoke("removePlayer", player->objectName());
            signup_count --;
        }
    }else{
        if(!game_started){
            // third case
            if(player->getGeneral() == NULL){
                if(player->property("default_general").isValid())
                    chooseCommand(player, QString());
            }

            if(player->property("default_general2").isValid())
                choose2Command(player, QString());
        }

        // fourth case
        setPlayerProperty(player, "state", "offline");

        bool someone_is_online = false;
        foreach(ServerPlayer *player, players){
            if(player->getState() == "online"){
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
            result.clear();

            sem->release();
        }
    }

    if(player == owner){
        owner = NULL;
        foreach(ServerPlayer *p, players){
            if(p->getState() == "online"){
                owner = p;
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
            result.clear();

            sem->release();
        }
    }else
        player->setState("online");

    broadcastProperty(player, "state");
}

void Room::processRequest(const QString &request){
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
        emit room_message(player->reportHeader() + request);
    }else
        emit room_message(QString("%1: %2 is not invokable").arg(player->reportHeader()).arg(command));
}

void Room::addRobotCommand(ServerPlayer *player, const QString &){
    if(player != owner)
        return;

    if(isFull())
        return;

    int n = 0;
    foreach(ServerPlayer *player, players){
        if(player->getState() == "robot")
            n ++;
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
    int left = player_count - signup_count, i;
    for(i=0; i<left; i++){
        addRobotCommand(player, QString());
    }
}

void Room::signup(ServerPlayer *player, const QString &screen_name, const QString &avatar, bool is_robot){
    player->setObjectName(generatePlayerName());    
    player->setProperty("avatar", avatar);
    player->setScreenName(screen_name);

    if(Config.ContestMode)
        player->startRecord();

    if(!is_robot){
        player->invoke("checkVersion", Sanguosha->getVersion());
        player->invoke("setup", Sanguosha->getSetupString());

        player->sendProperty("objectName");

        if(owner == NULL){
            owner = player;
            player->setOwner(true);
            broadcastProperty(player, "owner");
        }
    }

    // introduce the new joined player to existing players except himself
    QString contestant = tr("Contestant").toUtf8().toBase64();
    QString base64;
    if(Config.ContestMode)
        base64 = contestant;
    else
        base64 = screen_name.toUtf8().toBase64();
    broadcastInvoke("addPlayer", QString("%1:%2:%3").arg(player->objectName()).arg(base64).arg(avatar), player);

    if(!is_robot){
        // introduce all existing player to the new joined
        foreach(ServerPlayer *p, players){
            if(p == player)
                continue;

            QString name = p->objectName();            
            QString base64;
            if(Config.ContestMode)
                base64 = contestant;
            else
                base64 = p->screenName().toUtf8().toBase64();
            QString avatar = p->property("avatar").toString();

            player->invoke("addPlayer", QString("%1:%2:%3").arg(name).arg(base64).arg(avatar));
        }
    }

    signup_count ++;
    if(isFull()){
        prepareForStart();

#ifndef QT_NO_DEBUG
        left_seconds = 1;
#endif
        broadcastInvoke("startInXs", QString::number(left_seconds));
        startTimer(1000);
    }
}

void Room::signupCommand(ServerPlayer *player, const QString &arg){
    QStringList words = arg.split(":");

    QString base64 = words[0];
    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    QString screen_name = QString::fromUtf8(data);

    QString avatar = words[1];

    if(Config.ContestMode){
        QString password = words.value(2);
        if(password.isEmpty()){
            player->invoke("warn", "REQUIRE_PASSWORD");
            return;
        }

        ContestDB *db = ContestDB::GetInstance();
        if(!db->checkPassword(screen_name, password)){
            player->invoke("warn", "WRONG_PASSWORD");
            return;
        }
    }

    signup(player, screen_name, avatar, false);
}

void Room::assignRoles(){
    int n = players.count(), i;

    char roles[100];
    Sanguosha->getRoles(mode, roles);

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;

        qSwap(roles[r1], roles[r2]);
    }

    for(i=0; i<n; i++){
        ServerPlayer *player = players[i];

        QString role;
        switch(roles[i]){
        case 'Z': role = "lord"; break;
        case 'C': role = "loyalist"; break;
        case 'F': role = "rebel"; break;
        case 'N': role = "renegade"; break;
        }

        Q_ASSERT(!role.isEmpty());

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
    if(!Config.Enable2ndGeneral || player->getGeneral2())
        return;

    QString name = general_name;
    if(name.isEmpty())
        name = player->property("default_general2").toString();

    player->setGeneral2Name(name);
    player->sendProperty("general2");

    chosen_generals ++;
    if(chosen_generals == player_count * 2){
        startGame();
    }
}

void Room::chooseCommand(ServerPlayer *player, const QString &general_name){
    if(player->getGeneral()){
        // the player has chosen player, should ignore it
        return;
    }

    QString name = general_name;
    if(name.isEmpty())
        name = player->property("default_general").toString();

    player->setGeneralName(name);

    static const int max_choice = 5;
    const int total = Sanguosha->getGeneralCount();

    if(player->getRoleEnum() == Player::Lord){
        broadcastProperty(player, "general");

        const int max_available = (total-1) / (player_count-1);
        const int choice_count = qMin(max_choice, max_available);
        QSet<QString> the_lord;
        the_lord << player->getGeneralName();
        QStringList general_list = Sanguosha->getRandomGenerals((player_count-1) * choice_count, the_lord);

        int i,j;
        for(i=1; i<player_count; i++){
            QStringList choices;
            for(j=0; j<choice_count; j++)
                choices << general_list[(i-1)*choice_count + j];

            ServerPlayer *p = players.at(i);

            QString default_general = choices.first();
            if(p->getState() != "online"){
                chooseCommand(p, default_general);
            }else{
                p->setProperty("default_general", default_general);
                p->invoke("doChooseGeneral", choices.join("+"));
            }
        }
    }else
        player->sendProperty("general");

    chosen_generals ++;
    if(chosen_generals == player_count){
        if(Config.Enable2ndGeneral){
            const int max_available = (total - player_count) / player_count;
            const int choice_count = qMin(max_choice, max_available);

            QSet<QString> exists;
            foreach(ServerPlayer *player, players){
                exists << player->getGeneralName();
            }

            QStringList general_list = Sanguosha->getRandomGenerals(total - exists.size(), exists);
            int i, j;
            for(i=0; i<player_count; i++){
                QStringList choices;
                ServerPlayer *p = players.at(i);
                for(j=0; j< choice_count; j++){
                    QString choice;
                    foreach(QString general, general_list){
                        if(!BanPair::isBanned(p->getGeneralName(), general)){
                            choice = general;
                            break;
                        }
                    }

                    if(choice.isNull())
                        choice = general_list.first();

                    general_list.removeOne(choice);
                    choices << choice;
                }

                QString default_general2 = choices.first();
                if(p->getState() != "online"){
                    choose2Command(p, default_general2);
                }else{
                    p->setProperty("default_general2", default_general2);
                    p->invoke("doChooseGeneral2", choices.join("+"));
                }
            }

        }else
            startGame();
    }
}

void Room::speakCommand(ServerPlayer *player, const QString &arg){
    broadcastInvoke("speak", QString("%1:%2").arg(player->objectName()).arg(arg));
}

void Room::commonCommand(ServerPlayer *player, const QString &arg){
    result = arg;

    reply_player = NULL;
    reply_func.clear();

    sem->release();
}

void Room::useCard(const CardUseStruct &card_use){
    card_use.card->onUse(this, card_use);
}

void Room::loseHp(ServerPlayer *victim, int lose){
    setPlayerProperty(victim, "hp", victim->getHp() - lose);
    broadcastInvoke("hpChange", QString("%1:%2").arg(victim->objectName()).arg(-lose));

    if(victim->getHp() <= 0)
        enterDying(victim, NULL);
}

void Room::loseMaxHp(ServerPlayer *victim, int lose){
    victim->setMaxHP(qMax(victim->getMaxHP() - lose, 0));

    broadcastProperty(victim, "maxhp");
    broadcastProperty(victim, "hp");

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
        setEmotion(player, Recover);
    }
}

void Room::playCardEffect(const QString &card_name, bool is_male){
    QString gender = is_male ? "M" : "F";
    broadcastInvoke("playCardEffect", QString("%1:%2").arg(card_name).arg(gender));
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

bool Room::hasWelfare(ServerPlayer *player) const{
    if(mode == "06_3v3")
        return player->isLord() || player->getRole() == "renegade";
    else
        return player->isLord() && player_count > 4;
}

void Room::startGame(){    
    if(Config.ContestMode)
        tag.insert("StartTime", QDateTime::currentDateTime());

    int i;
    if(scenario == NULL){
        int start_index = 1;
        if(mode == "06_3v3" || mode == "02_1v1")
            start_index = 0;

        for(i = start_index; i < players.count(); i++)
            broadcastProperty(players.at(i), "general");
    }

    if(Config.Enable2ndGeneral || mode == "08boss"){
        foreach(ServerPlayer *player, players)
            broadcastProperty(player, "general2");
    }

    alive_players = players;
    for(i=0; i<player_count-1; i++)
        players.at(i)->setNext(players.at(i+1));
    players.last()->setNext(players.first());

    foreach(ServerPlayer *player, players){
        if(hasWelfare(player))
            player->setMaxHP(player->getGeneralMaxHP() + 1);
        else
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

    current = players.first();

    // initialize the place_map and owner_map;
    foreach(int card_id, *draw_pile){
        setCardMapping(card_id, NULL, Player::DrawPile);
    }

    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    thread = new RoomThread(this);

    GameRule *game_rule;
    if(mode == "08boss")
        game_rule = new BossMode(this);
    else
        game_rule = new GameRule(this);

    thread->constructTriggerTable(game_rule);

    if(scenario){
        const ScenarioRule *rule = scenario->getRule();
        if(rule)
            thread->addTriggerSkill(rule);
    }

    thread->start();
}

void Room::broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value){
    if(value.isNull()){
        QString real_value = player->property(property_name).toString();
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(real_value));
    }else
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(value));
}

void Room::drawCards(ServerPlayer *player, int n){
    if(n <= 0)
        return;

    QList<int> card_ids;
    QStringList cards_str;

    int i;
    for(i=0; i<n; i++){
        int card_id = drawCard();
        card_ids << card_id;
        const Card *card = Sanguosha->getCard(card_id);
        player->drawCard(card);

        cards_str << QString::number(card_id);

        // update place_map & owner_map
        setCardMapping(card_id, player, Player::Hand);
    }

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
}

void Room::throwCard(const Card *card){
    if(card == NULL)
        return;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards)
            throwCard(subcard);
    }else
        throwCard(card->getId());
}

void Room::throwCard(int card_id){
    moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DiscardedPile, true);
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

        int n = card->isVirtualCard() ? card->subcardsLength() : 1;
        QString private_move = QString("%1:%2->%3")
                               .arg(n)
                               .arg(from->objectName())
                               .arg(to->objectName());


        foreach(ServerPlayer *player, players){
            if(!scope.contains(player))
                player->invoke("moveNCards", private_move);
        }
    }

    CardMoveStruct move;
    move.to = to;
    move.to_place = place;
    move.open = open;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards){
            move.card_id = subcard;
            move.from = getCardOwner(subcard);
            move.from_place = getCardPlace(subcard);
            doMove(move, scope);
        }
    }else{
        move.card_id = card->getId();
        move.from = getCardOwner(move.card_id);
        move.from_place = getCardPlace(move.card_id);
        doMove(move, scope);
    }
}

void Room::doMove(const CardMoveStruct &move, const QSet<ServerPlayer *> &scope){
    // avoid useless operation
    if(move.from == move.to && move.from_place == move.to_place)
        return;

    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from)
        move.from->removeCard(card, move.from_place);
    else{
        if(move.from_place == Player::DiscardedPile)
            discard_pile->removeOne(move.card_id);
        else if(move.from_place == Player::DrawPile)
            draw_pile->removeOne(move.card_id);
        else if(move.from_place == Player::Special)
            table_cards.removeOne(move.card_id);
    }

    if(move.to){
        move.to->addCard(card, move.to_place);
    }else{
        if(move.to_place == Player::DiscardedPile)
            discard_pile->prepend(move.card_id);
        else if(move.to_place == Player::DrawPile)
            draw_pile->prepend(move.card_id);
        else if(move.to_place == Player::Special)
            table_cards.append(move.card_id);
    }

    setCardMapping(move.card_id, move.to, move.to_place);

    QString move_str = move.toString();
    if(!move.open){
        foreach(ServerPlayer *player, players){
            if(scope.contains(player))
                player->invoke("moveCard", move_str);
        }
    }else{
        broadcastInvoke("moveCard", move_str);
    }

    if(move.from){
        QVariant data = QVariant::fromValue(move);
        thread->trigger(CardLost, move.from, data);
    }

    if(move.to){
        QVariant data = QVariant::fromValue(move);
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

void Room::broadcastInvoke(const char *method, const QString &arg, ServerPlayer *except){
    broadcast(QString("%1 %2").arg(method).arg(arg), except);
}

void Room::getResult(const QString &reply_func, ServerPlayer *reply_player, bool move_focus){
    if(move_focus)
        broadcastInvoke("moveFocus", reply_player->objectName(), reply_player);

    this->reply_func = reply_func;
    this->reply_player = reply_player;

    sem->acquire();

    if(game_finished)
        thread->end();
}

void Room::acquireSkill(ServerPlayer *player, const Skill *skill, bool open){
    QString skill_name = skill->objectName();
    player->acquireSkill(skill_name);

    if(skill->inherits("TriggerSkill")){
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        thread->addTriggerSkill(trigger_skill);
    }

    if(open){
        QString acquire_str = QString("%1:%2").arg(player->objectName()).arg(skill_name);
        broadcastInvoke("acquireSkill", acquire_str);
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

void Room::setEmotion(ServerPlayer *target, TargetType type){
    QString emotion;
    switch(type){
    case Killer: emotion = "killer"; break;
    case Victim: emotion = "victim"; break;
    case DuelA: emotion = "duel-a"; break;
    case DuelB: emotion = "duel-b"; break;
    case Good: emotion = "good"; break;
    case Bad: emotion = "bad"; break;
    case Recover: emotion = "recover"; break;
    case DrawCard: emotion = "draw-card"; break;
    }

    broadcastInvoke("setEmotion", QString("%1:%2").arg(target->objectName()).arg(emotion), target);
}

void Room::activate(ServerPlayer *player, CardUseStruct &card_use){
    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_use.from = player;
        ai->activate(card_use);        
    }else{
        broadcastInvoke("activate", player->objectName());
        getResult("useCardCommand", player);

        if(result.isEmpty())
            return activate(player, card_use);

        if(result == ".")
            return;

        card_use.from = player;
        card_use.parse(result, this);

        if(!card_use.isValid()){
            emit room_message(tr("Card can not parse:\n %1").arg(result));
            return;
        }
    }
}

Card::Suit Room::askForSuit(ServerPlayer *player){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForSuit();

    player->invoke("askForSuit");
    getResult("chooseSuitCommand", player);

    if(result.isEmpty())
        return askForSuit(player);

    Card::Suit suit;
    if(result == ".")
        return Card::AllSuits[qrand() % 4];
    if(result == "spade")
        suit = Card::Spade;
    else if(result == "club")
        suit = Card::Club;
    else if(result == "heart")
        suit = Card::Heart;
    else
        suit = Card::Diamond;

    return suit;
}

QString Room::askForKingdom(ServerPlayer *player){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForKingdom();

    player->invoke("askForKingdom");
    getResult("chooseKingdomCommand", player);

    if(result.isEmpty())
        return askForKingdom(player);

    if(result == ".")
        return "wei";
    else
        return result;
}

bool Room::askForDiscard(ServerPlayer *target, const QString &reason, int discard_num, bool optional, bool include_equip){
    AI *ai = target->getAI();
    QList<int> to_discard;
    if(ai) {
        to_discard = ai->askForDiscard(reason, discard_num, optional, include_equip);
    }else{
        QString ask_str = QString::number(discard_num);
        if(optional)
            ask_str.append("o");
        if(include_equip)
            ask_str.append("e");

        target->invoke("askForDiscard", ask_str);
        getResult("discardCardsCommand", target);

        if(result.isEmpty())
            return askForDiscard(target, reason, discard_num, optional, include_equip);

        if(result == "."){
            if(optional)
                return false;

            // time is up, and the server choose the cards to discard
            to_discard = target->forceToDiscard(discard_num, include_equip);
        }else{
            QStringList card_strs = result.split("+");
            foreach(QString card_str, card_strs){
                int card_id = card_str.toInt();
                to_discard << card_id;
            }
        }
    }

    if(to_discard.isEmpty())
        return false;

    foreach(int card_id, to_discard){
        throwCard(card_id);

        LogMessage log;
        log.type = "$DiscardCard";
        log.from = target;
        log.card_str = QString::number(card_id);
        sendLog(log);
    }

    DummyCard *dummy_card = new DummyCard;
    foreach(int card_id, to_discard)
        dummy_card->addSubcard(card_id);

    CardStar card_star = dummy_card;
    QVariant data = QVariant::fromValue(card_star);
    thread->trigger(CardDiscarded, target, data);

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
        player->invoke("askForExchange", QString::number(discard_num));
        getResult("discardCardsCommand", player);

        if(result.isEmpty())
            return askForExchange(player, reason, discard_num);

        if(result == "."){
            to_exchange = player->forceToDiscard(discard_num, false);
        }else{
            QStringList card_strs = result.split("+");
            foreach(QString card_str, card_strs)
                to_exchange << card_str.toInt();
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

void Room::doGuanxing(ServerPlayer *zhuge, const QList<int> &cards, bool up_only){
    // if Zhuge Liang is not online, just return immediately
    if(zhuge->getState() != "online")
        return;

    QString guanxing_str = Card::IdsToStrings(cards).join("+");
    if(up_only)
        guanxing_str.append("!");
    zhuge->invoke("doGuanxing", guanxing_str);
    getResult("replyGuanxingCommand", zhuge);

    if(result.isEmpty()){
        // the method "doGuanxing" without any arguments
        // means to clear all the guanxing items
        zhuge->invoke("doGuanxing");
        foreach(int card_id, cards)
            draw_pile->prepend(card_id);
        return;
    }

    QStringList results = result.split(":");

    Q_ASSERT(results.length() == 2);

    QString top_str = results.at(0);
    QString bottom_str = results.at(1);

    QStringList top_list;
    if(!top_str.isEmpty())
        top_list = top_str.split("+");

    QStringList bottom_list;
    if(!bottom_str.isEmpty())
        bottom_list = bottom_str.split("+");

    Q_ASSERT(top_list.length() + bottom_list.length() == cards.length());

    LogMessage log;
    log.type = "#GuanxingResult";
    log.from = zhuge;
    log.arg = QString::number(top_list.length());
    log.arg2 = QString::number(bottom_list.length());
    sendLog(log);

    QList<int> top_cards = Card::StringsToIds(top_list);
    QList<int> bottom_cards = Card::StringsToIds(bottom_list);

    QListIterator<int> i(top_cards);
    i.toBack();
    while(i.hasPrevious())
        draw_pile->prepend(i.previous());

    i = bottom_cards;
    while(i.hasNext())
        draw_pile->append(i.next());
}

void Room::doGongxin(ServerPlayer *shenlumeng, ServerPlayer *target){
    if(shenlumeng->getState() != "online"){
        // throw the first card which suit is Heart
        QList<const Card *> cards = target->getHandcards();
        foreach(const Card *card, cards){
            if(card->getSuit() == Card::Heart){
                showCard(target, card->getEffectiveId());
                thread->delay();
                throwCard(card);
                return;
            }
        }

        return;
    }

    QList<int> handcards = target->handCards();

    QStringList handcards_str;
    foreach(int handcard, handcards)
        handcards_str << QString::number(handcard);

    shenlumeng->invoke("doGongxin", QString("%1:%2").arg(target->objectName()).arg(handcards_str.join("+")));
    getResult("replyGongxinCommand", shenlumeng);

    if(result.isEmpty() || result == ".")
        return;

    int card_id = result.toInt();
    showCard(target, card_id);    

    QString result = askForChoice(shenlumeng, "gongxin", "discard+put");
    if(result == "discard")
        throwCard(card_id);
    else{
        moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DrawPile, true);
    }
}

const Card *Room::askForPindian(ServerPlayer *player,
                                ServerPlayer *from,
                                ServerPlayer *to,
                                const QString &reason)
{
    if(player->getHandcardNum() == 1){
        return player->getHandcards().first();
    }

    AI *ai = player->getAI();
    if(ai)
        return ai->askForPindian(from, reason);

    QString ask_str = QString("%1->%2")
                       .arg(from->objectName())
                       .arg(to->objectName());

    player->invoke("askForPindian", ask_str);
    getResult("responseCardCommand", player);

    if(result.isEmpty())
        return askForPindian(player, from, to, reason);
    else if(result == "."){
        int card_id = player->getRandomHandCardId();
        return Sanguosha->getCard(card_id);
    }else{
        const Card *card = Card::Parse(result);
        if(card->isVirtualCard()){
            const Card *real_card = Sanguosha->getCard(card->getEffectiveId());
            delete card;
            return real_card;
        }else
            return card;
    }
}

ServerPlayer *Room::askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const QString &reason){
    if(targets.isEmpty())
        return NULL;
    else if(targets.length() == 1)
        return targets.first();

    AI *ai = player->getAI();
    if(ai)
        return ai->askForPlayerChosen(targets, reason);

    QStringList options;
    foreach(ServerPlayer *target, targets)
        options << target->objectName();

    player->invoke("askForPlayerChosen", options.join("+"));

    getResult("choosePlayerCommand", player);

    if(result.isEmpty())
        return askForPlayerChosen(player, targets, reason);

    QString player_name = result;
    if(player_name == ".")
        return NULL;
    else
        return findChild<ServerPlayer *>(player_name);
}

QString Room::askForGeneral(ServerPlayer *player, const QStringList &generals){
    AI *ai = player->getAI();
    if(ai)
        return generals.first();

    player->invoke("askForGeneral", generals.join("+"));
    getResult("chooseGeneralCommand", player);

    if(result.isEmpty() || result == ".")
        return generals.first();
    else
        return result;
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
    else
        broadcastInvoke("fillAG", card_str.join("+"));
}

void Room::takeAG(ServerPlayer *player, int card_id){
    if(player){
        player->addCard(Sanguosha->getCard(card_id), Player::Hand);
        setCardMapping(card_id, player, Player::Hand);
        broadcastInvoke("takeAG", QString("%1:%2").arg(player->objectName()).arg(card_id));
    }else{
        discard_pile->prepend(card_id);
        setCardMapping(card_id, NULL, Player::DiscardedPile);
        broadcastInvoke("takeAG", QString(".:%1").arg(card_id));
    }
}

void Room::provide(const Card *card){
    Q_ASSERT(provided == NULL);

    provided = card;
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

void Room::showCard(ServerPlayer *player, int card_id){
    broadcastInvoke("showCard", QString("%1:%2").arg(player->objectName()).arg(card_id));
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
            who->obtainCard(Sanguosha->getCard(card_id));
            return true;
        }else
            return false;
    }else{
        QStringList card_str;
        foreach(int card_id, cards)
            card_str << QString::number(card_id);

        guojia->invoke("askForYiji", card_str.join("+"));
        getResult("replyYijiCommand", guojia);

        if(result.isEmpty() || result == ".")
            return false;
        else{
            QRegExp rx("(.+)->(\\w+)");
            rx.exactMatch(result);

            QStringList texts = rx.capturedTexts();
            QList<int> ids = Card::StringsToIds(texts.at(1).split("+"));
            ServerPlayer *who = findChild<ServerPlayer *>(texts.at(2));

            DummyCard *dummy_card = new DummyCard;
            foreach(int card_id, ids){
                cards.removeOne(card_id);
                dummy_card->addSubcard(card_id);
            }

            moveCardTo(dummy_card, who, Player::Hand, false);
            delete dummy_card;

            setEmotion(who, DrawCard);

            return true;
        }
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
        sem->acquire();
    }else{
        result = qrand() % 2 == 0 ? "warm" : "cool";
    }

    return result;
}

void Room::selectOrderCommand(ServerPlayer *, const QString &arg){
    result = arg;
    sem->release();
}
