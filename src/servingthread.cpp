#include "servingthread.h"

ServingThread::ServingThread(QObject *parent, QTcpSocket *socket)
    :QThread(parent), socket(socket)
{
}

void ServingThread::run()
{
    QString request;
    while(true){
        request = socket->readLine();

    }
}
