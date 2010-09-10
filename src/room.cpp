#include "room.h"
#include "engine.h"
#include "settings.h"


#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count), current(NULL), reply_player(NULL),
    pile1(Sanguosha->getRandomCards()),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(Config.CountDownSeconds),
    chosen_generals(0), game_started(false), game_finished(false), signup_count(0),
    nullificators_count(0),
    thread(NULL), sem(NULL),
    legatee(NULL), menghuo(NULL)
{
    // init callback table
    callbacks["useCardCommand"] = &Room::commonCommand;
    callbacks["invokeSkillCommand"] = &Room::commonCommand;
    callbacks["replyNullificationCommand"] = &Room::commonCommand;
    callbacks["chooseCardCommand"] = &Room::commonCommand;
    callbacks["responseCardCommand"] = &Room::commonCommand;
    callbacks["responseCardWithTargetsCommand"] = &Room::commonCommand;
    callbacks["discardCardsCommand"] = &Room::commonCommand;
    callbacks["chooseSuitCommand"] = &Room::commonCommand;
    callbacks["chooseAGCommand"] = &Room::commonCommand;
    callbacks["selectChoiceCommand"] = &Room::commonCommand;

    callbacks["signupCommand"] = &Room::signupCommand;
    callbacks["chooseCommand"] = &Room::chooseCommand;
    callbacks["replyNullificationCommand"] = &Room::replyNullificationCommand;
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
            broadcastProperty(next, "face_up");
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

int Room::getJudgeCard(ServerPlayer *player){
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
                zhangjiao->obtainCard(card);
                card_id = subcards.first();
            }
        }
    }

    QVariant card_id_data = card_id;
    thread->trigger(JudgeOnEffect, player, card_id_data);

    return card_id;
}

QList<int> Room::getNCard(int n){
    QList<int> card_ids;
    int i;
    for(i=0; i<n; i++){
        card_ids << drawCard();
    }

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
    thread->trigger(SlashEffect, effect.from, data);
}

void Room::slashResult(const SlashResultStruct &result){
    QVariant data = QVariant::fromValue(result);
    thread->trigger(SlashResult, result.from, data);
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
    return result == "yes";
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
    CardMoveStruct move;
    move.card_id = card_id;
    move.from = getCardOwner(card_id);
    move.from_place = getCardPlace(card_id);
    move.to = target;
    move.to_place = Player::Hand;
    move.open = true;

    moveCard(move);
}

bool Room::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to){
    QString ask_str = QString("%1:%2->%3").arg(trick_name).arg(from->objectName()).arg(to->objectName());
    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        player->invoke("askForNullification", ask_str);
    }

    reply_func = "replyNullificationCommand";
    reply_player = NULL; // everyone can reply

    nullificators_count = players.length();
    nullificator = NULL;
    nullificator_map.clear();

    sem->acquire();

    if(nullificator == NULL)
        return false; // nobody nullify the trick
    else{
        int card_id = result.toInt();
        throwCard(card_id);

        foreach(ServerPlayer *player, players){
            bool replyed = nullificator_map.value(player, false);
            if(!replyed)
                player->invoke("closeNullification");
        }

        return !askForNullification("nullification", nullificator, to); // recursive call
    }
}

void Room::replyNullificationCommand(ServerPlayer *player, const QString &arg){
    result = arg;
    int card_id = result.toInt();
    if(card_id == -1){
        nullificators_count--;
        if(nullificators_count == 0)
            sem->release();
    }else{
        nullificator = player;        
        playCardEffect("nullification", player->getGeneral()->isMale());

        sem->release();
    }

    nullificator_map.insert(player, true);
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
    player->invoke("askForCard", QString("%1:%2").arg(pattern).arg(prompt));

    reply_func = "responseCardCommand";
    reply_player = player;

    sem->acquire();

    if(result != "."){
        const Card *card = Card::Parse(result);
        player->playCardEffect(card);
        throwCard(card);
        return card;
    }else
        return NULL;
}

const Card *Room::askForCardWithTargets(ServerPlayer *player, const QString &pattern, const QString &prompt, QList<ServerPlayer *> &targets){
    Q_ASSERT(pattern.startsWith("@@"));

    player->invoke("askForCard", QString("%1:%2").arg(pattern).arg(prompt));

    reply_func = "responseCardWithTargetsCommand";
    reply_player = player;

    sem->acquire();

    if(result != "."){
        QStringList words = result.split("->");
        const Card *card = Card::Parse(words.at(0));
        throwCard(card);

        QStringList target_names = words.at(1).split("+");
        foreach(QString target_name, target_names)
            targets << findChild<ServerPlayer *>(target_name);

        return card;
    }else
        return NULL;
}

