#include "room.h"
#include "engine.h"
#include "settings.h"
#include "standard.h"

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
    thread(NULL), sem(NULL),
    legatee(NULL), menghuo(NULL), zhurong(NULL),
    provided(NULL)
{
    // init callback table
    callbacks["useCardCommand"] = &Room::commonCommand;
    callbacks["invokeSkillCommand"] = &Room::commonCommand;
    callbacks["replyNullificationCommand"] = &Room::commonCommand;
    callbacks["chooseCardCommand"] = &Room::commonCommand;
    callbacks["responseCardCommand"] = &Room::commonCommand;
    callbacks["discardCardsCommand"] = &Room::commonCommand;
    callbacks["chooseSuitCommand"] = &Room::commonCommand;
    callbacks["chooseAGCommand"] = &Room::commonCommand;
    callbacks["choosePlayerCommand"] = &Room::commonCommand;
    callbacks["selectChoiceCommand"] = &Room::commonCommand;
    callbacks["replyYijiCommand"] = &Room::commonCommand;
    callbacks["replyGuanxingCommand"] = &Room::commonCommand;
    callbacks["replyGongxinCommand"] = &Room::commonCommand;
    callbacks["replyNullificationCommand"] = &Room::commonCommand;

    callbacks["signupCommand"] = &Room::signupCommand;
    callbacks["chooseCommand"] = &Room::chooseCommand;
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
        int index = alive_players.indexOf(next);
        int next_index = (index + 1) % alive_players.length();
        next = alive_players.at(next_index);

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

void Room::obit(ServerPlayer *victim, ServerPlayer *killer){
    victim->setAlive(false);
    broadcastProperty(victim, "alive", "false");

    broadcastProperty(victim, "role");
    broadcastInvoke("killPlayer", victim->objectName());
}

void Room::bury(ServerPlayer *player){
    if(legatee && player != legatee && legatee->isAlive())
        player->leaveTo(legatee);
    else
        player->leaveTo(NULL);

    int index = alive_players.indexOf(player);
    int i;
    for(i=index+1; i<alive_players.length(); i++){
        ServerPlayer *p = alive_players.at(i);
        p->setSeat(p->getSeat() - 1);
        broadcastProperty(p, "seat");
    }

    alive_players.removeOne(player);
}

void Room::setLegatee(ServerPlayer *legatee){
    this->legatee = legatee;
}

const Card *Room::getJudgeCard(ServerPlayer *player){
    int card_id = drawCard();
    throwCard(card_id);

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
            }
        }
    }

    LogMessage log;
    log.type = "$JudgeResult";
    log.from = player;
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

    broadcastInvoke("gameOver", QString("%1:%2").arg(winner).arg(all_roles.join("+")));
    thread->quit();

    game_finished = true;
}

void Room::slashEffect(const SlashEffectStruct &effect){
    QVariant data = QVariant::fromValue(effect);
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

bool Room::askForSkillInvoke(ServerPlayer *player, const QString &skill_name){
    player->invoke("askForSkillInvoke", skill_name);

    reply_func = "invokeSkillCommand";
    reply_player = player;

    sem->acquire();

    // result should be "yes" or "no"
    bool invoked =  result == "yes";
    if(invoked){
        LogMessage log;
        log.type = "#InvokeSkill";
        log.from = player;
        log.arg = skill_name;

        sendLog(log);
    }

    return invoked;
}

QString Room::askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices){
    QString ask_str = QString("%1:%2").arg(skill_name).arg(choices);
    player->invoke("askForChoice", ask_str);

    reply_func = "selectChoiceCommand";
    reply_player = player;

    sem->acquire();

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
    QString ask_str = QString("%1:%2->%3")
                      .arg(trick_name)
                      .arg(from ? from->objectName() : ".")
                      .arg(to->objectName());

    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(!player->hasNullification())
            continue;

        player->invoke("askForNullification", ask_str);

        reply_func = "replyNullificationCommand";
        reply_player = player;

        sem->acquire();

        int card_id = result.toInt();
        if(card_id == -1)
            continue;

        throwCard(card_id);

        CardStar card = Sanguosha->getCard(card_id);
        QVariant data = QVariant::fromValue(card);
        thread->trigger(CardResponsed, player, data);

        return !askForNullification("nullification", player, to);
    }

    return false;
}

