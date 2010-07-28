#include "room.h"
#include "engine.h"
#include "event.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QCoreApplication>
#include <QTimer>

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count), focus(NULL),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(5),
    chosen_generals(0), game_started(false), signup_count(0)
{
    Sanguosha->getRandomCards(pile1);
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

void Room::broadcast(const QString &message, Player *except){
    foreach(Player *player, players){
        if(player != except){
            ServerPlayer *server_player = qobject_cast<ServerPlayer*>(player);
            server_player->unicast(message);
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

bool Room::event(QEvent *event){
    QObject::event(event);

    if(event->type() != Sanguosha->getEventType())
        return false;

    //Event *e = static_cast<Event*>(event);
    event->accept();

    return true;
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

    if(focus && focus != player){
        player->unicast("! focusWarn " + player->objectName());
        return;
    }

    command.append("Command");
    QMetaObject::invokeMethod(this,
                              command.toAscii(),
                              Qt::DirectConnection,
                              Q_ARG(ServerPlayer *, player),
                              Q_ARG(QStringList, args));

   emit room_message(player->reportHeader() + request);
}

void Room::setCommand(ServerPlayer *player, const QStringList &args){
    QString field = args[1];
    QString value = args[2];
    player->setProperty(field.toAscii(), value);
}

void Room::signupCommand(ServerPlayer *player, const QStringList &args){
    QString name = args[1];
    QString avatar = args[2];

    if(findChild<ServerPlayer*>(name)){
        player->unicast("! duplicationError .");
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
        player->unicast(QString("! addPlayer %1:%2").arg(name).arg(avatar));
    }

    signup_count ++;
    if(isFull()){
        broadcast("! startInXs 5");
        game_started = true;
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
    strcpy(roles, role_assign_table[n]);

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
            broadcast(QString("#%1 role lord").arg(player->objectName()));

            QStringList lord_list;
            Sanguosha->getRandomLords(lord_list);            
            player->unicast("! getLords " + lord_list.join("+"));
        }else
            player->unicast(". role " + role);
    }

    Q_ASSERT(lord_index != -1);
    players.swap(0, lord_index);
}

void Room::chooseCommand(ServerPlayer *player, const QStringList &args){
    QString general_name = args[1];
    player->setGeneral(general_name);    
    broadcast(QString("#%1 general %2").arg(player->objectName()).arg(general_name));

    if(player->getRole() == "lord"){
        static const int max_choice = 5;
        const int total = Sanguosha->getGeneralCount();
        const int choice_count = qMin(max_choice, (total-1) / (player_count-1));
        QStringList general_list;
        Sanguosha->getRandomGenerals(general_list, (player_count-1) * choice_count + 1);
        general_list.removeOne(general_name);

        int i,j;
        for(i=1; i<player_count; i++){
            QStringList choices;
            for(j=0; j<choice_count; j++)
                choices << general_list[(i-1)*choice_count + j];

            players[i]->unicast(QString("! getGenerals %1+%2").arg(general_name).arg(choices.join("+")));
        }
    }

    chosen_generals ++;
    if(chosen_generals == player_count)
        startGame();
}

void Room::useCardCommand(ServerPlayer *player, const QStringList &args){
    QString card_str = args.at(1);
    const Card *card = NULL;
    if(card_str.contains(QChar('=')))
        card = Card::Parse(card_str);
    else
        card = Sanguosha->getCard(card_str.toInt());

    if(!card)
        return;

    QStringList target_names = args.at(2).split("+");

    const QString name = player->objectName();
    if(card->getType() == "equip"){
        const Card *equip = card;
        const Card *uninstalled = player->replaceEquip(equip);
        if(uninstalled){
            broadcast(QString("! moveCard %1:%2@equip->_").arg(uninstalled->getID()).arg(name));
            discard_pile->append(uninstalled->getID());
        }
        broadcast(QString("! moveCard %1:%2@hand->%2@equip").arg(equip->getID()).arg(name));
        player->removeCard(card, "hand");
    }else{      
        broadcast(QString("! moveCard %1:%2@hand->_").arg(card->getID()).arg(name));
        discard_pile->append(card->getID());
    }
}

void Room::startGame(){
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

    int i;
    for(i=1; i<player_count; i++)
        players[i]->setMaxHP(players[i]->getGeneralMaxHP());

    foreach(Player *player, players){
        player->setHp(player->getMaxHP());

        broadcast(QString("#%1 max_hp %2").arg(player->objectName()).arg(player->getMaxHP()));
        broadcast(QString("#%1 hp %2").arg(player->objectName()).arg(player->getHp()));
    }

    // every player draw 4 cards and them start from the lord
    for(i=0; i<player_count; i++){
        drawCards(players.at(i), 4);
    }

    ServerPlayer *the_lord = players.first();
    the_lord->setPhase(Player::Start);
    broadcast(QString("#%1 phase start").arg(the_lord->objectName()));
    broadcast("! startGame " + the_lord->objectName());
    active_records.push(the_lord);
}

void Room::endPhaseCommand(ServerPlayer *player, const QStringList &){
    if(player->getPhase() == Player::NotActive){
        qFatal(qPrintable(tr("Player that is not active should not use endPhase command!")));
        return;
    }

    Player::Phase next_phase = player->getNextPhase();    

    if(next_phase == Player::NotActive){
        int index = alive_players.indexOf(player);
        int next_index = (index + 1) % alive_players.length();
        ServerPlayer *next = alive_players.at(next_index);
        // FIXME: if face is down, turn over it

        player->setPhase(Player::NotActive);
        next->setPhase(Player::Start);

        broadcast(QString("#%1 phase not_active").arg(player->objectName()));
        broadcast(QString("#%1 phase start").arg(next->objectName()));

        broadcast(QString("! activate %1:PhaseChange:").arg(next->objectName()));
    }else{
        player->setPhase(next_phase);

        broadcast(QString("#%1 phase %2").arg(player->objectName()).arg(player->getPhaseString()));
        broadcast(QString("! activate %1:PhaseChange:").arg(player->objectName()));
    }
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

    player->unicast("! drawCards " + cards_str.join("+"));
    broadcast(QString("! drawNCards %1:%2").arg(player->objectName()).arg(n), player);
}

void Room::drawCardsCommand(ServerPlayer *player, const QStringList &args){
    int n = args.at(1).toInt();
    drawCards(player, n);
}

void Room::judgeCommand(ServerPlayer *player, const QStringList &args){
    QString target = args.at(1);
    int card_id = drawCard();
    discard_pile->append(card_id);

    broadcast(QString("! judge %1:%2").arg(target).arg(card_id));
}
