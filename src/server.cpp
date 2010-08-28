#include "server.h"
#include "settings.h"
#include "room.h"

#include <QInputDialog>
#include <QNetworkInterface>
#include <QMessageBox>

Server::Server(QObject *parent)
    :QTcpServer(parent)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), SLOT(processNewConnection()));

    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    if(addresses.length() == 1)
        listen(addresses.first(), port);
    else{
        QStringList items;
        foreach(QHostAddress address, addresses){
            quint32 ipv4 = address.toIPv4Address();
            if(ipv4)
                items << QHostAddress(ipv4).toString();
        }

        bool ok;
        int current = items.indexOf(Config.ListenAddress);
        if(current == -1)
            current = 0;

        QString result = QInputDialog::getItem(NULL, tr("Select network address"), tr("Network address"), items, current, false, &ok);
        if(ok){
            Config.ListenAddress = result;
            Config.setValue("ListenAddress", result);
            listen(QHostAddress(result), port);
            if(!isListening())
                QMessageBox::warning(NULL, tr("Warning"), tr("Can not start server on address %1 !").arg(result));
        }
    }
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

    Room *free_room = NULL;
    foreach(Room *room, rooms){
        if(!room->isFull()){
            free_room = room;
            break;
        }
    }

    if(free_room == NULL){
        free_room = new Room(this, Config.PlayerCount);
        rooms << free_room;
        connect(free_room, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    free_room->addSocket(socket);

    emit server_message(tr("%1 connected, port = %2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
}
