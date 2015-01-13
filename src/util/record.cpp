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

#include "record.h"
#include "client.h"
#include "protocol.h"

#include <QFile>
#include <QBuffer>
#include <QMessageBox>

using namespace QSanProtocol;

Record::Record(QObject *parent)
    :QObject(parent), mFormat(CompressedText)
{
}

Record::Record(const QString &fileName, QObject *parent)
    :QObject(parent), mFileName(fileName), mFormat(CompressedText)
{
}

void Record::addCommand(int elapsed, const QByteArray &data)
{
    Command command;
    command.elapsed = elapsed;
    command.data = data;
    addCommand(command);
}

bool Record::open()
{
    QIODevice *device = new QFile(mFileName);
    if (!device->open(QFile::ReadOnly))
        return false;

    char header;
    device->getChar(&header);
    if (header == '\0') {
        mFormat = CompressedText;
        QByteArray content = device->readAll();
        delete device;

        QByteArray *data = new QByteArray(qUncompress(content));
        device = new QBuffer(data);
        device->open(QFile::ReadOnly);
    } else {
        mFormat = PlainText;
        device->ungetChar(header);
        device->seek(0);
    }

    mCommands.clear();
    while (!device->atEnd()) {
        QByteArray line = device->readLine();
        int split = line.indexOf(' ');

        Command command;
        command.elapsed = line.left(split).toInt();
        command.data = line.mid(split + 1);

        mCommands << command;
    }

    return true;
}

bool Record::open(const QString &fileName)
{
    mFileName = fileName;
    return open();
}

bool Record::save() const
{
    return saveAs(mFileName);
}

bool Record::saveAs(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
        return false;

    switch (format()) {
    case CompressedText:{
        QByteArray data;
        foreach (const Command &command, mCommands) {
            data.append(QByteArray::number(command.elapsed));
            data.append(' ');
            data.append(command.data);
            if (!command.data.endsWith('\n'))
                data.append('\n');
        }

        file.putChar('\0');
        file.write(qCompress(data));
        break;
    }
    case PlainText:{
        foreach (const Command &command, mCommands) {
            file.write(QByteArray::number(command.elapsed));
            file.putChar(' ');
            file.write(command.data);
            if (!command.data.endsWith('\n'))
                file.putChar('\n');
        }
        break;
    }
    default:
        return false;
    }

    file.close();
    return true;
}

Recorder::Recorder(QObject *parent)
    : QObject(parent), mRecord(new Record(this))
{
    mWatch.start();
}

Recorder::Recorder(Record *parent)
    : QObject(parent), mRecord(parent)
{
    mWatch.start();
}

void Recorder::recordLine(const QByteArray &line) {
    if (line.isEmpty())
        return;

    mRecord->addCommand(mWatch.elapsed(), line);
}

Replayer::Replayer(QObject *parent)
    : QThread(parent), mRecord(NULL), mSpeed(1.0), mIsPlaying(true)
{
}

Replayer::Replayer(Record *record)
    : QThread(record), mSpeed(1.0), mIsPlaying(true)
{
    setRecord(record);
}

Replayer::Replayer(const QString &fileName, QObject *parent)
    : QThread(parent), mSpeed(1.0), mIsPlaying(true)
{
    Record *record = new Record(fileName, this);
    if (record->open())
        setRecord(record);
}

void Replayer::setRecord(Record *record)
{
    mRecord = record;
    if (record == NULL)
        return;

    const QList<Record::Command> &commands = mRecord->commands();
    if (commands.isEmpty())
        return;

    int time_offset = 0;
    mCommandOffset = 0;
    foreach (const Record::Command &command, commands) {
        Packet packet;
        if (packet.parse(command.data)) {
            if (packet.getCommandType() == S_COMMAND_START_IN_X_SECONDS) {
                time_offset = command.elapsed;
                break;
            }
        }
        mCommandOffset++;
    }
    mDuration = commands.last().elapsed - time_offset;
}

qreal Replayer::getSpeed() {
    qreal speed;
    mMutex.lock();
    speed = mSpeed;
    mMutex.unlock();
    return speed;
}

void Replayer::uniform() {
    mMutex.lock();

    if (mSpeed != 1.0) {
        mSpeed = 1.0;
        emit speedChanged(1.0);
    }

    mMutex.unlock();
}

void Replayer::speedUp() {
    mMutex.lock();

    if (mSpeed < 6.0) {
        qreal inc = mSpeed >= 2.0 ? 1.0 : 0.5;
        mSpeed += inc;
        emit speedChanged(mSpeed);
    }

    mMutex.unlock();
}

void Replayer::slowDown() {
    mMutex.lock();

    if (mSpeed >= 1.0) {
        qreal dec = mSpeed > 2.0 ? 1.0 : 0.5;
        mSpeed -= dec;
        emit speedChanged(mSpeed);
    }

    mMutex.unlock();
}

void Replayer::toggle() {
    mIsPlaying = !mIsPlaying;
    if (mIsPlaying)
        mPlaySemaphore.release(); // to play
}

void Replayer::run() {
    const QList<Record::Command> &commands = mRecord->commands();
    if (commands.isEmpty())
        return;

    const int pair_num = commands.length();

    int i = 0;
    while (i < mCommandOffset) {
        const Record::Command &command = commands.at(i);
        emit commandParsed(command.data);
        i++;
    }

    int last = 0;
    const int time_offset = commands.at(mCommandOffset).elapsed;
    while (i < pair_num) {
        const Record::Command &command = commands.at(i);

        int delay = qMax(0, qMin(command.elapsed - last, 2500));
        delay /= getSpeed();
        msleep(delay);

        emit elasped((command.elapsed - time_offset) / 1000);

        if (!mIsPlaying)
            mPlaySemaphore.acquire();

        emit commandParsed(command.data);

        last = command.elapsed;
        i++;
    }
}
