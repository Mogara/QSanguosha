#include "server.h"
#include "settings.h"

#include <QInputDialog>
#include <QNetworkInterface>

Server::Server(QObject *parent)
    :QTcpServer(parent)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), SLOT(processNewConnection()));

    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    if(addresses.length() == 1)
        listen(addresses.front(), port);
    else{
        QStringList items;
        foreach(QHostAddress address, addresses){
            quint32 ipv4 = address.toIPv4Address();
            if(ipv4)
                items << QHostAddress(ipv4).toString();
        }

        QString result = QInputDialog::getItem(NULL, tr("Select network address"), tr("Network address"), items);
        listen(QHostAddress(result), port);
    }
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

    Room *free_room = NULL;
    foreach(Room *room, rooms){
        if(!room->isFull())
            free_room = room;
    }

    if(free_room == NULL){
        free_room = new Room(this, 2);
        rooms << free_room;
        connect(free_room, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    free_room->addSocket(socket);

    emit server_message(tr("%1 connected, port = %2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
}
