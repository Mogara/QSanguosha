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

#include <QSqlQuery>

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
    if (args.size() != 2) return;

    QString setupString = args.at(0).toString();
    ushort hostPort = args.at(1).toUInt();

    //check if the room server can be connected
    QTcpSocket socket;
    socket.connectToHost(from->peerAddress(), hostPort);
    if (socket.waitForConnected(2000)) {
        ServerInfoStruct info;
        if (!info.parse(args.at(0).toString())) {
            emit serverMessage(tr("%1 Invalid setup string").arg(from->peerName()));
            return;
        }

        emit serverMessage(tr("%1 signed up as a Room Server on port %2").arg(from->peerName()).arg(hostPort));

        QSqlQuery query;
        query.prepare("INSERT INTO `room` (`hostaddress`, `setupstring`) VALUES (:hostaddress, :setupstring)");
        query.bindValue(":hostaddress", QString("%1:%2").arg(from->peerAddress()).arg(hostPort));
        query.bindValue(":setupstring", setupString);
        query.exec();

        remoteRoomId[from] = query.lastInsertId().toUInt();

        connect(from, &ClientSocket::disconnected, this, &Server::cleanupRemoteRoom);
    }
}

void Server::cleanupRemoteRoom()
{
    //no need to delete the socket for it's deleted in cleanup()
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (socket == NULL) return;

    emit serverMessage(tr("%1 Room Server disconnected").arg(socket->peerName()));
    unsigned id = remoteRoomId.value(socket);
    if (id > 0) {
        QSqlQuery query;
        query.prepare("DELETE FROM `room` WHERE `id`=?");
        query.addBindValue(id);
        query.exec();
    }
    remoteRoomId.remove(socket);
    socket->disconnect(this);
    this->disconnect(socket);
}

QVariant Server::getRoomList(int page)
{
    static const int limit = 10;
    int offset = (page - 1) * 10;

    QSqlQuery query;
    query.prepare("SELECT * FROM `room` WHERE 1 LIMIT :offset,:limit");
    query.bindValue(":offset", offset);
    query.bindValue(":limit", limit);
    query.exec();

    JsonArray data;
    while (query.next()) {
        JsonArray row;
        for(int i = 0; i < 3; i++)
            row << query.value(i);
        row << 0;
        if (row.at(2).isNull()) {
            Room *room = getRoom(row.at(0).toLongLong());
            if (room)
                row[3] = room->getPlayers().length();
        }
        data << QVariant(row);
    }

    return data;
}