int Room::askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason){
    player->invoke("askForCardChosen", QString("%1:%2:%3").arg(who->objectName()).arg(flags).arg(reason));

    reply_func = "chooseCardCommand";
    reply_player = player;

    sem->acquire();

    int card_id = result.toInt();
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
        player->invoke("askForCard", QString("%1:%2").arg(pattern).arg(prompt));

        reply_func = "responseCardCommand";
        reply_player = player;

        sem->acquire();

        if(result != "."){
            card = Card::Parse(result);

            LogMessage log;
            log.card_str = result;
            log.from = player;
            log.type = QString("#%1").arg(card->metaObject()->className());
            sendLog(log);

            player->playCardEffect(card);
            throwCard(card);
        }
    }

    if(card){
        CardStar card_ptr = card;
        QVariant card_star = QVariant::fromValue(card_ptr);
        thread->trigger(CardResponsed, player, card_star);
    }

    return card;
}

bool Room::askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt){
    Q_ASSERT(pattern.startsWith("@@"));

    player->invoke("askForCard", QString("%1:%2").arg(pattern).arg(prompt));

    reply_func = "useCardCommand";
    reply_player = player;

    sem->acquire();

    if(result != "."){
        useCard(player, result);
        return true;
    }else
        return false;
}

int Room::askForAG(ServerPlayer *player){
    player->invoke("askForAG");

    reply_func = "chooseAGCommand";
    reply_player = player;

    sem->acquire();

    return result.toInt();
}

int Room::askForCardShow(ServerPlayer *player, ServerPlayer *requestor){
    player->invoke("askForCardShow", requestor->getGeneralName());

    reply_func = "responseCardCommand";
    reply_player = player;

    sem->acquire();

    return result.toInt();
}

int Room::askForPeaches(ServerPlayer *dying, int peaches){
    QList<ServerPlayer *> players;
    if(current->hasSkill("wansha")){
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
            if(dying->isLord() && dying->hasSkill("jiuyuan")
                && player != dying  && player->getGeneral()->getKingdom() == "wu"){
                playSkillEffect("jiuyuan");

                got ++;
                peaches --;
            }

        }else
            break;
    }

    return got;
}

