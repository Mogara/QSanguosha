#include "recorder.h"
#include "client.h"

#include <cstdlib>
#include <cmath>

#include <QFile>
#include <QBuffer>
#include <QMessageBox>
using namespace QSanProtocol;

Recorder::Recorder(QObject *parent)
    :QObject(parent)
{
    watch.start();
}

void Recorder::record(const char *line)
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
    if(filename.endsWith(".txt")){
        QFile file(filename);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
            return file.write(data) != -1;
        else
            return false;
    }else if(filename.endsWith(".png")){
        return TXT2PNG(data).save(filename);
    }else
        return false;
}

QImage Recorder::TXT2PNG(QByteArray txtData){
    QByteArray data = qCompress(txtData, 9);
    qint32 actual_size = data.size();
    data.prepend((const char *)&actual_size, sizeof(qint32));

    // actual data = width * height - padding
    int width = ceil(sqrt((double)data.size()));
    int height = width;
    int padding = width * height - data.size();
    QByteArray paddingData;
    paddingData.fill('\0', padding);
    data.append(paddingData);

    QImage image((const uchar *)data.constData(), width, height, QImage::Format_ARGB32);
    return image;
}

Replayer::Replayer(QObject *parent, const QString &filename)
    :QThread(parent), m_commandSeriesCounter(1),
      filename(filename), speed(1.0), playing(true)
{
    QIODevice *device = NULL;
    if(filename.endsWith(".png")){
        QByteArray *data = new QByteArray(PNG2TXT(filename));
        QBuffer *buffer = new QBuffer(data);
        device = buffer;
    }else if(filename.endsWith(".txt")){
        QFile *file = new QFile(filename);
        device = file;
    }

    if(device == NULL)
        return;

    if(!device->open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    typedef char buffer_t[1024];

    while(!device->atEnd()){
        buffer_t line;
        memset(line, 0, sizeof(buffer_t));
        device->readLine(line, sizeof(buffer_t));

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

    delete device;
}

QByteArray Replayer::PNG2TXT(const QString filename){
    QImage image(filename);
    image = image.convertToFormat(QImage::Format_ARGB32);
    const uchar *imageData = image.bits();
    qint32 actual_size = *(const qint32 *)imageData;
    QByteArray data((const char *)(imageData+4), actual_size);
    data = qUncompress(data);

    return data;
}

QString &Replayer::commandProceed(QString &cmd){
    static QStringList split_flags;
    if(split_flags.isEmpty()){
        split_flags << ":" << "+" << "_" << "->";
    }

    foreach(QString flag, split_flags){
        QStringList messages = cmd.split(flag);
        if(messages.length() > 1){
            QStringList message_analyse;
            foreach(QString message, messages){
                message_analyse << commandProceed(message);
            }
            cmd = "[" + message_analyse.join(",") + "]";
        }
        else{
            bool ok = false;
            cmd.toInt(&ok);

            if(!cmd.startsWith("\"") && !cmd.startsWith("[") && !ok)
                cmd = "\"" + cmd +"\"";
        }
    }

    return cmd;
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

