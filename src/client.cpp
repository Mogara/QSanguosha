#include "client.h"
#include "settings.h"

Client::Client(QObject *parent) :
    QTcpSocket(parent)
{
    connectToHost(Config.HostAddress, Config.Port);

    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));
}

void Client::raiseError(QAbstractSocket::SocketError socket_error){
    // translate error message
    QString reason;
    switch(socket_error){
    case ConnectionRefusedError: reason = tr("Connection was refused or timeout"); break;
    case RemoteHostClosedError: reason = tr("Remote host close this connection"); break;
        // FIXME
    default: reason = tr("Unknow error"); break;
    }

    emit errorMessage(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}
