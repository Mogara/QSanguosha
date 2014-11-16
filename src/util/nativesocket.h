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
class QTimer;

class NativeServerSocket : public ServerSocket {
    Q_OBJECT

public:
    NativeServerSocket();

    virtual bool listen(const QHostAddress &address, ushort port = 0);
    virtual ushort serverPort() const;

private slots:
    void processNewConnection();

private:
    QTcpServer *server;
    QUdpSocket *daemon;
};


class NativeClientSocket : public ClientSocket {
    Q_OBJECT

public:
    NativeClientSocket();
    NativeClientSocket(QTcpSocket *socket);

    virtual void connectToHost(const QString &address);
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
    void keepAlive();
    void checkConnectionState();

private:
    enum PacketType{
        UnknownPacket,
        InlineTextPacket,   //Texts ended with '\n'
        KeepAlivePacket     //Checking the peer's state
    };

    void init();

    QTcpSocket *const socket;
    bool is_alive;
    QTimer *keep_alive_timer;

    static const qint64 KEEP_ALIVE_INTERVAL;
    static const qint64 TIMEOUT_LIMIT;
};

class NativeUdpSocket : public UdpSocket {
    Q_OBJECT

public:
    NativeUdpSocket(const QHostAddress &address, ushort port);

    virtual void writeDatagram(const QByteArray &data, const QHostAddress &to, ushort port);

private slots:
    void processNewDatagram();

private:
    QUdpSocket *socket;
};

#endif

