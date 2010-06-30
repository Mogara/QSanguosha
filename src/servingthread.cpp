#include "servingthread.h"

ServingThread::ServingThread(QObject *parent, QTcpSocket *socket)
    :QThread(parent), socket(socket)
{

}

void ServingThread::run()
{
    QString request;
    while(true){
        request = socket->readLine(1024);
        // limit the request line length, I think 1024 is enough for communication

        if(request.isEmpty())
            break;

    }
}