bool Room::askForSinglePeach(ServerPlayer *player, ServerPlayer *dying, int peaches){
    player->invoke("askForSinglePeach", QString("%1:%2").arg(dying->objectName()).arg(peaches));

    reply_func = "responseCardCommand";
    reply_player = player;

    sem->acquire();

    if(result == ".")
        return false;

    const Card *peach = Card::Parse(result);
    if(!peach)
        return false;

    throwCard(peach);
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

void Room::addSocket(QTcpSocket *socket){
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

void Room::broadcast(const QString &message, ServerPlayer *except){
    foreach(ServerPlayer *player, players){
        if(player != except){
            player->unicast(message);
        }
    }
}

int Room::drawCard(){
    if(draw_pile->isEmpty()){
        Q_ASSERT(!discard_pile->isEmpty());
        qSwap(draw_pile, discard_pile);

        broadcastInvoke("clearPile");
        broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

        int n = draw_pile->count(), i;
        for(i=0; i<n; i++){
            int r1 = qrand() % n;
            int r2 = qrand() % n;
            draw_pile->swap(r1, r2);
        }
    }
    return draw_pile->takeFirst();
}

void Room::timerEvent(QTimerEvent *event){
    if(left_seconds > 0){
        left_seconds --;
        broadcast("! startInXs " + QString::number(left_seconds));
    }else{
        killTimer(event->timerId());
        assignRoles();
    }
}

void Room::reportDisconnection(){
    ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());
    if(player){
        emit room_message(player->reportHeader() + tr("disconnected"));

        if(!player->objectName().isEmpty()){
            if(game_started){
                player->setState("offline");
                broadcast(QString("#%1 state %2").arg(player->objectName()).arg("offline"));
                player->setSocket(NULL);
                return;
            }else
                broadcast("! removePlayer " + player->objectName(), player);
        }

        player->setSocket(NULL);
        players.removeOne(player);
        if(!player->objectName().isEmpty())
            signup_count--;

        delete player;
    }
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

    if(reply_player && reply_player != player){
        player->invoke("focusWarn", player->objectName());
        QString should_be = reply_player->objectName();
        QString instead_of = player->objectName();
        emit room_message(tr("Reply player should be %1 instead of %2").arg(should_be).arg(instead_of));
        return;
    }

    command.append("Command");

    if(!reply_func.isEmpty() && reply_func != command){
        emit room_message(tr("Reply function should be %1 instead of %2").arg(reply_func).arg(command));
        return;
    }

    Callback callback = callbacks.value(command, NULL);
    if(callback){
        (this->*callback)(player, args.at(1));
        emit room_message(player->reportHeader() + request);
    }else
        emit room_message(QString("%1: %2 is not invokable").arg(player->reportHeader()).arg(command));
}

void Room::signupCommand(ServerPlayer *player, const QString &arg){
    QStringList words = arg.split(":");
    QString name = words[0];
    QString avatar = words[1];

    if(findChild<ServerPlayer*>(name)){
        player->invoke("duplicationError");
        return;
    }

    player->setObjectName(name);
    player->setProperty("avatar", avatar);

    player->invoke("setPlayerCount", QString::number(player_count));

    // introduce the new joined player to existing players except himself
    broadcast(QString("! addPlayer %1:%2").arg(name).arg(avatar), player);

    // introduce all existing player to the new joined
    foreach(ServerPlayer *p, players){
        if(p == player)
            continue;

        QString name = p->objectName();
        QString avatar = p->property("avatar").toString();

        player->invoke("addPlayer", QString("%1:%2").arg(name).arg(avatar));
    }

    signup_count ++;
    if(isFull()){
        broadcast(QString("! startInXs %1").arg(left_seconds));        
        startTimer(1000);
    }
}

void Room::assignRoles(){
    static const char *role_assign_table[] = {
        "",
        "",

        "ZF", // 2
        "ZFN", // 3
        "ZCFN", // 4
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
            player->invoke("getGenerals", lord_list.join("+"));
        }else
            player->sendProperty("role");
    }

    Q_ASSERT(lord_index != -1);
    players.swap(0, lord_index);

    for(i=0; i<players.length(); i++)
        players.at(i)->setSeat(i+1);
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

            players[i]->invoke("getGenerals", choices.join("+"));
        }
    }

    chosen_generals ++;
    if(chosen_generals == player_count){
        startGame();
    }
}

void Room::commonCommand(ServerPlayer *player, const QString &arg){
    result = arg;
    sem->release();
}

