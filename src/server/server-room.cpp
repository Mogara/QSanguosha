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
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "json.h"

using namespace QSanProtocol;

void Server::initLobbyFunctions()
{
    lobbyFunctions[S_COMMAND_CHECK_VERSION] = &Server::checkVersion;
    lobbyFunctions[S_COMMAND_SPEAK] = &Server::forwardLobbyMessage;

    serviceFunctions[S_SERVICE_DETECT_SERVER] = &Server::replyServerName;
    serviceFunctions[S_SERVICE_PLAYER_NUM] = &Server::replyPlayerNum;
}

void Server::connectToLobby()
{
    if (Config.LobbyAddress.isEmpty())
        return;

    lobby = new NativeClientSocket(this);
    connect(lobby, &NativeClientSocket::message_got, this, &Server::processMessage);
    //@todo: handle disconnection from lobby

    lobby->connectToHost(Config.LobbyAddress);
}

Room *Server::createNewRoom(const RoomConfig &config)
{
    Room *new_room = new Room(this, config);
    rooms.insert(new_room->getId(), new_room);

    connect(new_room, &Room::room_message, this, &Server::serverMessage);
    connect(new_room, &Room::game_end, this, &Server::cleanupRoom);

    return new_room;
}

Room *Server::getRoom(qlonglong room_id)
{
    return rooms.value(room_id);
}

void Server::processLobbyPacket(const Packet &packet)
{
    LobbyFunction func = lobbyFunctions[packet.getCommandType()];
    if (func) {
        (this->*func)(packet.getMessageBody());
    } else {
        emit serverMessage(tr("Lobby Packet %1 with an invalid command is discarded").arg(packet.toString()));
    }
}

void Server::signupPlayer(ServerPlayer *player) {
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void Server::cleanupRoom() {
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room->getId());

    bool someone_stays = false;
    QList<ServerPlayer *> room_players = room->getPlayers();
    foreach (ServerPlayer *player, room_players) {
        if (player->getState() == "online" || player->getState() == "trust") {
            someone_stays = true;
            break;
        }
    }

    foreach (ServerPlayer *player, room_players) {
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }

    if (someone_stays) {
        Room *new_room = createNewRoom(room->getConfig());
        foreach (ServerPlayer *player, room_players) {
            ClientSocket *socket = player->takeSocket();
            if (socket) {
                ServerPlayer *new_player = new_room->addSocket(socket);
                new_player->setObjectName(player->objectName());
                new_player->setScreenName(player->screenName());
                new_player->setProperty("avatar", player->property("avatar"));
                new_player->setOwner(player->isOwner());
                new_player->setState("absent");
            }
        }
    }

    room->deleteLater();

    currentRoomMutex.lock();
    if (current == room)
        current = NULL;
    currentRoomMutex.unlock();
}

void Server::notifyLobby(CommandType command, const QVariant &data)
{
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_LOBBY, command);
    packet.setMessageBody(data);
    lobby->send(packet.toJson());
}

void Server::checkVersion(const QVariant &server_version)
{
    QString version = server_version.toString();
    QString lobby_version_str, lobby_mod;
    if (version.contains(QChar(':'))) {
        QStringList texts = version.split(QChar(':'));
        lobby_version_str = texts.value(0);
        lobby_mod = texts.value(1);
    } else {
        lobby_version_str = version;
        lobby_mod = "official";
    }

    QString client_mod = Sanguosha->getMODName();
    if (client_mod != lobby_mod) {
        emit serverMessage(tr("Failed to setup for the MOD name is not the same as the lobby."));
        return;
    }

    const QSanVersionNumber &client_version = Sanguosha->getVersionNumber();
    QSanVersionNumber lobby_version(lobby_version_str);

    if (lobby_version == client_version) {
        JsonArray data;
        data << Sanguosha->getSetupString();
        data << serverPort();
        notifyLobby(S_COMMAND_SETUP, data);
    } else {
        emit serverMessage(tr("Failed to setup for the version is different from the lobby."));
        lobby->disconnectFromHost();
    }
}

void Server::forwardLobbyMessage(const QVariant &message)
{
    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_SPEAK);
    packet.setMessageBody(message);

    foreach (Room *room, rooms)
        room->broadcast(&packet);
}

void Server::replyServerName(const QByteArray &, const QHostAddress &from, ushort port)
{
    QByteArray data(1, S_SERVICE_DETECT_SERVER);
    data.append(Config.ServerName.toUtf8());
    daemon->writeDatagram(data, from, port);
}

void Server::replyPlayerNum(const QByteArray &, const QHostAddress &from, ushort port)
{
    QByteArray data(5, S_SERVICE_PLAYER_NUM);
    int playerNum = current != NULL ? current->getPlayers().size() : 0;
    for (int i = 4; i >= 1; i--) {
        data[i] = playerNum;
        playerNum >>= 8;
    }
    daemon->writeDatagram(data, from, port);
}
