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
    :QObject(parent), player_count(player_count), current(NULL),
    pile1(Sanguosha->getRandomCards()),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(Config.CountDownSeconds),
    chosen_generals(0), game_started(false), signup_count(0),
    nullificators_count(0),
    thread(NULL), sem(NULL)
{
    // init callback table
    callbacks["useCardCommand"] = &Room::useCardCommand;
    callbacks["invokeSkillCommand"] = &Room::invokeSkillCommand;
    callbacks["replyNullificationCommand"] = &Room::replyNullificationCommand;
    callbacks["chooseCardCommand"] = &Room::chooseCardCommand;
    callbacks["responseCardCommand"] = &Room::responseCardCommand;

    callbacks["signupCommand"] = &Room::signupCommand;
    callbacks["chooseCommand"] = &Room::chooseCommand;
}

ServerPlayer *Room::getCurrent() const{
    return current;
}

int Room::alivePlayerCount() const{
    return alive_players.count();
}

QString Room::askForSkillInvoke(ServerPlayer *player, const QString &ask_str){
    player->invoke("askForSkillInvoke", ask_str);    
    reply_func = "invokeSkillCommand";
    sem->acquire();

    return result;
}

void Room::invokeSkillCommand(ServerPlayer *, const QString &arg){
    result = arg;
    sem->release();
}

void Room::askForNullification(ServerPlayer *, const QVariant &data){
    broadcastInvoke("askForNullification", data.toString());

    int index = alive_players.indexOf(current), i;
    for(i=index; i<alive_players.count(); i++)
        nullificators << alive_players.at(i);
    for(i=0; i<index; i++)
        nullificators << alive_players.at(i);

    Q_ASSERT(nullificators.length() == alive_players.length());

    nullificators_count = alive_players.length();
}

void Room::askForCardChosen(ServerPlayer *player, const QVariant &data){
    player->invoke("askForCardChosen", data.toString());

}

void Room::requestForCard(ServerPlayer *player, const QVariant &data){
    player->invoke("requestForCard", data.toString());

}

void Room::responseCardCommand(ServerPlayer *player, const QString &arg){

}

void Room::setPlayerFlag(ServerPlayer *player, const QString &flag){
    player->setFlags(flag);
    broadcast(QString("#%1 flags %1").arg(flag));
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

    if(current && current != player){
        player->invoke("focusWarn", player->objectName());
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
            player->invoke("getLords", lord_list.join("+"));
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

            players[i]->invoke("getGenerals", QString("%1+%2").arg(general_name).arg(choices.join("+")));
        }
    }

    chosen_generals ++;
    if(chosen_generals == player_count){
        startGame();
    }
}

void Room::useCardCommand(ServerPlayer *player, const QString &arg){
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

    thread->invokePassiveSkills(CardUsed, player, QVariant::fromValue(data));
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

    ServerPlayer *the_lord = players.first();
    the_lord->setPhase(Player::Start);
    broadcastProperty(the_lord, "phase", "start");
    current = the_lord;

    sem = new QSemaphore;
    thread = new RoomThread(this, sem);
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
        cards_str << QString::number(drawCard());
    }

    player->invoke("drawCards", cards_str.join("+"));
    broadcast(QString("! drawNCards %1:%2").arg(player->objectName()).arg(n), player);
}

void Room::throwCard(ServerPlayer *player, const Card *card){
    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards)
            throwCard(player, subcard);
    }else
        throwCard(player, card->getId());
}

void Room::throwCard(ServerPlayer *player, int card_id){
    CardMoveStruct move;
    move.card_id = card_id;
    move.from_place = Player::Hand;
    move.to_place = Player::DiscardedPile;
    move.from = player;
    move.to = NULL;
    move.open = true;

    moveCard(move);
}

QList<int> *Room::getDiscardPile() const{
    return discard_pile;
}

void Room::moveCard(const CardMoveStruct &move){
    broadcast("! moveCard " + move.toString());
    ServerPlayer::MoveCard(move);
}

QString CardMoveStruct::toString() const{
    static QMap<Player::Place, QString> place2str;
    if(place2str.isEmpty()){
        place2str.insert(Player::Hand, "hand");
        place2str.insert(Player::Equip, "equip");
        place2str.insert(Player::DelayedTrick, "delayed_trick");
        place2str.insert(Player::Special, "special");
        place2str.insert(Player::DiscardedPile, "_");
    }

    QString from_str = from ? from->objectName() : "_";
    QString to_str = to ? to->objectName() : "_";

    return QString("%1:%2@%3->%4@%5")
            .arg(card_id)
            .arg(from_str).arg(place2str.value(from_place, "_"))
            .arg(to_str).arg(place2str.value(to_place, "_"));
}

void Room::replyNullificationCommand(ServerPlayer *player, const QString &arg){
    int card_id = arg.toInt();

    if(card_id == -1)
        nullificators.removeOne(player);
    nullificators_count --;

    if(nullificators_count == 0){

        if(nullificators.isEmpty()){
            // execute the trick
        }else{
            ServerPlayer *nullificator = nullificators.first();
            throwCard(nullificator, card_id);

            // FIXME
        }
    }
}

void Room::chooseCardCommand(ServerPlayer *player, const QString &arg){
    // FIXME
}

void Room::nextPhase(ServerPlayer *player){
    Player::Phase next_phase = player->getNextPhase();

    if(next_phase == Player::NotActive){
        int index = alive_players.indexOf(player);
        int next_index = (index + 1) % alive_players.length();
        ServerPlayer *next = alive_players.at(next_index);
        // FIXME: if face is down, turn over it

        player->setPhase(Player::NotActive);
        next->setPhase(Player::Start);
        current = next;

        broadcastProperty(player, "phase", "not_active");
        broadcastProperty(next, "phase", "start");

        changePhase(next);
    }else{
        player->setPhase(next_phase);        
        broadcastProperty(player, "phase", player->getPhaseString());

        changePhase(player);
    }
}

void Room::playSkillEffect(const QString &skill_name, int index){
    broadcastInvoke("playSkillEffect", QString("%1:%2").arg(skill_name).arg(index));
}

void Room::changePhase(ServerPlayer *target){
    thread->invokePassiveSkills(PhaseChange, target);
}

void Room::broadcastInvoke(const char *method, const QString &arg){
    broadcast(QString("! %1 %2").arg(method).arg(arg));
}

QString Room::activate(ServerPlayer *target){
    broadcastInvoke("activate", target->objectName());
    reply_func = "useCardCommand";

    sem->acquire();

    return result;
}
