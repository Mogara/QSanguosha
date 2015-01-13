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

    void setFormat(Format format) { mFormat = format; }
    Format format() const { return mFormat; }

    QString fileName() const { return mFileName; }

public slots:
    void addCommand(int elapsed, const QByteArray &data);
    void addCommand(const Command &command) { mCommands.append(command); }
    const QList<Command> &commands() const { return mCommands; }

    bool open();
    bool open(const QString &fileName);

    bool save() const;
    bool saveAs(const QString &fileName) const;

private:
    QList<Command> mCommands;
    QString mFileName;
    Format mFormat;
};

class Recorder : public QObject {
    Q_OBJECT

public:
    explicit Recorder(QObject *parent = 0);
    explicit Recorder(Record *parent);

    bool save(const QString &fileName) const { return mRecord->saveAs(fileName); }
    const Record *getRecord() const { return mRecord; }

public slots:
    void recordLine(const QByteArray &line);

private:
    QTime mWatch;
    Record *mRecord;
};

class Replayer : public QThread {
    Q_OBJECT

public:
    explicit Replayer(QObject *parent = 0);
    explicit Replayer(Record *record);
    explicit Replayer(const QString &fileName, QObject *parent);

    void setRecord(Record *record);
    int getDuration() const { return mDuration / 1000; }
    qreal getSpeed();

    QString getPath() const { return mRecord != NULL ? mRecord->fileName() : QString(); }
    const Record *getRecord() const { return mRecord; }

public slots:
    void uniform();
    void toggle();
    void speedUp();
    void slowDown();

protected:
    virtual void run();

private:
    Record *mRecord;
    qreal mSpeed;
    bool mIsPlaying;
    QMutex mMutex;
    QSemaphore mPlaySemaphore;
    int mDuration;
    int mCommandOffset;

signals:
    void commandParsed(const QByteArray &cmd);
    void elasped(int secs);
    void speedChanged(qreal mSpeed);
};

#endif

