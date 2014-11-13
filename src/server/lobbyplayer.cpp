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

#include "lobbyplayer.h"
#include "server.h"
#include "json.h"
#include "room.h"
#include "engine.h"
#include "settings.h"

using namespace QSanProtocol;

QHash<CommandType, LobbyPlayer::Callback> LobbyPlayer::callbacks;

LobbyPlayer::LobbyPlayer(Server *parent) :
    QObject(parent), server(parent), socket(NULL)
{
    if (callbacks.isEmpty()) {
        callbacks[S_COMMAND_SPEAK] = &LobbyPlayer::speakCommand;
        callbacks[S_COMMAND_ROOM_LIST] = &LobbyPlayer::roomListCommand;
        callbacks[S_COMMAND_CREATE_ROOM] = &LobbyPlayer::createRoomCommand;
        callbacks[S_COMMAND_ENTER_ROOM] = &LobbyPlayer::enterRoomCommand;
    }
}

void LobbyPlayer::setSocket(ClientSocket *new_socket)
{
    if (socket != NULL) {
        socket->disconnect(this);
        this->disconnect(socket);
        socket->deleteLater();
    }

    if (new_socket) {
        socket = new_socket;
        connect(socket, &ClientSocket::disconnected, this, &LobbyPlayer::disconnected);
        connect(socket, &ClientSocket::message_got, this, &LobbyPlayer::processMessage);
    }
}

void LobbyPlayer::notify(CommandType command, const QVariant &data)
{
    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(data);
    unicast(packet.toJson());
}

void LobbyPlayer::processMessage(const QByteArray &message)
{
    Packet packet;
    if (packet.parse(message)) {
        Callback func = callbacks.value(packet.getCommandType());
        if (func) {
            (this->*func)(packet.getMessageBody());
        } else {
            emit errorMessage(tr("%1 %2 send a packet with an invalid command: %3")
                              .arg(socket->peerName())
                              .arg(screenName)
                              .arg(QString::fromUtf8(message)));
        }
    } else {
        emit errorMessage(tr("%1 %2 send an invalid packet %3")
                          .arg(socket->peerName())
                          .arg(screenName)
                          .arg(QString::fromUtf8(message)));
    }
}

void LobbyPlayer::speakCommand(const QVariant &message)
{
    JsonArray body;
    body << screenName;
    body << message;
    server->broadcastNotification(S_COMMAND_SPEAK, body);
}

void LobbyPlayer::roomListCommand(const QVariant &data)
{
    notify(S_COMMAND_ROOM_LIST, server->getRoomList(data.toInt()));
}

void LobbyPlayer::createRoomCommand(const QVariant &data)
{
    socket->disconnect(this);
    this->disconnect(socket);

    RoomConfig config;
    if (!config.parse(data))
        return;
    if (Config.ForbidAddingRobot)
        config.ForbidAddingRobot = true;
    QStringList global_ban_packages(Sanguosha->getBanPackages());
    foreach (const QString &package, global_ban_packages) {
        config.BanPackages.insert(package);
    }

    Room *room = server->createNewRoom(config);
    ServerPlayer *player = room->addSocket(socket);
    socket = NULL;
    room->signup(player, screenName, avatar, false);

    emit disconnected();
    deleteLater();
}

void LobbyPlayer::enterRoomCommand(const QVariant &data)
{
    JsonArray args = data.value<JsonArray>();
    if (args.isEmpty())
        return;

    int room_id = args.at(0).toInt();
    Room *room = server->getRoom(room_id);

    if (!room->getConfig().Password.isEmpty()) {
        QString password;
        if (args.size() >= 2) {
            password = args.at(1).toString();
            password = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Md5).toHex();
        }

        if (password != room->getConfig().Password) {
            warn(S_WARNING_WRONG_PASSWORD);
            return;
        }
    }

    if (room == NULL || room->isFinished() || room->getPlayers().isEmpty()) {
        warn(S_WARNING_GAME_OVER);
        return;
    } else if (room->isFull()) {
        warn(S_WARNING_ROOM_IS_FULL);
        return;
    }

    ServerPlayer *player = room->addSocket(socket);
    if (player == NULL) {
        warn(S_WARNING_ROOM_IS_FULL);
        return;
    }

    socket->disconnect(this);
    this->disconnect(socket);
    socket = NULL;

    room->signup(player, screenName, avatar, false);

    emit disconnected();
    deleteLater();
}
