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

#include <QCryptographicHash>

#include "server.h"
#include "nativesocket.h"
#include "clientstruct.h"
#include "engine.h"
#include "settings.h"
#include "lobbyplayer.h"
#include "json.h"

using namespace QSanProtocol;

QHash<CommandType, Server::LobbyFunction> Server::lobbyFunctions;
QHash<CommandType, Server::RoomFunction> Server::roomFunctions;

Server::Server(QObject *parent, Role role)
    : QObject(parent),  role(role), server(new NativeServerSocket),
      current(NULL), lobby(NULL), daemon(NULL)
{
    server->setParent(this);

    if (lobbyFunctions.isEmpty())
        initLobbyFunctions();

    if (roomFunctions.isEmpty())
        initRoomFunctions();

    ServerInfo.parse(Sanguosha->getSetupString());

    connect(server, &NativeServerSocket::new_connection, this, &Server::processNewConnection);
}

void Server::daemonize()
{
    daemon = new NativeUdpSocket(QHostAddress::Any, serverPort());
    connect(daemon, &UdpSocket::new_datagram, this, &Server::processDatagram);
}

void Server::processDatagram(const QByteArray &data, const QHostAddress &from, ushort port)
{
    if (daemon) {
        //@todo: A map of callback functions is necessary for more datagrams to be added.
        if (data == "whoIsServer")
            daemon->writeDatagram(Config.ServerName.toUtf8(), from, port);
    }
}

void Server::broadcastSystemMessage(const QString &message)
{
    JsonArray body;
    body << ".";
    body << message;

    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT | S_DEST_ROOM, S_COMMAND_SPEAK);
    packet.setMessageBody(body);
    broadcast(&packet);
}

void Server::broadcastNotification(CommandType command, const QVariant &data, int destination)
{
    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | destination, command);
    packet.setMessageBody(data);
    broadcast(&packet);
}

void Server::broadcast(const AbstractPacket *packet)
{
    PacketDescription destination = packet->getPacketDestination();
    if (destination & S_DEST_CLIENT) {
        foreach (LobbyPlayer *player, lobbyPlayers)
            player->unicast(packet->toJson());
    }

    if (destination & S_DEST_ROOM) {
        QMapIterator<ClientSocket *, QVariant> iter(remoteRooms);
        while (iter.hasNext()) {
            iter.next();
            ClientSocket *socket = iter.key();
            socket->send(packet->toJson());
        }

        foreach(Room *room, rooms)
            room->broadcast(packet);
    }
}

void Server::cleanup() {
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (Config.ForbidSIMC)
        addresses.remove(socket->peerAddress());

    socket->deleteLater();
}

void Server::notifyClient(ClientSocket *socket, CommandType command, const QVariant &arg)
{
    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(arg);
    socket->send(packet.toJson());
}

void Server::processNewConnection(ClientSocket *socket)
{
    QString address = socket->peerAddress();
    if (Config.ForbidSIMC) {
        if (addresses.contains(address)) {
            socket->disconnectFromHost();
            emit serverMessage(tr("Forbid the connection of address %1").arg(address));
            return;
        } else {
            addresses.insert(address);
        }
    }

    if (Config.value("BannedIP").toStringList().contains(address)){
        socket->disconnectFromHost();
        emit serverMessage(tr("Forbid the connection of address %1").arg(address));
        return;
    }

    connect(socket, &ClientSocket::disconnected, this, &Server::cleanup);
    notifyClient(socket, S_COMMAND_CHECK_VERSION, Sanguosha->getVersion());

    emit serverMessage(tr("%1 connected").arg(socket->peerName()));
    connect(socket, &ClientSocket::message_got, this, &Server::processMessage);
}

void Server::processMessage(const QByteArray &message)
{
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (socket == NULL) return;

    Packet packet;
    if (!packet.parse(message)) {
        emit serverMessage(tr("%1 Invalid message %2").arg(socket->peerName()).arg(QString::fromUtf8(message)));
        return;
    }

    switch (packet.getPacketSource()) {
    case S_SRC_CLIENT:
        processClientSignup(socket, packet);
        break;
    case S_SRC_ROOM:
        processRoomPacket(socket, packet);
        break;
    case S_SRC_LOBBY:
        if (socket == lobby) {
            processLobbyPacket(packet);
        }
        break;
    default:
        emit serverMessage(tr("%1 Packet %2 with an unknown source is discarded").arg(socket->peerName()).arg(QString::fromUtf8(message)));
    }
}

void Server::processClientSignup(ClientSocket *socket, const Packet &signup)
{
    disconnect(socket, &ClientSocket::message_got, this, &Server::processMessage);

    if (signup.getCommandType() != S_COMMAND_SIGNUP) {
        emit serverMessage(tr("%1 Invalid signup string: %2").arg(socket->peerName()).arg(signup.toString()));
        notifyClient(socket, S_COMMAND_WARN, S_WARNING_INVALID_SIGNUP_STRING);
        socket->disconnectFromHost();
        return;
    }

    JsonArray body = signup.getMessageBody().value<JsonArray>();
    if (body.size() < 3)
        return;
    bool is_reconnection = body.at(0).toBool();
    QString screen_name = body.at(1).toString();
    QString avatar = body.at(2).toString();
    if (screen_name.isEmpty() || avatar.isEmpty())
        return;

    if (is_reconnection) {
        foreach (QString objname, name2objname.values(screen_name)) {
            ServerPlayer *player = players.value(objname);
            Room *room = player->getRoom();
            if (player && player->getState() == "offline" && room && !room->isFinished()) {
                room->reconnect(player, socket);
                return;
            }
        }
    }

    if (role == RoomRole) {
        if (!Config.RoomPassword.isEmpty()) {
            QString password = body.size() < 4 ? QString() : body.at(3).toString();
            if (password != Config.RoomPassword) {
                notifyClient(socket, S_COMMAND_WARN, S_WARNING_WRONG_PASSWORD);
                return;
            }
        }

        currentRoomMutex.lock();
        if (current == NULL || current->isFull() || current->isFinished())
            current = createNewRoom(SettingsInstance);

        ServerPlayer *player = current->addSocket(socket);
        current->signup(player, screen_name, avatar, false);
        currentRoomMutex.unlock();

        emit newPlayer(player);

    } else {
        notifyClient(socket, S_COMMAND_ENTER_LOBBY);

        LobbyPlayer *player = new LobbyPlayer(this);
        player->setSocket(socket);
        player->setScreenName(screen_name);
        player->setAvatar(avatar);
        lobbyPlayers << player;

        connect(player, &LobbyPlayer::errorMessage, this, &Server::serverMessage);
        connect(player, &LobbyPlayer::disconnected, this, &Server::cleanupLobbyPlayer);

        emit serverMessage(tr("%1 logged in as Player %2").arg(socket->peerName()).arg(screen_name));

        player->notify(S_COMMAND_ROOM_LIST, getRoomList());
    }
}
