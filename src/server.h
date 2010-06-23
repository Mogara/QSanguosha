#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class Server : public QTcpServer
{
Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void start();

signals:

private slots:
    void processNewConnection();
};

#endif // SERVER_H
