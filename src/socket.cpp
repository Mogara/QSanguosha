#include "socket.h"
#include "settings.h"

#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>

ServerSocket::ServerSocket(const QString &listen_str)
    :listen_str(listen_str)
{

}

ClientSocket::ClientSocket(const QString &host_str)
    :host_str(host_str)
{

}

NativeClientSocket::NativeClientSocket(const QString &host_str)
    :ClientSocket(host_str), socket(NULL), port(0)
{
    QRegExp rx("(.*):(\\d+)");
    if(!rx.exactMatch(host_str))
        return;

    QStringList texts = rx.capturedTexts();
    address = texts.at(1);
    QString port_str = texts.at(2);
    port = port_str.toUShort();

    socket = new QTcpSocket(this);
}

void NativeClientSocket::connectToHost(){
    if(socket){
        socket->connectToHost(address, port);

        socket->connectToHost(Config.HostAddress, Config.Port);
        connect(socket, SIGNAL(readyRead()), this, SLOT(emitReplies()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));
    }
}

typedef char buffer_t[1024];

void NativeClientSocket::emitReplies(){
    while(socket->canReadLine()){
        buffer_t reply;
        socket->readLine(reply, sizeof(reply));

        emit reply_got(reply);
    }
}

void NativeClientSocket::disconnectFromHost(){
    if(socket){
        socket->disconnectFromHost();
    }
}

void NativeClientSocket::send(const QString &message){
    if(socket){
        socket->write(message.toAscii());
        socket->write("\n");
    }
}

bool NativeClientSocket::isConnected() const{
    return socket && socket->state() == QTcpSocket::ConnectedState;
}

void NativeClientSocket::raiseError(QAbstractSocket::SocketError socket_error){
    // translate error message
    QString reason;
    switch(socket_error){
    case QAbstractSocket::ConnectionRefusedError:
        reason = tr("Connection was refused or timeout"); break;
    case QAbstractSocket::RemoteHostClosedError:
        reason = tr("Remote host close this connection"); break;
    case QAbstractSocket::HostNotFoundError:
        reason = tr("Host not found"); break;
    case QAbstractSocket::SocketAccessError:
        reason = tr("Socket access error"); break;
    case QAbstractSocket::NetworkError:
        reason = tr("Server's' firewall blocked the connection or the network cable was plugged out"); break;
        // FIXME
    default: reason = tr("Unknow error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}
