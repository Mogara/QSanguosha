#ifndef CLIENT_H
#define CLIENT_H

#include "player.h"

#include <QTcpSocket>

class Client : public QTcpSocket
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    void request(const QString &message);
    void sendField(const QString &field);
    void signup();

private:
    QList<Player*> players;
    QObject *room;

private slots:
    void update();
    void raiseError(QAbstractSocket::SocketError socket_error);

signals:
    void error_message(const QString &msg);
};

#endif // CLIENT_H
