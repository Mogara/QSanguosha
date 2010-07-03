#include "room.h"
#include "engine.h"
#include "event.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QCoreApplication>
#include <QScriptValueIterator>

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count)
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
        sockets.removeOne(socket);
        Player *player = players[socket];
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

        if(command == "set"){
            QString field = args[1];
            QString value = args[2];
            player->setProperty(field.toAscii(), value);
        }else if(command == "signup"){
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

        reportMessage(socket, request);
    }
}




