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

#include "lobbyplayer.h"
#include "server.h"
#include "json.h"

using namespace QSanProtocol;

QHash<CommandType, LobbyPlayer::Callback> LobbyPlayer::callbacks;

LobbyPlayer::LobbyPlayer(Server *parent) :
    QObject(parent), server(parent), socket(NULL)
{
    if (callbacks.isEmpty()) {
        callbacks[S_COMMAND_SPEAK] = &LobbyPlayer::speakCommand;
        callbacks[S_COMMAND_ROOM_LIST] = &LobbyPlayer::roomListCommand;
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
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(message_got(QByteArray)), SLOT(processMessage(QByteArray)));
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
            emit errorMessage(tr("Packet with an invalid command: %1, from %2(%3)")
                              .arg(QString::fromUtf8(message))
                              .arg(screenName)
                              .arg(socket->peerName()));
        }
    } else {
        emit errorMessage(tr("Invalid packet %1, from %2(%3)")
                          .arg(QString::fromUtf8(message))
                          .arg(screenName)
                          .arg(socket->peerName()));
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
