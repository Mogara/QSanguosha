#include "room.h"
#include "engine.h"
#include "settings.h"
#include "standard.h"
#include "ai.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>

LogMessage::LogMessage()
    :from(NULL)
{
}

QString LogMessage::toString() const{
    QStringList tos;
    foreach(ServerPlayer *player, to)
        tos << player->getGeneralName();

    return QString("%1:%2->%3:%4:%5:%6")
            .arg(type)
            .arg(from ? from->getGeneralName() : "")
            .arg(tos.join("+"))
            .arg(card_str).arg(arg).arg(arg2);
}

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count), current(NULL), reply_player(NULL),
    pile1(Sanguosha->getRandomCards()),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(Config.CountDownSeconds),
    chosen_generals(0), game_started(false), game_finished(false), signup_count(0),
    thread(NULL), sem(NULL), provided(NULL)
{
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
    callbacks["selectChoiceCommand"] = &Room::commonCommand;
    callbacks["replyYijiCommand"] = &Room::commonCommand;
    callbacks["replyGuanxingCommand"] = &Room::commonCommand;
    callbacks["replyGongxinCommand"] = &Room::commonCommand;
    callbacks["replyNullificationCommand"] = &Room::commonCommand;

    callbacks["signupCommand"] = &Room::signupCommand;
    callbacks["chooseCommand"] = &Room::chooseCommand;

    callbacks["speakCommand"] = &Room::speakCommand;
    callbacks["trustCommand"] = &Room::trustCommand;
    callbacks["kickCommand"] = &Room::kickCommand;
    callbacks["surrenderCommand"] = &Room::surrenderCommand;
}

ServerPlayer *Room::getCurrent() const{
    return current;
}

int Room::alivePlayerCount() const{
    return alive_players.count();
}

QList<ServerPlayer *> Room::getOtherPlayers(ServerPlayer *except){
    int index = alive_players.indexOf(except);

    QList<ServerPlayer *> other_players;
    int i;
    for(i=index+1; i<alive_players.length(); i++)
        other_players << alive_players.at(i);

    for(i=0; i<index; i++)
        other_players << alive_players.at(i);

    return other_players;
}

QList<ServerPlayer *> Room::getAllPlayers(){
    if(current == NULL)
        return alive_players;

    int index = alive_players.indexOf(current);

    QList<ServerPlayer *> all_players;
    int i;
    for(i=index; i<alive_players.length(); i++)
        all_players << alive_players.at(i);

    for(i=0; i<index; i++)
        all_players << alive_players.at(i);

    return all_players;
}

void Room::nextPlayer(){
    ServerPlayer *next = current;

    forever{
        int index = players.indexOf(next);
        int next_index = (index + 1) % players.length();
        next = players.at(next_index);

        if(!next->isAlive())
            continue;

        if(!next->faceUp()){
            next->turnOver();
            broadcastProperty(next, "faceup");
        }else{
            current = next;
            return;
        }
    }
}

void Room::output(const QString &message){
    emit room_message(message);
}

