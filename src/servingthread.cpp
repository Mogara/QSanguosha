#include "servingthread.h"

#include <QState>
#include <QStateMachine>

ServingThread::ServingThread(QObject *parent, QTcpSocket *socket)
    :QThread(parent), socket(socket)
{
    QStateMachine *machine = new QStateMachine(this);
    QState *init = new QState;
    QState *normal = new QState;
    QState *prompt = new QState;

    machine->addState(init);
    machine->addState(normal);
    machine->addState(prompt);

    machine->setInitialState(init);
    machine->start();
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
