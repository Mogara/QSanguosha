#include "server.h"
#include "settings.h"

Server::Server(QObject *parent)
    :QTcpServer(parent), room(NULL)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), SLOT(processNewConnection()));
    listen(QHostAddress::LocalHost, port);
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

    if(room == NULL || room->isFull()){
        room = new Room(this, 8);
        connect(room, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    room->addSocket(socket);

    emit server_message(tr("%1 connected, port = %2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
}
