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

#include "LobbyServer.h"
#include "LobbyPlayer.h"
#include "json.h"
#include "clientstruct.h"

using namespace QSanProtocol;

LobbyServer::LobbyServer(QObject *parent)
    :Server(parent)
{
    packetSource = S_SRC_LOBBY;

    callbacks[S_COMMAND_SETUP] = &LobbyServer::setupNewRoom;
}

void LobbyServer::broadcastSystemMessage(const QString &message)
{
    JsonArray body;
    body << ".";
    body << message;

    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT | S_DEST_ROOM, S_COMMAND_SPEAK);
    packet.setMessageBody(body);
    broadcast(&packet);
}

void LobbyServer::broadcastNotification(CommandType command, const QVariant &data, int destination)
{
    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | destination, command);
    packet.setMessageBody(data);
    broadcast(&packet);
}

void LobbyServer::broadcast(const AbstractPacket *packet)
{
    broadcast(packet->toJson(), packet->getPacketDestination());
}

void LobbyServer::broadcast(const QByteArray &message, int destination)
{
    if (destination & S_DEST_CLIENT) {
        foreach (LobbyPlayer *player, players) {
            player->unicast(message);
        }
    }

    if (destination & S_DEST_ROOM) {
        QMapIterator<ClientSocket *, QVariant> iter(rooms);
        while (iter.hasNext()) {
            ClientSocket *socket = iter.key();
            socket->send(message);
        }
    }
}

void LobbyServer::_processNewConnection(ClientSocket *socket)
{
    emit serverMessage(tr("%1 connected").arg(socket->peerName()));
    connect(socket, SIGNAL(message_got(QByteArray)), this, SLOT(processMessage(QByteArray)));
}

void LobbyServer::processMessage(const QByteArray &message)
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
    default:
        emit serverMessage(tr("%1 Packet %2 with an unknown source is discarded").arg(socket->peerName()).arg(QString::fromUtf8(message)));
    }
}

void LobbyServer::processClientSignup(ClientSocket *socket, const Packet &signup)
{
    socket->disconnect(this, SLOT(processMessage(QByteArray)));

    if (signup.getCommandType() != S_COMMAND_SIGNUP) {
        emit serverMessage(tr("%1 Invalid signup string: %2").arg(socket->peerName()).arg(signup.toString()));
        notifyClient(socket, S_COMMAND_WARN, "INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    notifyClient(socket, S_COMMAND_ENTER_LOBBY);

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

    sendRoomListTo(player);
}

void LobbyServer::processRoomPacket(ClientSocket *socket, const Packet &packet)
{
    Callback func = callbacks.value(packet.getCommandType());
    if (func) {
        (this->*func)(socket, packet.getMessageBody());
    } else {
        emit serverMessage(tr("%1 Packet %2 with an invalid command is discarded").arg(socket->peerName()).arg(packet.toString()));
    }
}

void LobbyServer::cleanupPlayer()
{
    LobbyPlayer *player = qobject_cast<LobbyPlayer *>(sender());
    if (player == NULL) return;

    emit serverMessage(tr("%1 Player %2 logged out").arg(player->getSocketName()).arg(player->getScreenName()));

    player->setSocket(NULL);
    player->disconnect(this);
    this->disconnect(player);
    players.removeOne(player);
}

void LobbyServer::setupNewRoom(ClientSocket *from, const QVariant &data)
{
    JsonArray args = data.value<JsonArray>();
    if (args.size() != 7) return;

    //check if the room server can be connected
    ushort port = args.at(1).toUInt();
    QTcpSocket socket;
    socket.connectToHost(from->peerAddress(), port);
    if (socket.waitForConnected(2000)) {
        emit serverMessage(tr("%1 signed up as a Room Server on port %2").arg(from->peerName()).arg(port));

        /*args.at(0).toString();  //SetupString
        args.at(1).toUInt();    //HostPort
        args.at(2).toInt();     //PlayerNum
        args.at(3).toInt();     //RoomNum
        args.at(4).toInt();     //MaxRoomNum
        args.at(5).toInt();     //AIDelay
        args.at(6).toBool();    //RewardTheFirstShowingPlayer*/
        ServerInfoStruct info;
        if (!info.parse(args.at(0).toString())) {
            emit serverMessage(tr("%1 Invalid setup string").arg(from->peerName()));
            return;
        }

        int maxRoomNum = args.at(4).toInt();
        if (maxRoomNum < -1 || args.at(3).toInt() > maxRoomNum) {
            emit serverMessage(tr("%1 Invalid room number or max room number").arg(from->peerName()));
            return;
        }

        JsonArray data(args);
        data[1] = QString("%1:%2").arg(from->peerAddress()).arg(port);
        rooms.insert(from, data);

        connect(from, SIGNAL(disconnected()), this, SLOT(cleanupRoom()));
    }
}

void LobbyServer::cleanupRoom()
{
    //no need to delete the socket for it's deleted in cleanup()
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (socket == NULL) return;

    emit serverMessage(tr("%1 Room Server disconnected").arg(socket->peerName()));
    rooms.remove(socket);
    socket->disconnect(this);
    this->disconnect(socket);
}

void LobbyServer::sendRoomListTo(LobbyPlayer *player, int page)
{
    if (rooms.isEmpty()) {
        player->notify(S_COMMAND_ROOM_LIST);
        return;
    }

    static const int pageLimit = 10;
    int offset = page * pageLimit;
    if (offset >= rooms.size())
        return;

    QMapIterator<ClientSocket *, QVariant> iter(rooms);
    for (int i = 0; i < offset; i++)
        iter.next();

    JsonArray data;
    for (int i = 0; i < pageLimit && iter.hasNext(); i++) {
        iter.next();
        data << iter.value();
    }
    player->notify(S_COMMAND_ROOM_LIST, data);
}
