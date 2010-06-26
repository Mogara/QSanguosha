#include "server.h"
#include "settings.h"
#include "servingthread.h"

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
    ServingThread *thread = new ServingThread(this, socket);
    connect(thread, SIGNAL(thread_message(QString)), this, SLOT(processThreadMessage(QString)));

    thread->start();

    emit server_message(tr("Start thread"));
}

void Server::processThreadMessage(const QString &message){
    ServingThread *thread = qobject_cast<ServingThread*>(sender());
    if(thread){
        int thread_addr = reinterpret_cast<int>(thread);
        emit server_message(tr("%1: %2").arg(thread_addr).arg(message));
    }
}
