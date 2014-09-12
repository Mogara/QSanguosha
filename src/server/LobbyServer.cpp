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

#include "LobbyServer.h"
#include "LobbyPlayer.h"
#include "json.h"

using namespace QSanProtocol;

LobbyServer::LobbyServer(QObject *parent)
    :Server(parent)
{
    packetSource = S_SRC_LOBBY;
}

void LobbyServer::broadcastSystemMessage(const QString &message)
{
    JsonArray body;
    body << ".";
    body << message;

    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_SPEAK);
    packet.setMessageBody(body);
    broadcast(packet.toJson(), true);
}

void LobbyServer::broadcast(const QByteArray &message, bool include_rooms)
{
    foreach (LobbyPlayer *player, players) {
        player->unicast(message);
    }

    if (include_rooms) {
        QMapIterator<ClientSocket *, RoomInfoStruct *> iter(rooms);
        while (iter.hasNext()) {
            ClientSocket *socket = iter.key();
            socket->send(message);
        }
    }
}

void LobbyServer::_processNewConnection(ClientSocket *socket)
{
    notifyClient(socket, S_COMMAND_ENTER_LOBBY);
    emit serverMessage(tr("%1 connected").arg(socket->peerName()));
    connect(socket, SIGNAL(message_got(QByteArray)), this, SLOT(processMessage(QByteArray)));
}

void LobbyServer::processMessage(const QByteArray &message)
{
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (socket == NULL) return;

    Packet packet;
    if (!packet.parse(message)) {
        emit serverMessage(tr("Invalid message %1 from %2").arg(QString::fromUtf8(message)).arg(socket->peerName()));
        return;
    }

    switch (packet.getPacketSource()) {
    case S_SRC_CLIENT:
        processClientSignup(socket, packet);
        break;
    case S_SRC_ROOM:
        break;
    default:
        emit serverMessage(tr("Packet %1 from %2 with an unknown source is discarded").arg(QString::fromUtf8(message)).arg(socket->peerName()));
    }
}

void LobbyServer::processClientSignup(ClientSocket *socket, const Packet &signup)
{
    socket->disconnect(this, SLOT(processMessage(QByteArray)));

    if (signup.getCommandType() != S_COMMAND_SIGNUP) {
        emit serverMessage(tr("Invalid signup string: %1").arg(signup.toString()));
        notifyClient(socket, S_COMMAND_WARN, "INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    JsonArray body = signup.getMessageBody().value<JsonArray>();
    //bool is_reconnection = body[0].toBool();
    QString screen_name = body[1].toString();
    QString avatar = body[2].toString();

    LobbyPlayer *player = new LobbyPlayer(this);
    player->setSocket(socket);
    player->setScreenName(screen_name);
    player->setAvatar(avatar);
    players << player;

    connect(player, SIGNAL(errorMessage(QString)), SIGNAL(serverMessage(QString)));
    connect(player, SIGNAL(disconnected()), SLOT(cleanupPlayer()));

    emit serverMessage(tr("%1 logged in as Player %2").arg(socket->peerName()).arg(screen_name));
}

void LobbyServer::cleanupPlayer()
{
    LobbyPlayer *player = qobject_cast<LobbyPlayer *>(sender());
    if (player == NULL) return;

    emit serverMessage(tr("Player %1(%2) logged out").arg(player->getScreenName()).arg(player->getSocketName()));

    player->setSocket(NULL);
    player->disconnect(this);
    this->disconnect(player);
    players.removeOne(player);
}
