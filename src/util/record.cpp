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
    :QObject(parent), m_format(CompressedText)
{
}

Record::Record(const QString &fileName, QObject *parent)
    :QObject(parent), m_fileName(fileName), m_format(CompressedText)
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
    QIODevice *device = new QFile(m_fileName);
    if (!device->open(QFile::ReadOnly))
        return false;

    char header;
    device->getChar(&header);
    if (header == '\0') {
        m_format = CompressedText;
        QByteArray content = device->readAll();
        delete device;

        QByteArray *data = new QByteArray(qUncompress(content));
        device = new QBuffer(data);
        device->open(QFile::ReadOnly);
    } else {
        m_format = PlainText;
        device->ungetChar(header);
        device->seek(0);
    }

    m_commands.clear();
    while (!device->atEnd()) {
        QByteArray line = device->readLine();
        int split = line.indexOf(' ');

        Command command;
        command.elapsed = line.left(split).toInt();
        command.data = line.mid(split + 1);

        m_commands << command;
    }

    return true;
}

bool Record::open(const QString &fileName)
{
    m_fileName = fileName;
    return open();
}

bool Record::save() const
{
    return saveAs(m_fileName);
}

bool Record::saveAs(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
        return false;

    switch (format()) {
    case CompressedText:{
        QByteArray data;
        foreach (const Command &command, m_commands) {
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
        foreach (const Command &command, m_commands) {
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
    : QObject(parent), m_record(new Record(this))
{
    m_watch.start();
}

Recorder::Recorder(Record *parent)
    : QObject(parent), m_record(parent)
{
    m_watch.start();
}

void Recorder::recordLine(const QByteArray &line) {
    if (line.isEmpty())
        return;

    m_record->addCommand(m_watch.elapsed(), line);
}

Replayer::Replayer(QObject *parent)
    : QThread(parent), m_record(NULL), m_speed(1.0), m_isPlaying(true)
{
}

Replayer::Replayer(Record *record)
    : QThread(record), m_speed(1.0), m_isPlaying(true)
{
    setRecord(record);
}

Replayer::Replayer(const QString &fileName, QObject *parent)
    : QThread(parent), m_speed(1.0), m_isPlaying(true)
{
    Record *record = new Record(fileName, this);
    if (record->open())
        setRecord(record);
}

void Replayer::setRecord(Record *record)
{
    m_record = record;
    if (record == NULL)
        return;

    const QList<Record::Command> &commands = m_record->commands();
    if (commands.isEmpty())
        return;

    int time_offset = 0;
    m_commandOffset = 0;
    foreach (const Record::Command &command, commands) {
        Packet packet;
        if (packet.parse(command.data)) {
            if (packet.getCommandType() == S_COMMAND_START_IN_X_SECONDS) {
                time_offset = command.elapsed;
                break;
            }
        }
        m_commandOffset++;
    }
    m_duration = commands.last().elapsed - time_offset;
}

qreal Replayer::getSpeed() {
    qreal speed;
    m_mutex.lock();
    speed = m_speed;
    m_mutex.unlock();
    return speed;
}

void Replayer::uniform() {
    m_mutex.lock();

    if (m_speed != 1.0) {
        m_speed = 1.0;
        emit speedChanged(1.0);
    }

    m_mutex.unlock();
}

void Replayer::speedUp() {
    m_mutex.lock();

    if (m_speed < 6.0) {
        qreal inc = m_speed >= 2.0 ? 1.0 : 0.5;
        m_speed += inc;
        emit speedChanged(m_speed);
    }

    m_mutex.unlock();
}

void Replayer::slowDown() {
    m_mutex.lock();

    if (m_speed >= 1.0) {
        qreal dec = m_speed > 2.0 ? 1.0 : 0.5;
        m_speed -= dec;
        emit speedChanged(m_speed);
    }

    m_mutex.unlock();
}

void Replayer::toggle() {
    m_isPlaying = !m_isPlaying;
    if (m_isPlaying)
        m_playSemaphore.release(); // to play
}

void Replayer::run() {
    const QList<Record::Command> &commands = m_record->commands();
    if (commands.isEmpty())
        return;

    const int pair_num = commands.length();

    int i = 0;
    while (i < m_commandOffset) {
        const Record::Command &command = commands.at(i);
        emit commandParsed(command.data);
        i++;
    }

    int last = 0;
    const int time_offset = commands.at(m_commandOffset).elapsed;
    while (i < pair_num) {
        const Record::Command &command = commands.at(i);

        int delay = qMax(0, qMin(command.elapsed - last, 2500));
        delay /= getSpeed();
        msleep(delay);

        emit elasped((command.elapsed - time_offset) / 1000);

        if (!m_isPlaying)
            m_playSemaphore.acquire();

        emit commandParsed(command.data);

        last = command.elapsed;
        i++;
    }
}
