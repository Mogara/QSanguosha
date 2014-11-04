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

#include "server.h"
#include "lobbyplayer.h"
#include "json.h"
#include "clientstruct.h"
#include "room.h"

using namespace QSanProtocol;

void Server::initRoomFunctions()
{
    roomFunctions[S_COMMAND_SETUP] = &Server::setupNewRemoteRoom;
}

void Server::processRoomPacket(ClientSocket *socket, const Packet &packet)
{
    RoomFunction func = roomFunctions.value(packet.getCommandType());
    if (func) {
        (this->*func)(socket, packet.getMessageBody());
    } else {
        emit serverMessage(tr("%1 Packet %2 with an invalid command is discarded").arg(socket->peerName()).arg(packet.toString()));
    }
}

void Server::cleanupLobbyPlayer()
{
    LobbyPlayer *player = qobject_cast<LobbyPlayer *>(sender());
    if (player == NULL) return;

    if (!player->getSocketName().isEmpty())
        emit serverMessage(tr("%1 Player %2 logged out").arg(player->getSocketName()).arg(player->getScreenName()));

    player->setSocket(NULL);
    lobbyPlayers.removeOne(player);
}

void Server::setupNewRemoteRoom(ClientSocket *from, const QVariant &data)
{
    JsonArray args = data.value<JsonArray>();
    if (args.size() != 6) return;

    //check if the room server can be connected
    ushort port = args.at(1).toUInt();
    QTcpSocket socket;
    socket.connectToHost(from->peerAddress(), port);
    if (socket.waitForConnected(2000)) {
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
        if (maxRoomNum < -1 || (maxRoomNum != -1 && args.at(3).toInt() > maxRoomNum)) {
            emit serverMessage(tr("%1 Invalid room number or max room number").arg(from->peerName()));
            return;
        }

        emit serverMessage(tr("%1 signed up as a Room Server on port %2").arg(from->peerName()).arg(port));

        args[1] = QString("%1:%2").arg(from->peerAddress()).arg(port);
        remoteRooms.insert(from, args);

        connect(from, SIGNAL(disconnected()), this, SLOT(cleanupRemoteRoom()));
    }
}

void Server::cleanupRemoteRoom()
{
    //no need to delete the socket for it's deleted in cleanup()
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (socket == NULL) return;

    emit serverMessage(tr("%1 Room Server disconnected").arg(socket->peerName()));
    remoteRooms.remove(socket);
    socket->disconnect(this);
    this->disconnect(socket);
}

QVariant Server::getRoomList(int page)
{
    if (rooms.isEmpty() && remoteRooms.isEmpty()) {
        return QVariant();
    }

    static const int pageLimit = 10;
    int offset = page * pageLimit;
    if (offset >= rooms.size() + remoteRooms.size())
        return QVariant();

    int end = offset + pageLimit;
    JsonArray data;
    int i = 0;

    QSetIterator<Room *> iter1(rooms);
    while (iter1.hasNext()) {
        Room *room = iter1.next();
        if (i >= offset) {
            JsonArray item;
            item << room->getSetupString();
            item << QVariant();//No host address. It's not a remote room.
            item << room->getPlayers().size();
            item << room->getId();//room number
            item << rooms.size();//max room number
            const RoomConfig &config = room->getConfig();
            item << config.AIDelay;
            data << QVariant(item);
        }
        i++;
        if (i >= end)
            return data;
    }

    QMapIterator<ClientSocket *, QVariant> iter2(remoteRooms);
    while (iter2.hasNext()) {
        iter2.next();
        if (i >= offset)
            data << iter2.value();
        i++;
        if (i >= end)
            return data;
    }
    return data;
}
