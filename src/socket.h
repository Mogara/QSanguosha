#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

class ClientSocket;

class ServerSocket: public QObject{
    Q_OBJECT

public:
    virtual bool listen() = 0;

signals:
    void new_connection(ClientSocket *connection);
};

class ClientSocket: public QObject{
    Q_OBJECT

public:
    virtual void connectToHost() = 0;
    virtual void disconnectFromHost() = 0;
    virtual void send(const QString &message) = 0;
    virtual bool isConnected() const = 0;
    virtual QString peerName() const = 0;

signals:
    void message_got(char *msg);
    void error_message(const QString &msg);
    void disconnected();
};

#endif // SOCKET_H
