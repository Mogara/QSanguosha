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

#include "Server.h"
#include "nativesocket.h"
#include "clientstruct.h"
#include "engine.h"
#include "settings.h"

#include <QApplication>

using namespace QSanProtocol;

Server::Server(QObject *parent)
    : QObject(parent), server(new NativeServerSocket)
{
    server->setParent(this);

    ServerInfo.parse(Sanguosha->getSetupString());

    connect(server, SIGNAL(new_connection(ClientSocket *)), this, SLOT(processNewConnection(ClientSocket *)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Server::cleanup() {
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (Config.ForbidSIMC)
        addresses.removeOne(socket->peerAddress());

    socket->deleteLater();
}

void Server::notifyClient(ClientSocket *socket, CommandType command, const QVariant &arg)
{
    Packet packet(packetSource | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(arg);
    socket->send(packet.toJson());
}

void Server::processNewConnection(ClientSocket *socket)
{
    QString address = socket->peerAddress();
    if (Config.ForbidSIMC) {
        if (addresses.contains(address)) {
            addresses.append(address);
            socket->disconnectFromHost();
            emit serverMessage(tr("Forbid the connection of address %1").arg(address));
            return;
        } else {
            addresses.append(address);
        }
    }

    if (Config.value("BannedIP").toStringList().contains(address)){
        socket->disconnectFromHost();
        emit serverMessage(tr("Forbid the connection of address %1").arg(address));
        return;
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));
    notifyClient(socket, S_COMMAND_CHECK_VERSION, Sanguosha->getVersion());

    _processNewConnection(socket);
}
