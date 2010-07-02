#ifndef SERVER_H
#define SERVER_H

#include "room.h"

#include <QTcpServer>

class Server : public QTcpServer
{
Q_OBJECT
public:
    explicit Server(QObject *parent);

signals:
    void server_message(const QString &);

private:
    Room *room;

private slots:
    void processNewConnection();
};

#endif // SERVER_H
