#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

class Room;

#include <QThread>
#include <QMutex>
#include <QVariant>

class RoomThread : public QThread
{
Q_OBJECT
public:
    explicit RoomThread(Room *room, QMutex *mutex);

protected:
    virtual void run();

private:
    Room *room;
    QMutex *mutex;
    QVariant data;
};

#endif // ROOMTHREAD_H
