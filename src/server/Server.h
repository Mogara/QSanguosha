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

#ifndef SERVER_H
#define SERVER_H

#include "nativesocket.h"
#include "protocol.h"

#include <QObject>
#include <QStringList>

class Server : public QObject {
    Q_OBJECT

public:
    explicit Server(QObject *parent);

    virtual void broadcastSystemMessage(const QString &msg) = 0;

    bool listen() { return server->listen(); }
    bool listen(const QHostAddress &address, ushort port) { return server->listen(address, port); }
    ushort serverPort() const {return server->serverPort(); }
    void daemonize() { server->daemonize(); }

protected slots:
    virtual void processNewConnection(ClientSocket *socket);
    void cleanup();

protected:
    void notifyClient(ClientSocket *socket, QSanProtocol::CommandType command, const QVariant &arg = QVariant());
    virtual void _processNewConnection(ClientSocket *socket) = 0;

    ServerSocket *server;
    QStringList addresses;
    QSanProtocol::PacketDescription packetSource;

signals:
    void serverMessage(const QString &);
};

#endif // SERVER_H