void Room::killPlayer(ServerPlayer *victim, ServerPlayer *killer){
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

const Card *Room::getJudgeCard(ServerPlayer *player){
    int card_id = drawCard();
    throwCard(card_id);

    LogMessage log;
    log.type = "$InitialJudge";
    log.from = player;
    log.card_str = QString::number(card_id);
    sendLog(log);

    QList<ServerPlayer *> all_players = getAllPlayers();
    foreach(ServerPlayer *p, all_players){
        if(p->hasSkill("guicai")){
            ServerPlayer *simayi = p;
            if(simayi->isKongcheng())
                continue;

            const Card *card = askForCard(simayi, "@guicai", "@guicai-card");
            if(card){
                QList<int> subcards = card->getSubcards();
                Q_ASSERT(!subcards.isEmpty());
                card_id = subcards.first();

                log.type = "$ChangedJudge";
                log.card_str = QString::number(card_id);
                sendLog(log);

                setEmotion(simayi, Normal);
            }
        }

        if(p->hasSkill("guidao")){
            ServerPlayer *zhangjiao = p;
            if(zhangjiao->isNude())
                continue;

            const Card *card = askForCard(zhangjiao, "@guidao", "@guidao-card");
            if(card){
                QList<int> subcards = card->getSubcards();
                Q_ASSERT(!subcards.isEmpty());
                obtainCard(zhangjiao, card_id);
                card_id = subcards.first();

                log.type = "$ChangedJudge";
                log.card_str = QString::number(card_id);
                sendLog(log);

                setEmotion(zhangjiao, Normal);
            }
        }
    }

    log.type = "$JudgeResult";
    log.card_str = QString::number(card_id);
    sendLog(log);

    // judge delay
    thread->delay();   

    CardStar card = Sanguosha->getCard(card_id);
    QVariant card_data = QVariant::fromValue(card);
    thread->trigger(JudgeOnEffect, player, card_data);

    return card;
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
    broadcastInvoke("gameOver", QString("%1:%2").arg(winner).arg(all_roles.join("+")));

    if(QThread::currentThread() == thread)
        thread->end();
    else
        sem->release();
}

void Room::slashEffect(const SlashEffectStruct &effect){
    QVariant data = QVariant::fromValue(effect);

    setEmotion(effect.from, Killer);
    setEmotion(effect.to, Victim);

    bool broken = thread->trigger(SlashEffect, effect.from, data);
    if(!broken)
        thread->trigger(SlashEffected, effect.to, data);
}

void Room::slashResult(const SlashResultStruct &result){
    QVariant data = QVariant::fromValue(result);
    thread->trigger(SlashResult, result.from, data);
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

void Room::promptUser(ServerPlayer *to, const QString &prompt_str){
    to->invoke("prompt", prompt_str);
}

bool Room::askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data){
    bool invoked;
    AI *ai = player->getAI();
    if(ai)
        invoked = ai->askForSkillInvoke(skill_name, data);
    else{
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
            return skill->getDefaultChoice();
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
    moveCardTo(card_id, target, Player::Hand, true);
}

bool Room::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to){
    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(!player->hasNullification())
            continue;

        int card_id = -1;

        trust:
        AI *ai = player->getAI();
        if(ai){
            card_id = ai->askForNullification(trick_name, from, to);
        }else{
            QString ask_str = QString("%1:%2->%3")
                              .arg(trick_name)
                              .arg(from ? from->objectName() : ".")
                              .arg(to->objectName());

            player->invoke("askForNullification", ask_str);
            getResult("replyNullificationCommand", player, false);

            if(result.isEmpty())
                goto trust;

            card_id = result.toInt();
        }

        if(card_id == -1)
            continue;

        throwCard(card_id);

        CardStar card = Sanguosha->getCard(card_id);
        if(!card->inherits("Nullification")){
            Card *vcard = new Nullification(card->getSuit(), card->getNumber());
            vcard->addSubcard(card_id);
            vcard->setSkillName("kanpo");
            card = vcard;
        }

        LogMessage log;
        log.type = "#UseCard";
        log.from = player;
        log.card_str = card->toString();
        sendLog(log);

        LogMessage log2;
        log2.type = "#NullificationDetails";
        log2.from = from;
        log2.to << to;
        log2.arg = trick_name;
        sendLog(log2);

        playCardEffect("nullification", player->getGeneral()->isMale());

        QVariant data = QVariant::fromValue(card);
        thread->trigger(CardResponsed, player, data);

        if(card->isVirtualCard())
            delete card;

        return !askForNullification("nullification", player, to);
    }

    return false;
}

