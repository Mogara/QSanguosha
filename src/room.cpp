#include "room.h"
#include "engine.h"

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

bool Room::event(QEvent *){
    return true;
}

void Room::reportDisconnection(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(socket){
        int index = sockets.indexOf(socket);
        emit room_message(tr("%1[%2:%3] disconnected")
                          .arg("name")
                          .arg(socket->peerAddress().toString())
                          .arg(index+1));
    }
}

void Room::getRequest(){
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(socket && socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the '\n' character
        emit room_message(request);
    }
}
