/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#ifndef _SOCKET_H
#define _SOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

class ClientSocket;

class ServerSocket : public QObject {
    Q_OBJECT

public:
    virtual bool listen() = 0;
    virtual void daemonize() = 0;

signals:
    void new_connection(ClientSocket *connection);
};

class ClientSocket : public QObject {
    Q_OBJECT

public:
    virtual void connectToHost() = 0;
    virtual void connectToHost(const QHostAddress &address) = 0;
    virtual void connectToHost(const QHostAddress &address, ushort port) = 0;
    virtual void disconnectFromHost() = 0;
    virtual void send(const QByteArray &message) = 0;
    virtual bool isConnected() const = 0;
    virtual QString peerName() const = 0;
    virtual QString peerAddress() const = 0;
    virtual ushort peerPort() const = 0;

signals:
    void message_got(const QByteArray &msg);
    void error_message(const QString &msg);
    void disconnected();
    void connected();
};

typedef char buffer_t[65535];

#endif

