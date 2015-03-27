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

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "abstractclientsocket.h"

class QTcpSocket;
class QTimer;

class ClientSocket : public AbstractClientSocket
{
    Q_OBJECT

public:
    ClientSocket(QObject *parent = 0);
    ClientSocket(QTcpSocket *socket, QObject *parent = 0);

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
    void raiseError(QAbstractSocket::SocketError socketError);
    void keepAlive();
    void checkConnectionState();

private:
    enum PacketType {
        UnknownPacket,
        InlineTextPacket,       // Texts ended with '\n'
        KeepAlivePacket,        // Checking the peer's state
        AcknowledgePacket
    };

    void init();

    QTcpSocket *const m_socket;
    bool m_isAlive;
    QTimer *m_keepAliveTimer;

    static const qint64 KEEP_ALIVE_INTERVAL;
    static const qint64 TIMEOUT_LIMIT;
};

#endif // CLIENTSOCKET_H
