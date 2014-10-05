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

#include "roomserver.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "scenario.h"
#include "FreeChooseDialog.h"
#include "customassigndialog.h"
#include "miniscenarios.h"
#include "SkinBank.h"
#include "banpair.h"

#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QApplication>
#include <QHostInfo>
#include <QAction>

using namespace QSanProtocol;

QHash<CommandType, RoomServer::Callback> RoomServer::callbacks;

RoomServer::RoomServer(QObject *parent)
    : Server(parent), current(NULL), lobby(NULL)
{
    packetSource = S_SRC_ROOM;

    if (callbacks.isEmpty()) {
        callbacks[S_COMMAND_CHECK_VERSION] = &RoomServer::checkVersion;
    }
}

void RoomServer::connectToLobby()
{
    if (Config.LobbyAddress.isEmpty())
        return;

    lobby = new NativeClientSocket;
    lobby->setParent(this);
    connect(lobby, SIGNAL(message_got(QByteArray)), SLOT(processMessage(QByteArray)));
    //@todo: handle disconnection from lobby

    lobby->connectToHost(Config.LobbyAddress);
}

void RoomServer::broadcastSystemMessage(const QString &msg) {
    JsonArray arg;
    arg << ".";
    arg << msg;

    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_SPEAK);
    packet.setMessageBody(arg);

    foreach(Room *room, rooms)
        room->broadcast(&packet);
}

Room *RoomServer::createNewRoom() {
    Room *new_room = new Room(this, Config.GameMode);
    current = new_room;
    rooms.insert(current);

    connect(current, SIGNAL(room_message(QString)), this, SIGNAL(serverMessage(QString)));
    connect(current, SIGNAL(game_over(QString)), this, SLOT(gameOver()));

    return current;
}

void RoomServer::_processNewConnection(ClientSocket *socket) {
    notifyClient(socket, S_COMMAND_SETUP, Sanguosha->getSetupString());
    emit serverMessage(tr("%1 connected").arg(socket->peerName()));
    connect(socket, SIGNAL(message_got(QByteArray)), this, SLOT(processMessage(QByteArray)));
}

void RoomServer::processMessage(const QByteArray &message)
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
    case S_SRC_LOBBY:
        if (socket == lobby) {
            processLobbyPacket(packet);
        }
        break;
    default:
        emit serverMessage(tr("Packet %1 from an unknown source %2").arg(QString::fromUtf8(message)).arg(socket->peerName()));
    }
}

void RoomServer::processClientSignup(ClientSocket *socket, const Packet &signup)
{
    socket->disconnect(this, SLOT(processMessage(QByteArray)));

    if (signup.getCommandType() != S_COMMAND_SIGNUP) {
        emit serverMessage(tr("Invalid signup string: %1").arg(signup.toString()));
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

void RoomServer::processLobbyPacket(const Packet &packet)
{
    Callback func = callbacks[packet.getCommandType()];
    if (func) {
        (this->*func)(packet.getMessageBody());
    } else {
        emit serverMessage(tr("Lobby Packet %1 with an invalid command is discarded").arg(packet.toString()));
    }
}

void RoomServer::signupPlayer(ServerPlayer *player) {
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void RoomServer::gameOver() {
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room);

    foreach(ServerPlayer *player, room->findChildren<ServerPlayer *>()) {
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }
}

void RoomServer::checkVersion(const QVariant &server_version)
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
        setup();
    } else {
        emit serverMessage(tr("Failed to setup for the version is different from the lobby."));
        lobby->disconnectFromHost();
    }
}

void RoomServer::notifyLobby(CommandType command, const QVariant &data)
{
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_LOBBY, command);
    packet.setMessageBody(data);
    lobby->send(packet.toJson());
}

void RoomServer::setup()
{
    JsonArray data;
    data << Sanguosha->getSetupString();
    data << serverPort();
    data << players.size();
    data << rooms.size();
    data << -1;//unlimited room number
    data << Config.AIDelay;
    data << Config.RewardTheFirstShowingPlayer;
    notifyLobby(S_COMMAND_SETUP, data);
}
