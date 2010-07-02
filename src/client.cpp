#include "client.h"
#include "settings.h"

Client::Client(QObject *parent)
    :QTcpSocket(parent), seat_no(0)
{
    connectToHost(Config.HostAddress, Config.Port);

    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));
}

void Client::request(const QString &message){
    write(message.toAscii());
    write("\n");
    waitForBytesWritten();
}

void Client::setField(const QString &key, const QString &value){
    request(QString("set %1 %2").arg(key).arg(value));
}

void Client::signup(){
    setField("name", Config.UserName);
    setField("avatar", Config.UserAvatar);
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
