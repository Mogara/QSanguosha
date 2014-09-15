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

#ifndef _NATIVESOCKET_H
#define _NATIVESOCKET_H

#include "socket.h"

class QUdpSocket;

class NativeServerSocket : public ServerSocket {
    Q_OBJECT

public:
    NativeServerSocket();

    virtual bool listen();
    virtual void daemonize();

private slots:
    void processNewConnection();
    void processNewDatagram();

private:
    QTcpServer *server;
    QUdpSocket *daemon;
};


class NativeClientSocket : public ClientSocket {
    Q_OBJECT

public:
    NativeClientSocket();
    NativeClientSocket(QTcpSocket *socket);

    virtual void connectToHost();
    virtual void connectToHost(const QHostAddress &address);
    virtual void connectToHost(const QHostAddress &address, ushort port);
    virtual void disconnectFromHost();
    virtual void send(const QByteArray &message);
    virtual bool isConnected() const;
    virtual QString peerName() const;
    virtual QString peerAddress() const;
    virtual ushort peerPort() const;

private slots:
    void getMessage();
    void raiseError(QAbstractSocket::SocketError socket_error);

private:
    QTcpSocket *const socket;

    void init();
};

#endif