int Room::askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason){
    int card_id;

    AI *ai = player->getAI();
    if(ai){
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
        card_id = who->getRandomHandCard();

    return card_id;
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt){
    const Card *card = NULL;

    QVariant asked = pattern;
    thread->trigger(CardAsked, player, asked);
    if(provided){
        card = provided;
        provided = NULL;
    }else{
        AI *ai = player->getAI();
        if(ai)
            card = ai->askForCard(pattern);
        else{
            player->invoke("askForCard", QString("%1:%2").arg(pattern).arg(prompt));
            getResult("responseCardCommand", player);

            if(result.isEmpty())
                return askForCard(player, pattern, prompt);

            if(result != ".")
                card = Card::Parse(result);
        }
    }

    if(card){
        throwCard(card);

        if(!card->inherits("DummyCard")){
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
    if(ai)
        answer = ai->askForUseCard(pattern, prompt);
    else{
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

int Room::askForAG(ServerPlayer *player, const QList<int> &card_ids){
    if(card_ids.length() == 1)
        return card_ids.first();

    int card_id;

    AI *ai = player->getAI();
    if(ai)
        card_id = ai->askForAG(card_ids);
    else{
        player->invoke("askForAG");
        getResult("chooseAGCommand", player);

        if(result.isEmpty())
            return askForAG(player, card_ids);

        card_id = result.toInt();
    }

    return card_id;
}

int Room::askForCardShow(ServerPlayer *player, ServerPlayer *requestor){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForCardShow(requestor);

    player->invoke("askForCardShow", requestor->getGeneralName());
    getResult("responseCardCommand", player);

    if(result.isEmpty())
        return askForCardShow(player, requestor);
    else if(result == ".")
        return player->getRandomHandCard();

    return result.toInt();
}

int Room::askForPeaches(ServerPlayer *dying, int peaches){
    LogMessage log;
    log.type = "#AskForPeaches";
    log.from = dying;
    log.arg = QString::number(peaches);
    sendLog(log);

    QList<ServerPlayer *> players;
    if(current->hasSkill("wansha") && current->isAlive()){
        players << current;
        if(dying != current)
            players << dying;
    }else
        players = getAllPlayers();

    int got = 0;
    foreach(ServerPlayer *player, players){
        got += askForPeach(player, dying, peaches);      
        if(got >= peaches)
            return got;
    }

    return got;
}

int Room::askForPeach(ServerPlayer *player, ServerPlayer *dying, int peaches){
    int got = 0;
    while(peaches > 0){
        bool provided = askForSinglePeach(player, dying, peaches);
        if(provided){
            got ++;
            peaches --;

            // jiuyuan process
            if(dying->hasSkill("jiuyuan") && player != dying  && player->getKingdom() == "wu"){
                bool can_invoke = true;
                if(!dying->isLord())
                    can_invoke = dying->hasSkill("guixin2");

                if(can_invoke){
                    playSkillEffect("jiuyuan");

                    got ++;
                    peaches --;
                }
            }
        }else
            break;
    }

    return got;
}

bool Room::askForSinglePeach(ServerPlayer *player, ServerPlayer *dying, int peaches){
    const Card *peach = NULL;

    if(player->hasSkill("jijiu")){
        ServerPlayer *huatuo = player;
        if(huatuo->isKongcheng()){
            QList<const Card *> equips;
            equips << huatuo->getWeapon() << huatuo->getArmor()
                    << huatuo->getDefensiveHorse() << huatuo->getOffensiveHorse();
            bool has_red = false;
            foreach(const Card *equip, equips){
                if(equip && equip->isRed()){
                    has_red = true;
                    break;
                }
            }

            if(!has_red)
                return false;
        }
    }else{
        if(player->isKongcheng())
            return false;
    }

    AI *ai = player->getAI();
    if(ai)
        peach = ai->askForSinglePeach(dying);
    else{
        player->invoke("askForSinglePeach", QString("%1:%2").arg(dying->objectName()).arg(peaches));
        getResult("responseCardCommand", player);

        if(result.isEmpty())
            return askForSinglePeach(player, dying, peaches);

        if(result == ".")
            return false;

        peach = Card::Parse(result);
    }

    if(!peach)
        return false;

    throwCard(peach);
    playCardEffect("peach", player->getGeneral()->isMale());

    LogMessage log;
    log.type = "#Peach";
    log.card_str = peach->toString();
    log.from = player;
    sendLog(log);

    return true;
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

void Room::setPlayerCorrect(ServerPlayer *player, const QString &correct_str){
    player->setCorrect(correct_str);
    broadcastProperty(player, "correct", correct_str);
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

void Room::broadcast(const QString &message, ServerPlayer *except){
    foreach(ServerPlayer *player, players){
        if(player != except){
            player->unicast(message);
        }
    }
}

int Room::drawCard(){
    if(draw_pile->isEmpty()){
        if(discard_pile->isEmpty()){
            // the standoff
            gameOver(".");
            return 0;
        }

        qSwap(draw_pile, discard_pile);

        broadcastInvoke("clearPile");
        broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

        qShuffle(*draw_pile);

        foreach(int card_id, *draw_pile){
            setCardMapping(card_id, NULL, Player::DrawPile);
        }
    }
    return draw_pile->takeFirst();
}

void Room::timerEvent(QTimerEvent *event){
    if(left_seconds > 0){
        left_seconds --;
        broadcastInvoke("startInXs", QString::number(left_seconds));
    }else{
        killTimer(event->timerId());
        assignRoles();
    }
}

void Room::reportDisconnection(){
    ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());

    if(player == NULL)
        return;

    // send disconnection message to server log
    emit room_message(player->reportHeader() + tr("disconnected"));

    // the 3 kinds of circumstances
    // 1. Just connected, with no object name : just remove it from player list
    // 2. Connected, with an object name : remove it, tell other clients and decrease signup_count
    // 3. Game is not started, but role is assigned, give it the default general and others same with fourth case
    // 4. Game is started, do not remove it just set its state as offline, and nullify the socket
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
        if(!game_started && player->getGeneral() == NULL){
            // third case
            QString default_general = player->property("default_general").toString();
            if(default_general.isEmpty())
                default_general = "lumeng";
            chooseCommand(player, default_general);
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
        player->invoke("gameOverWarn");
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

void Room::signupCommand(ServerPlayer *player, const QString &arg){
    QStringList words = arg.split(":");
    QString base64 = words[0];
    QString avatar = words[1];

    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    QString screen_name = QString::fromUtf8(data);

    player->setObjectName(generatePlayerName());
    player->setScreenName(screen_name);
    player->setProperty("avatar", avatar);

    player->invoke("checkVersion", Sanguosha->getVersion());

    int timeout = Config.OperationNoLimit ? 0 : Config.OperationTimeout;
    QString flags;
    if(Config.FreeChoose)
        flags.append("F");

    player->invoke("setup", QString("%1:%2:%3:%4")
                   .arg(player_count).arg(timeout)
                   .arg(Sanguosha->getBanPackages().join("+"))
                   .arg(flags));

    // introduce the new joined player to existing players except himself
    player->sendProperty("objectName");
    broadcastInvoke("addPlayer", QString("%1:%2:%3").arg(player->objectName()).arg(base64).arg(avatar), player);

    // introduce all existing player to the new joined
    foreach(ServerPlayer *p, players){
        if(p == player)
            continue;

        QString name = p->objectName();
        QString base64 = p->screenName().toUtf8().toBase64();
        QString avatar = p->property("avatar").toString();

        player->invoke("addPlayer", QString("%1:%2:%3").arg(name).arg(base64).arg(avatar));
    }

    signup_count ++;
    if(isFull()){
        broadcastInvoke("startInXs", QString::number(left_seconds));
        startTimer(1000);
    }
}

void Room::assignRoles(){
    static const char *role_assign_table[] = {
        "",
        "",

        "ZF", // 2
        "ZFN", // 3
        "ZNFF", // 4
        "ZCFFN", // 5
        "ZCFFFN", // 6
        "ZCCFFFN", // 7
        "ZCCFFFFN", // 8
    };

    int n = players.count(), i;

    char roles[100];
    qstrcpy(roles, role_assign_table[n]);

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;

        qSwap(roles[r1], roles[r2]);
    }

    int lord_index = -1;
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
        if(role == "lord"){
            lord_index = i;
            broadcastProperty(player, "role", "lord");

            QStringList lord_list = Sanguosha->getRandomLords();
            player->setProperty("default_general", lord_list.first());
            player->invoke("doChooseGeneral", lord_list.join("+"));
        }else
            player->sendProperty("role");
    }

    Q_ASSERT(lord_index != -1);
    players.swap(0, lord_index);

    for(i=0; i<players.length(); i++)
        players.at(i)->setSeat(i+1);

    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach(ServerPlayer *player, players){
        player_circle << player->objectName();
    }
    broadcastInvoke("arrangeSeats", player_circle.join("+"));
}

void Room::chooseCommand(ServerPlayer *player, const QString &general_name){
    player->setGeneralName(general_name);

    if(player->getRole() == "lord"){
        broadcastProperty(player, "general");

        static const int max_choice = 5;
        const int total = Sanguosha->getGeneralCount();
        const int choice_count = qMin(max_choice, (total-1) / (player_count-1));
        QStringList general_list = Sanguosha->getRandomGenerals((player_count-1) * choice_count + 1);
        general_list.removeOne(general_name);

        int i,j;
        for(i=1; i<player_count; i++){
            QStringList choices;
            for(j=0; j<choice_count; j++)
                choices << general_list[(i-1)*choice_count + j];

            players[i]->setProperty("default_general", choices.first());
            players[i]->invoke("doChooseGeneral", choices.join("+"));
        }
    }else
        player->sendProperty("general");

    chosen_generals ++;
    if(chosen_generals == player_count){
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
    ServerPlayer *player = card_use.from;

#ifdef QT_NO_DEBUG

    CardStar card = card_use.card;
    bool can_use = false;
    if(card->subcardsLength() == 0 && card->isVirtualCard()){
        can_use = true;
    }else
        can_use = getCardOwner(card) == player;

    if(!can_use){
        if(card->isVirtualCard()){
            QList<int> card_ids = card->getSubcards();
            foreach(int card_id, card_ids){
                broadcastInvoke("moveCard", QString("%1:%2@hand->_@_").arg(card_id).arg(player->objectName()));
            }
        }else{
            int card_id = card->getId();
            broadcastInvoke("moveCard", QString("%1:%2@hand->_@_").arg(card_id).arg(player->objectName()));
        }

        return;
    }

#endif

    LogMessage log;
    log.from = player;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString();
    sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    thread->trigger(CardUsed, player, data);

    thread->trigger(CardFinished, player, data);
}

void Room::loseHp(ServerPlayer *victim, int lose){
    int new_hp = victim->getHp() - lose;
    damage(victim, lose);

    if(new_hp <= 0){
        DyingStruct dying;
        dying.damage = NULL;
        dying.peaches = 1 - new_hp;

        QVariant dying_data = QVariant::fromValue(dying);
        thread->trigger(Dying, victim, dying_data);
    }
}

void Room::damage(ServerPlayer *victim, int damage){
    int new_hp = victim->getHp() - damage;
    new_hp = qMax(0, new_hp);

    setPlayerProperty(victim, "hp", new_hp);
    broadcastInvoke("hpChange", QString("%1:%2").arg(victim->objectName()).arg(-damage));
}

void Room::recover(ServerPlayer *player, int recover, bool set_emotion){
    if(player->getLostHp() == 0)
        return;

    int new_hp = qMin(player->getHp() + recover, player->getMaxHP());
    setPlayerProperty(player, "hp", new_hp);
    broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(recover));

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
    if(effect.card->inherits("TrickCard") && askForNullification(effect.card->objectName(), effect.from, effect.to))
        return false;

    directCardEffect(effect);

    return true;
}

void Room::directCardEffect(const CardEffectStruct &effect){
    QVariant data = QVariant::fromValue(effect);
    bool broken = false;
    if(effect.from)
        broken = thread->trigger(CardEffect, effect.from, data);

    if(!broken){
        thread->trigger(CardEffected, effect.to, data);
    }
}

void Room::damage(const DamageStruct &damage_data){
    if(damage_data.to == NULL)
        return;

    if(damage_data.to->isDead())
        return;

    QVariant data = QVariant::fromValue(damage_data);

    // predamage
    bool broken = false;
    if(damage_data.from)
        broken = thread->trigger(Predamage, damage_data.from, data);
    if(broken)
        return;

    // predamaged
    broken = thread->trigger(Predamaged, damage_data.to, data);
    if(broken)
        return;    

    // damage
    if(damage_data.from)
        broken = thread->trigger(Damage, damage_data.from, data);
    if(broken)
        return;

    // damaged
    thread->trigger(Damaged, damage_data.to, data);
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

void Room::startGame(){
    // broadcast all generals except the lord
    int i;
    for(i=1; i<players.count(); i++)
        broadcastProperty(players.at(i), "general");

    alive_players = players;

    // set hp full state
    int lord_welfare = player_count > 4 ? 1 : 0;
    ServerPlayer *the_lord = players.first();
    the_lord->setMaxHP(the_lord->getGeneralMaxHP() + lord_welfare);

    for(i=1; i<player_count; i++)
        players[i]->setMaxHP(players[i]->getGeneralMaxHP());

    foreach(ServerPlayer *player, players){
        player->setHp(player->getMaxHP());

        broadcastProperty(player, "maxhp");
        broadcastProperty(player, "hp");

        // setup AI
        player->setAIByGeneral();
    }

    broadcastInvoke("startGame");
    game_started = true;

    current = the_lord;

    // initialize the place_map and owner_map;
    foreach(int card_id, *draw_pile){
        setCardMapping(card_id, NULL, Player::DrawPile);
    }

    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    sem = new QSemaphore;
    thread = new RoomThread(this);
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
    QStringList cards_str;

    int i;
    for(i=0; i<n; i++){
        int card_id = drawCard();
        const Card *card = Sanguosha->getCard(card_id);
        player->drawCard(card);
        cards_str << QString::number(card_id);

        // update place_map & owner_map
        setCardMapping(card_id, player, Player::Hand);
    }

    player->invoke("drawCards", cards_str.join("+"));
    broadcastInvoke("drawNCards", QString("%1:%2").arg(player->objectName()).arg(n), player);
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
    moveCardTo(card_id, NULL, Player::DiscardedPile, true);
}

RoomThread *Room::getThread() const{
    return thread;
}

void Room::moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open){
    if(!open){
        ServerPlayer *from = getCardOwner(card);

        int n = card->isVirtualCard() ? card->subcardsLength() : 1;
        QString private_move = QString("%1:%2->%3")
                               .arg(n)
                               .arg(from->objectName())
                               .arg(to->objectName());

        foreach(ServerPlayer *player, players){
            if(player != from && player != to)
                player->invoke("moveNCards", private_move);
        }
    }

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards)
            moveCardTo(subcard, to, place, open);
    }else
        moveCardTo(card->getId(), to, place, open);
}

void Room::moveCardTo(int card_id, ServerPlayer *to, Player::Place place, bool open){
    CardMoveStruct move;
    move.card_id = card_id;
    move.from = getCardOwner(card_id);
    move.from_place = getCardPlace(card_id);
    move.to = to;
    move.to_place = place;

    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from)
        move.from->removeCard(card, move.from_place);
    else{
        if(place == Player::DiscardedPile)
            discard_pile->removeOne(move.card_id);
        else if(place == Player::DrawPile)
            draw_pile->removeOne(move.card_id);
    }

    if(move.to){
        move.to->addCard(card, move.to_place);
    }else{
        if(place == Player::DiscardedPile)
            discard_pile->prepend(move.card_id);
        else if(place == Player::DrawPile)
            draw_pile->prepend(move.card_id);
    }
    setCardMapping(move.card_id, move.to, move.to_place);

    QString public_move = move.toString();
    // tell the clients
    if(open){
        broadcastInvoke("moveCard", public_move);
    }else{
        move.from->invoke("moveCard", public_move);
        if(move.to && move.to != move.from)
            move.to->invoke("moveCard", public_move);
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

void Room::setEmotion(ServerPlayer *target, TargetType type){
    QString emotion;
    switch(type){
    case Killer: emotion = "killer"; break;
    case Victim: emotion = "victim"; break;
    case DuelA: emotion = "duel-a"; break;
    case DuelB: emotion = "duel-b"; break;
    case Good: emotion = "good"; break;
    case Bad: emotion = "bad"; break;
    case Normal: emotion = "normal"; break;
    case Recover: emotion = "recover"; break;
    case DrawCard: emotion = "draw-card"; break;
    }

    broadcastInvoke("setEmotion", QString("%1:%2").arg(target->objectName()).arg(emotion), target);
}

void Room::activate(ServerPlayer *player, CardUseStruct &card_use){
    AI *ai = player->getAI();
    if(ai){
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

bool Room::askForDiscard(ServerPlayer *target, int discard_num, bool optional, bool include_equip, Card::Suit suit){
    QList<int> to_discard;

    AI *ai = target->getAI();
    if(ai)
       to_discard = ai->askForDiscard(discard_num, optional, include_equip, suit);
    else{
        QString ask_str = QString::number(discard_num);
        if(optional)
            ask_str.append("o");
        if(include_equip)
            ask_str.append("e");

        switch(suit){
        case Card::Spade: ask_str.append("S"); break;
        case Card::Club: ask_str.append("C"); break;
        case Card::Heart: ask_str.append("H"); break;
        case Card::Diamond: ask_str.append("D"); break;
        default: break;
        }

        target->invoke("askForDiscard", ask_str);
        getResult("discardCardsCommand", target);

        if(result.isEmpty())
            return askForDiscard(target, discard_num, optional, include_equip, suit);

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

    foreach(int card_id, to_discard)
        throwCard(card_id);

    int length = to_discard.length();

    QVariant data = length;
    thread->trigger(CardDiscarded, target, data);

    return true;
}

void Room::setCardMapping(int card_id, ServerPlayer *owner, Player::Place place){
    owner_map.insert(card_id, owner);
    place_map.insert(card_id, place);
}

ServerPlayer *Room::getCardOwner(int card_id) const{
    return owner_map.value(card_id);
}

ServerPlayer *Room::getCardOwner(const Card *card) const{
    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        if(subcards.isEmpty())
            return NULL;
        else
            return getCardOwner(subcards.first());
    }else
        return getCardOwner(card->getId());
}

Player::Place Room::getCardPlace(int card_id) const{
    return place_map.value(card_id);
}

void Room::skip(Player::Phase phase){
    skip_set << phase;
}

bool Room::isSkipped(Player::Phase phase){
    if(skip_set.contains(phase)){
        skip_set.remove(phase);
        return true;
    }else
        return false;
}

ServerPlayer *Room::getLord() const{
    return players.first();
}

void Room::doGuanxing(ServerPlayer *zhuge){
    int n = qMin(5, alive_players.length());

    QList<int> cards = getNCards(n, false);
    QStringList cards_str;
    foreach(int card_id, cards)
        cards_str << QString::number(card_id);

    zhuge->invoke("doGuanxing", cards_str.join("+"));
    getResult("replyGuanxingCommand", zhuge);

    if(result.isEmpty()){
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

    Q_ASSERT(top_list.length() + bottom_list.length() == n);

    LogMessage log;
    log.type = "#GuanxingResult";
    log.from = zhuge;
    log.arg = QString::number(top_list.length());
    log.arg2 = QString::number(bottom_list.length());
    sendLog(log);

    QList<int> top_cards;
    foreach(QString top, top_list)
        top_cards << top.toInt();

    QList<int> bottom_cards;
    foreach(QString bottom, bottom_list)
        bottom_cards << bottom.toInt();

    QListIterator<int> i(top_cards);
    i.toBack();
    while(i.hasPrevious())
        draw_pile->prepend(i.previous());

    i = bottom_cards;
    while(i.hasNext())
        draw_pile->append(i.next());
}

void Room::doGongxin(ServerPlayer *shenlumeng, ServerPlayer *target){
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
        moveCardTo(card_id, NULL, Player::DrawPile, true);
    }
}

const Card *Room::askForPindian(ServerPlayer *player, const QString &ask_str){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForPindian();

    player->invoke("askForPindian", ask_str);
    getResult("responseCardCommand", player);

    if(result.isEmpty())
        return askForPindian(player, ask_str);
    else if(result == "."){
        int card_id = player->getRandomHandCard();
        return Sanguosha->getCard(card_id);
    }else
        return Card::Parse(result);
}

bool Room::pindian(ServerPlayer *source, ServerPlayer *target){
    QString ask_str = QString("%1->%2").arg(source->getGeneralName()).arg(target->getGeneralName());

    LogMessage log;
    log.type = "#Pindian";
    log.from = source;
    log.to << target;
    sendLog(log);

    const Card *card1 = askForPindian(source, ask_str);
    const Card *card2 = askForPindian(target, ask_str);

    throwCard(card1);
    log.type = "$PindianResult";
    log.from = source;
    log.card_str = QString::number(card1->getId());
    sendLog(log);
    thread->delay();    

    throwCard(card2);
    log.type = "$PindianResult";
    log.from = target;
    log.card_str = QString::number(card2->getId());
    sendLog(log);
    thread->delay();

    bool success = card1->getNumber() > card2->getNumber();
    log.type = success ? "#PindianSuccess" : "#PindianFailure";
    log.from = source;
    log.to.clear();
    log.to << target;
    log.card_str.clear();
    sendLog(log);

    if(success)
        setEmotion(source, Good);
    else
        setEmotion(source, Bad);

    return success;
}

ServerPlayer *Room::askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets){
    if(targets.length() == 1)
        return targets.first();

    AI *ai = player->getAI();
    if(ai)
        return ai->askForPlayerChosen(targets);

    QStringList options;
    foreach(ServerPlayer *target, targets)
        options << target->objectName();

    player->invoke("askForPlayerChosen", options.join("+"));

    getResult("choosePlayerCommand", player);

    if(result.isEmpty())
        return askForPlayerChosen(player, targets);

    QString player_name = result;
    if(player_name == ".")
        return NULL;
    else
        return findChild<ServerPlayer *>(player_name);
}

void Room::kickCommand(ServerPlayer *player, const QString &arg){
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

bool Room::askForYiji(ServerPlayer *guojia, QList<int> &cards){
    if(cards.isEmpty())
        return false;

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
        QStringList ids = texts.at(1).split("+");
        ServerPlayer *who = findChild<ServerPlayer *>(texts.at(2));

        DummyCard *dummy_card = new DummyCard;
        foreach(QString id, ids){
            int card_id = id.toInt();
            cards.removeOne(card_id);
            dummy_card->addSubcard(card_id);
        }

        moveCardTo(dummy_card, who, Player::Hand, false);
        delete dummy_card;

        setEmotion(who, DrawCard);

        return true;
    }
}

QString Room::generatePlayerName(){
    static int id = 0;
    id ++;
    return QString("sgs%1").arg(id);
}
