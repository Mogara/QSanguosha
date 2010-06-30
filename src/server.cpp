#include "server.h"
#include "settings.h"
#include "servingthread.h"

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
    listen(QHostAddress::LocalHost, port);
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();
    ServingThread *thread = new ServingThread(this, socket);
    connect(thread, SIGNAL(thread_message(QString)), SIGNAL(server_message(QString)));

    thread->start();

    emit server_message(tr("%1 connected, port = %2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
}
