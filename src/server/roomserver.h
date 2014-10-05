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

#ifndef _SERVER_H
#define _SERVER_H

class Room;
class QGroupBox;
class QLabel;
class QRadioButton;

#include "socket.h"
#include "detector.h"
#include "clientstruct.h"
#include "server.h"

class Package;
class Scenario;
class ServerPlayer;
class BanIpDialog;

class RoomServer : public Server {
    Q_OBJECT

public:
    friend class BanIpDialog;

    explicit RoomServer(QObject *parent);

    void connectToLobby();
    void broadcastSystemMessage(const QString &msg);

    Room *createNewRoom();
    void signupPlayer(ServerPlayer *player);

protected slots:
    void processMessage(const QByteArray &message);
    void gameOver();

protected:
    void _processNewConnection(ClientSocket *socket);
    void processClientSignup(ClientSocket *socket, const QSanProtocol::Packet &signup);
    void processLobbyPacket(const QSanProtocol::Packet &packet);
    void notifyLobby(QSanProtocol::CommandType command, const QVariant &data = QVariant());
    void setup();

    //callbacks for lobby server
    void checkVersion(const QVariant &server_version);

    ServerSocket *server;
    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer *> players;
    QMultiHash<QString, QString> name2objname;

    ClientSocket *lobby;
    typedef void (RoomServer::*Callback)(const QVariant &);
    static QHash<QSanProtocol::CommandType, Callback> callbacks;

signals:
    void newPlayer(ServerPlayer *player);
};

#endif
