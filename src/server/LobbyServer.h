/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#ifndef LOBBYSERVER_H
#define LOBBYSERVER_H

#include "Server.h"

class LobbyPlayer;

class LobbyServer : public Server
{
    Q_OBJECT

public:
    LobbyServer(QObject *parent);

    void broadcastSystemMessage(const QString &message);
    void broadcastNotification(QSanProtocol::CommandType command, const QVariant &data = QVariant(), int destination = QSanProtocol::S_DEST_CLIENT);
    void broadcast(const QSanProtocol::Packet *packet);
    void broadcast(const QByteArray &message, int destination = QSanProtocol::S_DEST_CLIENT);

protected:
    void _processNewConnection(ClientSocket *socket);
    void processClientSignup(ClientSocket *socket, const QSanProtocol::Packet &signup);

    struct RoomInfoStruct{
        QString SetupString;
        QString Address;
        ushort Port;
        int PlayerNum;
        int RoomNum;
        int MaxRoomNum;
    };

    QList<LobbyPlayer *> players;
    QMap<ClientSocket *, RoomInfoStruct *> rooms;

protected slots:
    void processMessage(const QByteArray &message);
    void cleanupPlayer();
};

#endif // LOBBYSERVER_H
