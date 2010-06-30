#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

#include "servingthread.h"

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
    void addServingThread(ServingThread* thread);
    void broadcast(const QString &message);
    void processRequest(const QString &request);

protected:
    virtual void run();

private:
    QMutex mutex;
    QStack<QScriptValue> events;
    QList<ServingThread*> serving_threads;
};

#endif // ROOMTHREAD_H
