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

#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "abstractudpsocket.h"

class QUdpSocket;

class UdpSocket : public AbstractUdpSocket
{
    Q_OBJECT

public:
    UdpSocket(QObject *parent = 0);

    virtual void bind(const QHostAddress &address, ushort port);
    virtual void writeDatagram(const QByteArray &data, const QString &to);
    virtual void writeDatagram(const QByteArray &data, const QHostAddress &to, ushort port);

private slots:
    void processNewDatagram();

private:
    QUdpSocket *m_socket;
};

#endif // UDPSOCKET_H
