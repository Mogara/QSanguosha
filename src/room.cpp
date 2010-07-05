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
    :QObject(parent), player_count(player_count), focus(NULL)
{
}

void Room::addSocket(QTcpSocket *socket){
    Player *player = new Player(this);
    player->setSocket(socket);
    players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processRequest(QString)));

    QStringList cards;
    int i;
    for(i=0;i<5;i++){
        cards << QString::number(qrand()%104);
    }
    player->unicast("! drawCards " + cards.join("+"));

    if(isFull())
        QTimer::singleShot(5000, this, SLOT(startGame()));    
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

void Room::pushEvent(const QScriptValue &event){
    if(event.isObject()){
        Event *e = new Event(event);
        QCoreApplication::postEvent(this, e);
    }
}

bool Room::event(QEvent *event){
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

void Room::startGame(){
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

    for(i=0; i<n; i++){
        Player *player = players[i];
        player->setRole(roles[i]);
        if(roles[i] == "lord")
            broadcast(QString("#%1 role lord").arg(player->objectName()));
        else
            player->unicast(". role " + roles[i]);
    }
}

void Room::chooseCommand(Player *player, const QStringList &args){

}



