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

#ifndef LOBBYPLAYER_H
#define LOBBYPLAYER_H

#include "protocol.h"
#include "nativesocket.h"

class ClientSocket;
class Server;

#include <QObject>

class LobbyPlayer : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString screenName READ getScreenName WRITE setScreenName)
    Q_PROPERTY(QString avatar READ getAvatar WRITE setAvatar)

    explicit LobbyPlayer(Server *parent = 0);

    void setSocket(ClientSocket *new_socket);
    QString getIP() const { return socket != NULL ? socket->peerAddress() : QString(); }
    QString getSocketName() const { return socket != NULL ? socket->peerName() : QString(); }

    void notify(QSanProtocol::CommandType command, const QVariant &data = QVariant());
    void unicast(const QByteArray &message) { socket->send(message); }
    void unicast(const QSanProtocol::Packet *packet) { socket->send(packet->toJson()); }

    QString getScreenName() const { return screenName; }
    void setScreenName(const QString &name) { screenName = name; }

    QString getAvatar() const{ return avatar; }
    void setAvatar(const QString &new_avatar) { avatar = new_avatar; }

    void warn(QSanProtocol::WarningType warning) { notify(QSanProtocol::S_COMMAND_WARN, warning); }

public slots:
    void processMessage(const QByteArray &message);

signals:
    void disconnected();
    void errorMessage(const QString &message);

protected:
    void speakCommand(const QVariant &message);
    void roomListCommand(const QVariant &data);
    void createRoomCommand(const QVariant &data);
    void enterRoomCommand(const QVariant &data);

    Server *server;
    QString screenName;
    QString avatar;
    ClientSocket *socket;

    typedef void (LobbyPlayer::*Callback)(const QVariant &data);
    static QHash<QSanProtocol::CommandType, Callback> callbacks;
};

#endif // LOBBYPLAYER_H
