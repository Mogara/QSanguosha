#include "roomthread.h"

RoomThread::RoomThread(QObject *parent) :
    QThread(parent)
{
}

void RoomThread::resume()
{
    mutex.unlock();
}

void RoomThread::pushEvent(const QScriptValue &event)
{
    events.push(event);
}

void RoomThread::run()
{
    while(!events.isEmpty()){
        QScriptValue event = events.pop();

        // FIXME: process the event
    }
}
