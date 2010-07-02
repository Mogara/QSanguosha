#include "server.h"
#include "settings.h"

Server::Server(QObject *parent)
    :QTcpServer(parent), room_thread(NULL)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), SLOT(processNewConnection()));
    listen(QHostAddress::LocalHost, port);
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

    if(room_thread == NULL || room_thread->isFull())
        room_thread = new RoomThread(this, 8);

    room_thread->addSocket(socket);
    room_thread->start();

    emit server_message(tr("%1 connected, port = %2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
}
