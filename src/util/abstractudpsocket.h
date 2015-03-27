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

#ifndef ABSTRACTUDPSOCKET_H
#define ABSTRACTUDPSOCKET_H

#include <QHostAddress>

class AbstractUdpSocket : public QObject
{
    Q_OBJECT

public:
    virtual void bind(const QHostAddress &address, ushort port) = 0;
    virtual void writeDatagram(const QByteArray &data, const QString &to) = 0;
    virtual void writeDatagram(const QByteArray &data, const QHostAddress &to, ushort port) = 0;

signals:
    void newDatagram(const QByteArray &data, const QHostAddress &from, ushort port);
};

#endif // ABSTRACTUDPSOCKET_H

