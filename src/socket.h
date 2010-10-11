#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QTcpSocket>

class ServerSocket: public QObject{
    Q_OBJECT

public:
    ServerSocket(const QString &listen_str);
    virtual bool listen() = 0;

protected:
    QString listen_str;
};

class ClientSocket: public QObject{
    Q_OBJECT

public:
    ClientSocket(const QString &host_str);

    virtual void connectToHost() = 0;
    virtual void disconnectFromHost() = 0;
    virtual void send(const QString &message) = 0;
    virtual bool isConnected() const = 0;

protected:
    QString host_str;

signals:
    void reply_got(char *reply);
    void error_message(const QString &msg);
};

class NativeClientSocket: public ClientSocket{
    Q_OBJECT

public:
    NativeClientSocket(const QString &host_str);

    virtual void connectToHost();
    virtual void disconnectFromHost();
    virtual void send(const QString &message);
    virtual bool isConnected() const;

private slots:
    void emitReplies();
    void raiseError(QAbstractSocket::SocketError socket_error);

private:
    QTcpSocket *socket;
    QString address;
    ushort port;
};

#endif // SOCKET_H
