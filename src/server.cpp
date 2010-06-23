#include "server.h"
#include "settings.h"

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
}

void Server::start(){
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
    listen(QHostAddress::Any, port);
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();
}
