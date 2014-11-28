/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "recorder.h"
#include "client.h"
#include "protocol.h"

#include <QFile>
#include <QBuffer>
#include <QMessageBox>

#include <cmath>
using namespace QSanProtocol;

Recorder::Recorder(QObject *parent)
    : QObject(parent)
{
    watch.start();
}

void Recorder::recordLine(const QByteArray &line) {
    if (line.isEmpty())
        return;

    data.append(QString::number(watch.elapsed()));
    data.append(' ');
    data.append(line);
    if (!line.endsWith('\n'))
        data.append('\n');
}

bool Recorder::save(const QString &filename) const{
    if (filename.endsWith(".qsgs")) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            return file.write(data) != -1;
        else
            return false;
    }
    else if (filename.endsWith(".png")) {
        return TXT2PNG(data).save(filename);
    }
    else
        return false;
}

QList<QByteArray> Recorder::getRecords() const{
    QList<QByteArray> records = data.split('\n');
    return records;
}

QImage Recorder::TXT2PNG(const QByteArray &txtData) {
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
    : QThread(parent), m_commandSeriesCounter(1),
    filename(filename), speed(1.0), playing(true)
{
    QIODevice *device = NULL;
    if (filename.endsWith(".png")) {
        QByteArray *data = new QByteArray(PNG2TXT(filename));
        QBuffer *buffer = new QBuffer(data);
        device = buffer;
    }
    else if (filename.endsWith(".qsgs")) {
        QFile *file = new QFile(filename);
        device = file;
    }

    if (device == NULL)
        return;

    if (!device->open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!device->atEnd()) {
        QByteArray line = device->readLine();
        int split = line.indexOf(' ');

        Pair pair;
        pair.elapsed = line.left(split).toInt();
        pair.cmd = line.mid(split + 1);

        pairs << pair;
    }

    delete device;

    int time_offset = 0;
    pair_offset = 0;
    foreach (const Pair &pair, pairs) {
        Packet packet;
        if (packet.parse(pair.cmd)) {
            if (packet.getCommandType() == S_COMMAND_START_IN_X_SECONDS) {
                time_offset = pair.elapsed;
                break;
            }
        }
        pair_offset++;
    }
    duration = pairs.last().elapsed - time_offset;
}

QByteArray Replayer::PNG2TXT(const QString &filename) {
    QImage image(filename);
    image = image.convertToFormat(QImage::Format_ARGB32);
    const uchar *imageData = image.bits();
    qint32 actual_size = *(const qint32 *)imageData;
    QByteArray data((const char *)(imageData + 4), actual_size);
    data = qUncompress(data);

    return data;
}

qreal Replayer::getSpeed() {
    qreal speed;
    mutex.lock();
    speed = this->speed;
    mutex.unlock();
    return speed;
}

void Replayer::uniform() {
    mutex.lock();

    if (speed != 1.0) {
        speed = 1.0;
        emit speed_changed(1.0);
    }

    mutex.unlock();
}

void Replayer::speedUp() {
    mutex.lock();

    if (speed < 6.0) {
        qreal inc = speed >= 2.0 ? 1.0 : 0.5;
        speed += inc;
        emit speed_changed(speed);
    }

    mutex.unlock();
}

void Replayer::slowDown() {
    mutex.lock();

    if (speed >= 1.0) {
        qreal dec = speed > 2.0 ? 1.0 : 0.5;
        speed -= dec;
        emit speed_changed(speed);
    }

    mutex.unlock();
}

void Replayer::toggle() {
    playing = !playing;
    if (playing)
        play_sem.release(); // to play
}

void Replayer::run() {
    int i = 0;
    const int pair_num = pairs.length();

    while (i < pair_offset) {
        const Pair &pair = pairs.at(i);
        emit command_parsed(pair.cmd);
        i++;
    }

    int last = 0;
    const int time_offset = pairs.at(pair_offset).elapsed;
    while (i < pair_num) {
        const Pair &pair = pairs.at(i);

        int delay = qMax(0, qMin(pair.elapsed - last, 2500));
        delay /= getSpeed();
        msleep(delay);

        emit elasped((pair.elapsed - time_offset) / 1000);

        if (!playing)
            play_sem.acquire();

        emit command_parsed(pair.cmd);

        last = pair.elapsed;
        i++;
    }
}

QString Replayer::getPath() const{
    return filename;
}

