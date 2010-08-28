#ifndef SERVER_H
#define SERVER_H

class Room;

#include <QTcpServer>

class Server : public QTcpServer{
    Q_OBJECT

public:
    explicit Server(QObject *parent);

private:
    QList<Room*> rooms;

private slots:
    void processNewConnection();

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
