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

#ifndef _RECORD_H
#define _RECORD_H

#include <QTime>
#include <QThread>
#include <QMutex>
#include <QSemaphore>

class Record : public QObject
{
    Q_OBJECT

public:
    struct Command
    {
        int elapsed;
        QByteArray data;
    };

    enum Format
    {
        CompressedText,
        PlainText
    };

    explicit Record(QObject *parent = 0);
    explicit Record(const QString &fileName, QObject *parent = 0);

    void setFormat(Format format) { m_format = format; }
    Format format() const { return m_format; }

    QString fileName() const { return m_fileName; }

public slots:
    void addCommand(int elapsed, const QByteArray &data);
    void addCommand(const Command &command) { m_commands.append(command); }
    const QList<Command> &commands() const { return m_commands; }

    bool open();
    bool open(const QString &fileName);

    bool save() const;
    bool saveAs(const QString &fileName) const;

private:
    QList<Command> m_commands;
    QString m_fileName;
    Format m_format;
};

class Recorder : public QObject {
    Q_OBJECT

public:
    explicit Recorder(QObject *parent = 0);
    explicit Recorder(Record *parent);

    bool save(const QString &fileName) const { return m_record->saveAs(fileName); }
    const Record *getRecord() const { return m_record; }

public slots:
    void recordLine(const QByteArray &line);

private:
    QTime m_watch;
    Record *m_record;
};

class Replayer : public QThread {
    Q_OBJECT

public:
    explicit Replayer(QObject *parent = 0);
    explicit Replayer(Record *record);
    explicit Replayer(const QString &fileName, QObject *parent);

    void setRecord(Record *record);
    int getDuration() const { return m_duration / 1000; }
    qreal getSpeed();

    QString getPath() const { return m_record != NULL ? m_record->fileName() : QString(); }
    const Record *getRecord() const { return m_record; }

public slots:
    void uniform();
    void toggle();
    void speedUp();
    void slowDown();

protected:
    virtual void run();

private:
    Record *m_record;
    qreal m_speed;
    bool m_isPlaying;
    QMutex m_mutex;
    QSemaphore m_playSemaphore;
    int m_duration;
    int m_commandOffset;

signals:
    void commandParsed(const QByteArray &cmd);
    void elasped(int secs);
    void speedChanged(qreal m_speed);
};

#endif

