#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

class ServerSocket: public QObject{
    Q_OBJECT

public:
    virtual bool listen() = 0;
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
    void reply_got(char *reply);
    void error_message(const QString &msg);
    void disconnected();
};

#endif // SOCKET_H
