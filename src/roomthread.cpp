#include "roomthread.h"
#include "engine.h"

RoomThread::RoomThread(QObject *parent, int player_count)
    :QThread(parent), player_count(player_count)
{
}

void RoomThread::addSocket(QTcpSocket *socket){
    sockets << socket;

    socket->write((QString::number(socketCount())+"\n").toAscii());
}

bool RoomThread::isFull() const
{
    return sockets.length() == player_count;
}

int RoomThread::socketCount() const
{
    return sockets.length();
}

void RoomThread::run()
{
}
