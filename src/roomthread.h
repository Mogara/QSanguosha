#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

#include <QThread>
#include <QStack>
#include <QMutex>
#include <QScriptValue>

class RoomThread : public QThread
{
    Q_OBJECT
public:
    explicit RoomThread(QObject *parent = 0);
    void resume();
    void pushEvent(const QScriptValue &event);

protected:
    virtual void run();

private:
    QMutex mutex;
    QStack<QScriptValue> events;
};

#endif // ROOMTHREAD_H
