#include "recorder.h"
#include "client.h"

#include <stdlib.h>

#include <QFile>
#include <QBuffer>

Recorder::Recorder(QObject *parent)
    :QObject(parent)
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
    :QThread(parent), filename(filename), speed(1.0), playing(true)
{
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    typedef char buffer_t[1024];

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

        Pair pair;
        pair.elapsed = elapsed;
        pair.cmd = cmd;

        pairs << pair;
    }

    file.close();
}

int Replayer::getDuration() const{
    return pairs.last().elapsed / 1000.0;
}

qreal Replayer::getSpeed() {
    qreal speed;
    mutex.lock();
    speed = this->speed;
    mutex.unlock();
    return speed;
}

void Replayer::uniform(){
    mutex.lock();

    if(speed != 1.0){
        speed = 1.0;
        emit speed_changed(1.0);
    }

    mutex.unlock();
}

void Replayer::speedUp(){
    mutex.lock();

    if(speed < 6.0){
        qreal inc = speed >= 2.0 ? 1.0 : 0.5;
        speed += inc;
        emit speed_changed(speed);
    }

    mutex.unlock();
}

void Replayer::slowDown(){
    mutex.lock();

    if(speed >= 1.0){
        qreal dec = speed >= 2.0 ? 1.0 : 0.5;
        speed -= dec;
        emit speed_changed(speed);
    }

    mutex.unlock();
}

void Replayer::toggle(){
    playing = !playing;
    if(playing)
        play_sem.release(); // to play
}

void Replayer::run(){
    int last = 0;

    QStringList nondelays;
    nondelays << "addPlayer" << "removePlayer" << "speak";

    foreach(Pair pair, pairs){
        int delay = qMin(pair.elapsed - last, 2500);
        last = pair.elapsed;

        bool delayed = true;
        foreach(QString nondelay, nondelays){
            if(pair.cmd.startsWith(nondelay)){
                delayed = false;
                break;
            }
        }

        if(delayed){
            delay /= getSpeed();

            msleep(delay);
            emit elasped(pair.elapsed / 1000.0);

            if(!playing)
                play_sem.acquire();
        }

        emit command_parsed(pair.cmd);
    }
}

