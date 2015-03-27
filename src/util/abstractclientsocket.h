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

#ifndef ABSTRACTCLIENTSOCKET_H
#define ABSTRACTCLIENTSOCKET_H

#include <QHostAddress>
#include <QObject>

class AbstractClientSocket : public QObject
{
    Q_OBJECT

public:
    virtual void connectToHost(const QString &address) = 0;
    virtual void connectToHost(const QHostAddress &address, ushort port) = 0;
    virtual void disconnectFromHost() = 0;
    virtual void send(const QByteArray &message) = 0;
    virtual bool isConnected() const = 0;
    virtual QString peerName() const = 0;
    virtual QString peerAddress() const = 0;
    virtual ushort peerPort() const = 0;

signals:
    void messageGot(const QByteArray &msg);
    void errorMessage(const QString &msg);
    void disconnected();
    void connected();
};

#endif // ABSTRACTCLIENTSOCKET_H

