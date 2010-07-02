#include "room.h"
#include "engine.h"
#include "event.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QCoreApplication>

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
}

bool Room::isFull() const
{
    return sockets.length() == player_count;
}

void Room::pushEvent(const QScriptValue &event){
    // FIXME
}

bool Room::event(QEvent *event){
    if(event->type() != Sanguosha->getEventType())
        return false;

    // FIXME

    event->accept();
    return true;
}

void Room::reportDisconnection(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(socket){
        reportMessage(socket, tr("disconnected"));
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
    if(socket && socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        QStringList args = request.split(" ");
        QString command = args.front();

        if(command == "set"){
            QString field = args[1];
            QString value = args[2];
            players[socket]->setProperty(field.toAscii(), value);
        }

        reportMessage(socket, request);
    }
}
