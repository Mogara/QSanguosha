#include "server.h"
#include "settings.h"

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
}

bool Server::start(){
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
    return listen(QHostAddress::LocalHost, port);
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

}
