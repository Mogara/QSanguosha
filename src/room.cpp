#include "room.h"
#include "engine.h"
#include "event.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QCoreApplication>
#include <QScriptValueIterator>
#include <QTimer>

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count), focus(NULL),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(5), chosen_generals(0)
{
    Sanguosha->getRandomCards(pile1);
}

void Room::addSocket(QTcpSocket *socket){
    Player *player = new Player(this);
    player->setSocket(socket);
    players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processRequest(QString)));

    if(isFull()){
        broadcast("! startInXs 5");
        startTimer(1000);
    }
}

bool Room::isFull() const
{
    return players.length() == player_count;
}

void Room::broadcast(const QString &message, Player *except){
    foreach(Player *player, players){
        if(player != except)
            player->unicast(message);
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

void Room::drawCards(QList<int> &cards, int count){
    cards.clear();

    int i;
    for(i=0; i<count; i++)
        cards << drawCard();
}

void Room::pushEvent(const QScriptValue &event){
    if(event.isObject()){
        Event *e = new Event(event);
        QCoreApplication::postEvent(this, e);
    }
}

bool Room::event(QEvent *event){
    QObject::event(event);

    if(event->type() != Sanguosha->getEventType())
        return false;

    Event *e = static_cast<Event*>(event);

    // dump event
    QString dump_str= "{";
    QScriptValueIterator itor(e->getValue());
    while(itor.hasNext()){
        itor.next();
        dump_str.append(QString("%1=%2").arg(itor.name()).arg(itor.value().toString()));

        if(itor.hasNext())
            dump_str.append(", ");
    }
    dump_str.append("}");
    emit room_message(tr("Event: ") + dump_str);

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
    Player *player = qobject_cast<Player*>(sender());
    if(player){
        emit room_message(player->reportHeader() + tr("disconnected"));

        if(!player->objectName().isEmpty()){
            broadcast("! removePlayer " + player->objectName(), player);
        }

        player->setSocket(NULL);
        players.removeOne(player);
        delete player;
    }
}

void Room::processRequest(const QString &request){
    QStringList args = request.split(" ");
    QString command = args.front();
    Player *player = qobject_cast<Player*>(sender());
    if(player == NULL)
        return;

    if(focus && focus != player){
        player->unicast("! focusWarn .");
        return;
    }

    command.append("Command");
    QMetaObject::invokeMethod(this,
                              command.toAscii(),
                              Qt::DirectConnection,
                              Q_ARG(Player *, player),
                              Q_ARG(QStringList, args));

   emit room_message(player->reportHeader() + request);
}

void Room::setCommand(Player *player, const QStringList &args){
    QString field = args[1];
    QString value = args[2];
    player->setProperty(field.toAscii(), value);
}

void Room::signupCommand(Player *player, const QStringList &args){
    QString name = args[1];
    QString avatar = args[2];

    if(findChild<Player*>(name)){
        player->unicast("! duplicationError .");
        return;
    }

    player->setObjectName(name);
    player->setProperty("avatar", avatar);

    // introduce the new joined player to existing players except himself
    broadcast(QString("! addPlayer %1:%2").arg(name).arg(avatar), player);

    // introduce all existing player to the new joined
    foreach(Player *p, players){
        if(p == player)
            continue;

        QString name = p->objectName();
        QString avatar = p->property("avatar").toString();
        player->unicast(QString("! addPlayer %1:%2").arg(name).arg(avatar));
    }
}

void Room::assignRoles(){
    struct assign_table{
        int lords;
        int loyalists;
        int rebels;
        int renegades;
    };

    static struct assign_table role_assign_table[] = {
        {},
        {},

        { 1, 0, 1, 0}, // 2
        { 1, 0, 1, 1}, // 3
        { 1, 1, 1, 1}, // 4
        { 1, 1, 2, 1}, // 5
        { 1, 1, 3, 1}, // 6
        { 1, 2, 3, 1}, // 7
        { 1, 2, 4, 1}, // 8
    };

    int n = players.count();

    struct assign_table *table = &role_assign_table[n];
    QStringList roles;
    int i;
    for(i=0;i<table->lords;i++)
        roles << "lord";
    for(i=0;i<table->loyalists;i++)
        roles << "loyalist";
    for(i=0;i<table->rebels;i++)
        roles << "rebel";
    for(i=0;i<table->renegades;i++)
        roles << "renegade";

    Q_ASSERT(roles.count() == n);

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        roles.swap(r1, r2);
    }

    int lord_index = -1;
    for(i=0; i<n; i++){
        Player *player = players[i];
        player->setRole(roles[i]);
        if(roles[i] == "lord"){
            lord_index = i;
            broadcast(QString("#%1 role lord").arg(player->objectName()));

            QStringList lord_list;
            Sanguosha->getRandomLords(lord_list);            
            player->unicast("! getLords " + lord_list.join("+"));
        }else
            player->unicast(". role " + roles[i]);
    }

    Q_ASSERT(lord_index != -1);
    players.swap(0, lord_index);
}

void Room::chooseCommand(Player *player, const QStringList &args){
    QString general_name = args[1];
    player->setGeneral(general_name);    
    broadcast(QString("#%1 general %2").arg(player->objectName()).arg(general_name));

    if(player->getRole() == "lord"){        
        const int choice_count = 3;
        QStringList general_list;
        Sanguosha->getRandomGenerals(general_list, (player_count-1) * choice_count + 1);
        general_list.removeOne(general_name);

        int i,j;
        for(i=1; i<player_count; i++){
            QStringList choices;
            for(j=0; j<3; j++)
                choices << general_list[(i-1)*3 + j];

            players[i]->unicast(QString("! getGenerals %1+%2").arg(general_name).arg(choices.join("+")));
        }
    }

    chosen_generals ++;
    if(chosen_generals == player_count)
        startGame();
}

void Room::startGame(){
    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach(Player *player, players)
        player_circle << player->objectName();
    broadcast("$ circle " + player_circle.join("+"));

    // every player draw 4 cards and them start from lord
    int i;
    for(i=0; i<player_count; i++){
        QList<int> cards;
        drawCards(cards, 4);

        QStringList cards_str;
        foreach(int card, cards)
            cards_str << QString::number(card);

        players[i]->unicast("! drawCards " + cards_str.join("+"));
    }
}



