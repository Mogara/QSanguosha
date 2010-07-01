#include "roomthread.h"
#include "engine.h"

RoomThread::RoomThread(QObject *parent, int player_count)
    :QThread(parent), player_count(player_count)
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

void RoomThread::addServingThread(ServingThread *thread){
    serving_threads << thread;

    if(serving_threads.length() == player_count){
        // start automatically
        QScriptValue init = Sanguosha->newObject();
        init.setProperty("name", "init");
        pushEvent(init);
        start();
    }
}

void RoomThread::broadcast(const QString &message){
    foreach(ServingThread *thread, serving_threads){
        thread->response(message);
    }
}

void RoomThread::processRequest(const QString &request){

}

bool RoomThread::isFull() const{
    return serving_threads.length() == player_count;
}

void RoomThread::run()
{
    while(!events.isEmpty()){
        QScriptValue event = events.pop();

        // FIXME: process the event
    }
}
