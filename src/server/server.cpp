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
#include "nativesocket.h"
#include "clientstruct.h"
#include "json.h"
#include "room.h"
#include "settings.h"
#include "engine.h"
#include "scenario.h"

#include <QApplication>

using namespace QSanProtocol;

Server::Server(QObject *parent)
    : QObject(parent)
{
    server = new NativeServerSocket;
    server->setParent(this);

    //synchronize ServerInfo on the server side to avoid ambiguous usage of Config and ServerInfo
    ServerInfo.parse(Sanguosha->getSetupString());

    current = NULL;

    connect(server, SIGNAL(new_connection(ClientSocket *)), this, SLOT(processNewConnection(ClientSocket *)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Server::broadcastSystemMessage(const QString &msg) {
    JsonArray arg;
    arg << ".";
    arg << msg;

    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_SPEAK);
    packet.setMessageBody(arg);

    foreach(Room *room, rooms)
        room->broadcast(&packet);
}

bool Server::listen() {
    return server->listen();
}

void Server::daemonize() {
    server->daemonize();
}

Room *Server::createNewRoom() {
    Room *new_room = new Room(this, Config.GameMode);
    current = new_room;
    rooms.insert(current);

    connect(current, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    connect(current, SIGNAL(game_over(QString)), this, SLOT(gameOver()));

    return current;
}

void Server::processNewConnection(ClientSocket *socket) {
    QString address = socket->peerAddress();
    if (Config.ForbidSIMC) {
        if (addresses.contains(address)) {
            addresses.append(address);
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(address));
            return;
        } else {
            addresses.append(address);
        }
    }

    if (Config.value("BannedIP").toStringList().contains(address)){
        socket->disconnectFromHost();
        emit server_message(tr("Forbid the connection of address %1").arg(address));
        return;
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));

    notifyClient(socket, S_COMMAND_CHECK_VERSION, Sanguosha->getVersion());
    notifyClient(socket, S_COMMAND_SETUP, Sanguosha->getSetupString());

    emit server_message(tr("%1 connected").arg(socket->peerName()));

    connect(socket, SIGNAL(message_got(QByteArray)), this, SLOT(processRequest(QByteArray)));
}

void Server::processRequest(const QByteArray &request)
{
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());

    Packet packet;
    if (!packet.parse(request)) {
        emit server_message(tr("Invalid message %1 from %2").arg(QString::fromUtf8(request)).arg(socket->peerAddress()));
        return;
    }

    switch (packet.getPacketSource()) {
    case S_SRC_CLIENT:
        processClientRequest(socket, packet);
        break;
    default:
        emit server_message(tr("Packet %1 from an unknown source %2").arg(QString::fromUtf8(request)).arg(socket->peerAddress()));
    }
}

void Server::notifyClient(ClientSocket *socket, CommandType command, const QVariant &arg)
{
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(arg);
    socket->send(packet.toJson());
}

void Server::processClientRequest(ClientSocket *socket, const Packet &signup)
{
    socket->disconnect(this, SLOT(processRequest(QByteArray)));

    if (signup.getCommandType() != S_COMMAND_SIGNUP) {
        emit server_message(tr("Invalid signup string: %1").arg(signup.toString()));
        notifyClient(socket, S_COMMAND_WARN, "INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    JsonArray body = signup.getMessageBody().value<JsonArray>();
    bool is_reconnection = body[0].toBool();
    QString screen_name = body[1].toString();
    QString avatar = body[2].toString();

    if (is_reconnection) {
        foreach (QString objname, name2objname.values(screen_name)) {
            ServerPlayer *player = players.value(objname);
            if (player && player->getState() == "offline" && !player->getRoom()->isFinished()) {
                player->getRoom()->reconnect(player, socket);
                return;
            }
        }
    }

    if (current == NULL || current->isFull() || current->isFinished())
        createNewRoom();

    ServerPlayer *player = current->addSocket(socket);
    current->signup(player, screen_name, avatar, false);
    emit newPlayer(player);

    if (current->getPlayers().length() == 1 && current->getScenario() && current->getScenario()->objectName() == "jiange_defense") {
        for (int i = 0; i < 4; ++i)
            current->addRobotCommand(player, QVariant());
    }
}

void Server::cleanup() {
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (Config.ForbidSIMC)
        addresses.removeOne(socket->peerAddress());

    socket->deleteLater();
}

void Server::signupPlayer(ServerPlayer *player) {
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void Server::gameOver() {
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room);

    foreach(ServerPlayer *player, room->findChildren<ServerPlayer *>()) {
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }
}