void Room::useCard(ServerPlayer *player, const QString &arg){
    QStringList words = arg.split("->");
    QString card_str = words.at(0);
    QString target_str = words.at(1);

    const Card *card = Card::Parse(card_str);

    if(card == NULL){
        emit room_message(tr("Card can not parse:\n %1").arg(card_str));
        return;
    }

    CardUseStruct data;
    data.card = card;
    data.from = player;

    if(target_str != "."){
        QStringList target_names = target_str.split("+");
        foreach(QString target_name, target_names)
            data.to << findChild<ServerPlayer *>(target_name);
    }

    LogMessage log;
    log.from = player;
    log.to = data.to;
    log.type = QString("#%1").arg(card->metaObject()->className());
    log.card_str = card_str;
    sendLog(log);

    QVariant vdata = QVariant::fromValue(data);
    thread->trigger(CardUsed, player, vdata);
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

void Room::recover(ServerPlayer *player, int recover){
    int new_hp = player->getHp() + recover;
    if(new_hp > player->getMaxHP())
        return;

    setPlayerProperty(player, "hp", new_hp);
    broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(recover));
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

    LogMessage log;

    if(damage_data.from){
        log.type = "#Damage";
        log.from = damage_data.from;
    }else{
        log.type = "#NoSourceDamage";
    }

    log.to << damage_data.to;
    log.arg = QString::number(damage_data.damage);

    switch(damage_data.nature){
    case DamageStruct::Normal: log.arg2 = "normal_nature"; break;
    case DamageStruct::Fire: log.arg2 = "fire_nature"; break;
    case DamageStruct::Thunder: log.arg2 = "thunder_nature"; break;
    }

    sendLog(log);

    // damage
    if(damage_data.from)
        broken = thread->trigger(Damage, damage_data.from, data);
    if(broken)
        return;

    // damaged
    thread->trigger(Damaged, damage_data.to, data);
}

