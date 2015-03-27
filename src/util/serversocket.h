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

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "abstractserversocket.h"

class QTcpServer;

class ServerSocket : public AbstractServerSocket
{
    Q_OBJECT

public:
    ServerSocket(QObject *parent = 0);

    virtual bool listen(const QHostAddress &address, ushort port = 0);
    virtual ushort serverPort() const;

private slots:
    void processNewConnection();

private:
    QTcpServer *m_server;
};

#endif // SERVERSOCKET_H
