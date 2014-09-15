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

#ifndef _DETECTOR_H
#define _DETECTOR_H

#include <QObject>
#include <QString>
#include <QUdpSocket>
#include <QThread>

class Detector : public QObject {
    Q_OBJECT

public slots:
    virtual void detect() = 0;
    virtual void stop() = 0;

signals:
    void detected(const QString &server_name, const QString &address);
};

class UdpDetector : public Detector {
    Q_OBJECT

public:
    UdpDetector();
    virtual void detect();
    virtual void stop();

private slots:
    void onReadReady();

private:
    QUdpSocket *socket;
};

#endif