void Room::startGame(){
    // broadcast all generals except the lord
    int i;
    for(i=1; i<players.count(); i++)
        broadcastProperty(players.at(i), "general");

    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach(ServerPlayer *player, players){
        alive_players << player;
        player_circle << player->objectName();
    }
    broadcastInvoke("arrangeSeats", player_circle.join("+"));

    // set hp full state
    int lord_welfare = player_count > 4 ? 1 : 0;
    ServerPlayer *the_lord = players.first();
    the_lord->setMaxHP(the_lord->getGeneralMaxHP() + lord_welfare);

    for(i=1; i<player_count; i++)
        players[i]->setMaxHP(players[i]->getGeneralMaxHP());

    foreach(ServerPlayer *player, players){
        player->setHp(player->getMaxHP());

        broadcastProperty(player, "max_hp");
        broadcastProperty(player, "hp");
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

    LogMessage log;
    log.type = "#DrawCards";
    log.from = player;
    log.arg = QString::number(n);
    sendLog(log);

    player->invoke("drawCards", cards_str.join("+"));
    broadcast(QString("! drawNCards %1:%2").arg(player->objectName()).arg(n), player);
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
    if(card_id == -1)
        return;

    moveCardTo(card_id, NULL, Player::DiscardedPile, true);
}

RoomThread *Room::getThread() const{
    return thread;
}

void Room::moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open){
    if(!open){
        ServerPlayer *from = getCardOwner(card);

        QString private_move = QString("%1:%2->%3")
                               .arg(card->subcardsLength())
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
        if(move.to)
            discard_pile->removeOne(move.card_id);
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

void Room::broadcastInvoke(const char *method, const QString &arg){
    broadcast(QString("! %1 %2").arg(method).arg(arg));
}

QString Room::activate(ServerPlayer *target){
    broadcastInvoke("activate", target->objectName());

    reply_func = "useCardCommand";
    reply_player = target;

    sem->acquire();

    return result;
}

Card::Suit Room::askForSuit(ServerPlayer *player){
    player->invoke("askForSuit");

    reply_func = "chooseSuitCommand";
    reply_player = player;

    sem->acquire();

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = player;
    log.arg = result;
    sendLog(log);

    if(result == "spade")
        return Card::Spade;
    else if(result == "club")
        return Card::Club;
    else if(result == "heart")
        return Card::Heart;
    else if(result == "diamond")
        return Card::Diamond;
    else
        return Card::NoSuit;
}

bool Room::askForDiscard(ServerPlayer *target, int discard_num, bool optional, bool include_equip, Card::Suit suit){
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

    reply_func = "discardCardsCommand";
    reply_player = target;

    sem->acquire();

    if(result == ".")
        return false;

    QStringList card_strs = result.split("+");   
    foreach(QString card_str, card_strs){
        int card_id = card_str.toInt();
        throwCard(card_id);
    }

    int length = card_strs.length();

    LogMessage log;
    log.type = "#DiscardCards";
    log.from = target;
    log.arg = QString::number(length);
    sendLog(log);

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

void Room::setMenghuo(ServerPlayer *menghuo){
    this->menghuo = menghuo;
}

ServerPlayer *Room::getMenghuo() const{
    return menghuo;
}

void Room::setZhurong(ServerPlayer *zhurong){
    this->zhurong = zhurong;
}

ServerPlayer *Room::getZhurong() const{
    return zhurong;
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

    reply_func = "replyGuanxingCommand";
    reply_player = zhuge;

    sem->acquire();

    QStringList results = result.split(":");

    Q_ASSERT(results.length() == 2);

    QStringList top_list = results.at(0).split("+");
    QStringList bottom_list = results.at(1).split("+");

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

    reply_func = "replyGongxinCommand";
    reply_player = shenlumeng;

    sem->acquire();

    if(result == ".")
        return;

    int card_id = result.toInt();
    QString result = askForChoice(shenlumeng, "gongxin", "discard+put");
    if(result == "discard")
        throwCard(card_id);
    else{
        moveCardTo(card_id, NULL, Player::DrawPile, false);

        // quick-and-dirty
        shenlumeng->invoke("moveCard", QString("%1:%2@hand->_@=").arg(card_id).arg(target->objectName()));

        foreach(ServerPlayer *player, alive_players){
            if(player != shenlumeng && player != target){
                player->invoke("moveCardToDrawPile", target->objectName());
            }
        }
    }
}

const Card *Room::askForPindian(ServerPlayer *player, const QString &ask_str){
    player->invoke("askForPindian", ask_str);

    reply_func = "responseCardCommand";
    reply_player = player;

    sem->acquire();

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
    log.card_str.clear();
    sendLog(log);

    return success;
}

ServerPlayer *Room::askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets){
    QStringList options;
    foreach(ServerPlayer *target, targets)
        options << target->objectName();

    player->invoke("askForPlayerChosen", options.join("+"));

    reply_func = "choosePlayerCommand";
    reply_player = player;

    sem->acquire();

    QString player_name = result;
    if(player_name == ".")
        return NULL;
    else
        return findChild<ServerPlayer *>(player_name);
}

void Room::takeAG(ServerPlayer *player, int card_id){
    QString who = player ? player->objectName() : ".";
    broadcastInvoke("takeAG", QString("%1:%2").arg(who).arg(card_id));
}

void Room::provide(const Card *card){
    Q_ASSERT(provided == NULL);

    provided = card;
}

QList<ServerPlayer *> Room::getLieges(const ServerPlayer *lord) const{
    QString kingdom = lord->getGeneral()->getKingdom();
    QList<ServerPlayer *> lieges;
    foreach(ServerPlayer *player, alive_players){
        if(player != lord && player->getGeneral()->getKingdom() == kingdom)
            lieges << player;
    }

    return lieges;
}

void Room::sendLog(const LogMessage &log){
    if(log.type.isEmpty())
        return;

    broadcastInvoke("log", log.toString());
}

bool Room::askForYiji(ServerPlayer *guojia, QList<int> &cards){
    if(cards.isEmpty())
        return false;

    QStringList card_str;
    foreach(int card_id, cards)
        card_str << QString::number(card_id);

    guojia->invoke("askForYiji", card_str.join("+"));

    reply_func = "replyYijiCommand";
    reply_player = guojia;

    sem->acquire();

    if(result == ".")
        return false;
    else{
        QRegExp rx("(.+)->(\\w+)");
        rx.exactMatch(result);

        QStringList texts = rx.capturedTexts();
        QStringList ids = texts.at(1).split("+");
        ServerPlayer *who = findChild<ServerPlayer *>(texts.at(2));

        foreach(QString id, ids){
            int card_id = id.toInt();
            cards.removeOne(card_id);
            moveCardTo(card_id, who, Player::Hand, false);
        }

        return true;
    }
}