int Room::askForAG(ServerPlayer *player){
    player->invoke("askForAG");

    reply_func = "chooseAGCommand";
    reply_player = player;

    sem->acquire();

    return result.toInt();
}

int Room::askForCardShow(ServerPlayer *player){
    player->invoke("askForCardShow");

    reply_func = "showCard";
    reply_player = player;

    sem->acquire();

    return result.toInt();
}

bool Room::askForSave(ServerPlayer *dying, int peaches){
    QList<ServerPlayer *> players;
    if(current->hasSkill("wansha")){
        players << current;
        if(dying != current)
            players << dying;
    }else
        players = getAllPlayers();

    foreach(ServerPlayer *player, players){
        int got = askForPeach(player, dying, peaches);
        peaches -= got;
        if(peaches == 0)
            return true;
    }

    return false;
}

int Room::askForPeach(ServerPlayer *player, ServerPlayer *dying, int peaches){
    int got = 0;
    while(peaches > 0){
        bool provided = askForSinglePeach(player, dying, peaches);
        if(provided){
            got ++;
            peaches --;
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

void Room::setPlayerCorrect(ServerPlayer *player, const QString &field, int correct){
    QString correct_str = QString("%1:%2").arg(field).arg(correct);
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

            QStringList lord_list = Sanguosha->getRandomLords(Config.LordCount);
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

    QVariant vdata = QVariant::fromValue(data);
    thread->trigger(CardUsed, player, vdata);
}

void Room::lostHp(ServerPlayer *victim){
    if(victim->getHp() == 1){
        bool saved = askForSave(victim, 1);
        if(!saved)
            thread->trigger(Death, victim);
    }else{
        victim->setHp(victim->getHp() - 1);
        broadcastProperty(victim, "hp");
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

void Room::cardEffect(const CardEffectStruct &effect){
    if(effect.card->inherits("TrickCard") && askForNullification(effect.card->objectName(), effect.from, effect.to))
        return;

    QVariant data = QVariant::fromValue(effect);
    bool broken = thread->trigger(CardEffect, effect.from, data);
    if(!broken)
        thread->trigger(CardEffected, effect.to, data);
}

void Room::damage(const DamageStruct &damage_data){
    QVariant data = QVariant::fromValue(damage_data);
    bool broken = thread->trigger(Predamage, damage_data.from, data);
    if(!broken)
        broken = thread->trigger(Predamaged, damage_data.to, data);

    if(!broken)
        broken = thread->trigger(Damage, damage_data.from, data);

    if(!broken)
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
    broadcast("! arrangeSeats " + player_circle.join("+"));

    // set hp full state
    int lord_welfare = player_count > 4 ? 1 : 0;
    players.first()->setMaxHP(players.first()->getGeneralMaxHP() + lord_welfare);

    for(i=1; i<player_count; i++)
        players[i]->setMaxHP(players[i]->getGeneralMaxHP());

    foreach(ServerPlayer *player, players){
        player->setHp(player->getMaxHP());

        broadcastProperty(player, "max_hp");
        broadcastProperty(player, "hp");
    }

    broadcast("! startGame .");
    game_started = true;

    current = players.first();

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

    CardMoveStruct move;
    move.card_id = card_id;
    move.from_place = getCardPlace(card_id);
    move.to_place = Player::DiscardedPile;
    move.from = getCardOwner(card_id);
    move.to = NULL;
    move.open = true;

    moveCard(move);
}

RoomThread *Room::getThread() const{
    return thread;
}

void Room::moveCard(const CardMoveStruct &move){
    broadcast("! moveCard " + move.toString());

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
        if(move.from)
            discard_pile->prepend(move.card_id);
    }

    setCardMapping(move.card_id, move.to, move.to_place);

    if(move.from){
        QVariant data = QVariant::fromValue(move);
        thread->trigger(CardMove, move.from, data);
    }

    card->onMove(move);
}

void Room::moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open){
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
    move.open = open;

    moveCard(move);
}

QString CardMoveStruct::toString() const{
    static QMap<Player::Place, QString> place2str;
    if(place2str.isEmpty()){
        place2str.insert(Player::Hand, "hand");
        place2str.insert(Player::Equip, "equip");
        place2str.insert(Player::DelayedTrick, "delayed_trick");
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

bool Room::askForDiscard(ServerPlayer *target, int discard_num){
    target->invoke("askForDiscard", QString::number(discard_num));

    reply_func = "discardCardsCommand";
    reply_player = target;

    sem->acquire();

    QStringList card_strs = result.split("+");   
    foreach(QString card_str, card_strs){
        int card_id = card_str.toInt();
        throwCard(card_id);
    }

    return !card_strs.isEmpty();
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

void Room::setMenghuo(ServerPlayer *menghuo){
    this->menghuo = menghuo;
}

ServerPlayer *Room::getMenghuo() const{
    return menghuo;
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
