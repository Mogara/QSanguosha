#include "roomthread.h"
#include "room.h"

RoomThread::RoomThread(Room *room, QMutex *mutex)
    :QThread(room), mutex(mutex)
{
}


void RoomThread::run(){

}
