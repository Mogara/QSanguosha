#ifndef SERVINGTHREAD_H
#define SERVINGTHREAD_H

class RoomThread;

#include <QThread>
#include <QTcpSocket>

class ServingThread : public QThread
{
    Q_OBJECT
public:
    explicit ServingThread(QObject *parent, QTcpSocket *socket);
    void setRoomThread(RoomThread *room_thread);
    void response(const QString &message);

protected:
    virtual void run();

private:
    QTcpSocket *socket;
    RoomThread *room_thread;

signals:
    void thread_message(const QString &);
};

#endif // SERVINGTHREAD_H
