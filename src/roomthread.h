#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

#include <QThread>
#include <QMutex>
#include <QScriptValue>
#include <QTcpSocket>

class RoomThread : public QThread
{
    Q_OBJECT
public:
    explicit RoomThread(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;
    int socketCount() const;

protected:
    virtual void run();

private:
    QMutex mutex;
    QList<QTcpSocket*> sockets;
    int player_count;
};

#endif // ROOMTHREAD_H
