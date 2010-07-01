#ifndef SERVER_H
#define SERVER_H

#include "roomthread.h"

#include <QTcpServer>

class Server : public QTcpServer
{
Q_OBJECT
public:
    explicit Server(QObject *parent);

signals:
    void server_message(const QString &);

private:
    RoomThread *room_thread;

private slots:
    void processNewConnection();
};

#endif // SERVER_H
