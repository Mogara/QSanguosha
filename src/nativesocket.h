#ifndef NATIVESOCKET_H
#define NATIVESOCKET_H

#include "socket.h"

class NativeServerSocket: public ServerSocket{
    Q_OBJECT

public:
    NativeServerSocket();
    virtual bool listen();

private:
    QTcpServer *server;
};


class NativeClientSocket: public ClientSocket{
    Q_OBJECT

public:
    NativeClientSocket();
    NativeClientSocket(QTcpSocket *socket);

    virtual void connectToHost();
    virtual void disconnectFromHost();
    virtual void send(const QString &message);
    virtual bool isConnected() const;
    virtual QString peerName() const;

private slots:
    void emitReplies();
    void raiseError(QAbstractSocket::SocketError socket_error);

private:
    QTcpSocket * const socket;
};

#endif // NATIVESOCKET_H
