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

#include "udpsocket.h"

#include <QUdpSocket>

UdpSocket::UdpSocket(QObject *parent)
    : m_socket(new QUdpSocket(this))
{
    setParent(parent);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpSocket::processNewDatagram);
}

void UdpSocket::bind(const QHostAddress &address, ushort port)
{
    m_socket->bind(address, port);
}

void UdpSocket::writeDatagram(const QByteArray &data, const QString &to)
{
    QHostAddress address(QHostAddress::Broadcast);
    ushort port = 0;
    if (to.contains(QChar(':'))) {
        QStringList texts = to.split(QChar(':'));
        address.setAddress(texts.at(0));
        port = texts.at(1).toUShort();
    } else {
        address.setAddress(to);
    }

    m_socket->writeDatagram(data, address, port);
}

void UdpSocket::writeDatagram(const QByteArray &data, const QHostAddress &to, ushort port)
{
    m_socket->writeDatagram(data, to, port);
}

void UdpSocket::processNewDatagram() {
    while (m_socket->hasPendingDatagrams()) {
        QHostAddress from;
        QByteArray data;
        quint16 port;

        data.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(data.data(), data.size(), &from, &port);
        emit newDatagram(data, from, port);
    }
}

