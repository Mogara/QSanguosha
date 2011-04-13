#include "recorder.h"
#include "client.h"

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
    recordLine(line);
}

void Recorder::recordLine(const QString &line){
    int elapsed = watch.elapsed();
    if(line.endsWith("\n"))
        data.append(QString("%1 %2").arg(elapsed).arg(line));
    else
        data.append(QString("%1 %2\n").arg(elapsed).arg(line));
}

bool Recorder::save(const QString &filename) const{
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

void Replayer::run(){
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    typedef char buffer_t[1024];

    int last = 0;
    QStringList nondelays;
    nondelays << "addPlayer" << "removePlayer" << "speak";

    while(!file.atEnd()){
        buffer_t line;
        memset(line, 0, sizeof(buffer_t));
        file.readLine(line, sizeof(buffer_t));

        char *space = strchr(line, ' ');
        if(space == NULL)
            continue;

        *space = '\0';
        QString cmd = space + 1;

        int elapsed = atoi(line);
        int delay = qMin(elapsed - last, 2500);
        last = elapsed;

        bool delayed = true;
        foreach(QString nondelay, nondelays){
            if(cmd.startsWith(nondelay)){
                delayed = false;
                break;
            }
        }

        if(delayed)
            msleep(delay);
        emit command_parsed(cmd);
    }

    file.close();
}

