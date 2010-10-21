#include "recorder.h"

#include <stdlib.h>

#include <QFile>
#include <QBuffer>

Recorder::Recorder(QObject *parent) :
    QObject(parent)
{
    watch.start();
}

void Recorder::record(char *line)
{
    int elapsed = watch.elapsed();
    data.append(QString("%1 %2").arg(elapsed).arg(line));
}

bool Recorder::save(const QString &filename){
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return file.write(data) != -1;
    }else
        return false;
}

Replayer::Replayer(QObject *parent, const QString &filename)
    :QThread(parent), filename(filename)
{

}

#include "client.h"

void Replayer::run(){
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    typedef char buffer_t[1024];

    int last = 0;
    static int minimum_delay = 2500;

    while(!file.atEnd()){
        buffer_t line;
        memset(line, 0, sizeof(buffer_t));
        file.readLine(line, sizeof(buffer_t));

        char *space = strchr(line, ' ');
        if(space){
            *space = '\0';

            int elapsed = atoi(line);
            int delay = elapsed - last;
            delay = qMin(delay, minimum_delay);
            msleep(delay);
            last = elapsed;

            QString cmd = space + 1;
            emit command_parsed(cmd);
        }
    }

    file.close();
}

