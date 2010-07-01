#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>

class Client : public QTcpSocket
{
Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    void signup();

signals:
    void errorMessage(const QString &msg);

private slots:
    void raiseError(QAbstractSocket::SocketError socket_error);
};

#endif // CLIENT_H
