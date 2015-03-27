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

#include "clientsocket.h"
#include "serversocket.h"

#include <QTcpServer>

ServerSocket::ServerSocket(QObject *parent)
{
    setParent(parent);
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &ServerSocket::processNewConnection);
}

bool ServerSocket::listen(const QHostAddress &address, ushort port)
{
    return m_server->listen(address, port);
}

ushort ServerSocket::serverPort() const
{
    return m_server->serverPort();
}

void ServerSocket::processNewConnection() {
    QTcpSocket *socket = m_server->nextPendingConnection();
    ClientSocket *connection = new ClientSocket(socket);
    emit newConnection(connection);
}
