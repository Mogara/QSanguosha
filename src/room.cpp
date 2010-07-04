#include "room.h"
#include "engine.h"
#include "event.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QCoreApplication>
#include <QScriptValueIterator>

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count), focus(NULL)
{
}

void Room::addSocket(QTcpSocket *socket){
    sockets << socket;
    Player *player = new Player(this);
    players[socket] = player;

    connect(socket, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(getRequest()));   

    QStringList cards;
    int i;
    for(i=0;i<5;i++){
        cards << QString::number(qrand()%104);
    }
    unicast(socket, "! drawCards " + cards.join("+"));

    if(isFull())
        startGame();
}

bool Room::isFull() const
{
    return sockets.length() == player_count;
}

void Room::unicast(QTcpSocket *socket, const QString &message){
    socket->write(message.toAscii());
    socket->write("\n");
}

void Room::broadcast(const QString &message, QTcpSocket *except){
    foreach(QTcpSocket *socket, sockets){
        if(socket != except)
            unicast(socket, message);
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
    emit room_message(dump_str);

    event->accept();
    return true;
}

void Room::reportDisconnection(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(socket){
        reportMessage(socket, tr("disconnected"));

        Player *player = players[socket];
        if(!player->objectName().isEmpty()){
            broadcast("! removePlayer " + player->objectName(), socket);
        }

        sockets.removeOne(socket);        
        players.remove(socket);
        delete player;
    }
}

void Room::reportMessage(QTcpSocket *socket, const QString &message){
    int index = sockets.indexOf(socket);
    QString name = players[socket]->objectName();
    if(name.isEmpty())
        name = tr("anonymous");

    emit room_message(QString("%1[%2:%3] %4")
                      .arg(name)
                      .arg(socket->peerAddress().toString())
                      .arg(index+1)
                      .arg(message));
}

void Room::getRequest(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(!socket)
        return;

    while(socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        QStringList args = request.split(" ");
        QString command = args.front();
        Player *player = players[socket];
        if(player == NULL)
            return;

        if(focus && focus != player){
            unicast(socket, "! focusWarn .");
            return;
        }

        command.append("Command");
        QMetaObject::invokeMethod(this,
                                  command.toAscii(),
                                  Qt::DirectConnection,
                                  Q_ARG(QTcpSocket *, socket),
                                  Q_ARG(Player *, player),
                                  Q_ARG(QStringList, args));

        reportMessage(socket, request);
    }
}

void Room::setCommand(QTcpSocket *socket, Player *player, const QStringList &args){
    QString field = args[1];
    QString value = args[2];
    player->setProperty(field.toAscii(), value);
}

void Room::signupCommand(QTcpSocket *socket, Player *player, const QStringList &args){
    QString name = args[1];
    QString avatar = args[2];

    player->setObjectName(name);
    player->setProperty("avatar", avatar);

    // introduce the new joined player to existing players except himself
    broadcast(QString("! addPlayer %1:%2").arg(name).arg(avatar), socket);

    // introduce all existing player to the new joined
    QMapIterator<QTcpSocket*,Player*> itor(players);
    while(itor.hasNext()){
        itor.next();
        Player *to_introduce = itor.value();
        if(to_introduce == player)
            continue;

        QString name = to_introduce->objectName();
        QString avatar = to_introduce->property("avatar").toString();
        unicast(socket, QString("! addPlayer %1:%2").arg(name).arg(avatar));
    }
}

void Room::startGame(){
    struct assign_table{
        int lords;
        int loyalists;
        int rebels;
        int renegades;
    };

    struct assign_table role_assign_table[] = {
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

    int n = sockets.count();

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

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        roles.swap(r1, r2);
    }

    for(i=0; i<n; i++){
        QTcpSocket *socket = sockets[i];
        Player *player = players[socket];
        player->setRole(roles[i]);

        if(roles[i] == "lord")
            broadcast(QString("#%1 role lord").arg(player->objectName()));
        else
            unicast(socket, ". role " + player->getRole());
    }
}



