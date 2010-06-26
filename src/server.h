#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class Server : public QTcpServer
{
Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    bool start();

signals:
    void server_message(const QString &);

private slots:
    void processNewConnection();
    void processThreadMessage(const QString &message);
};

#endif // SERVER_H
