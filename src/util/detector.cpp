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

#include "detector.h"
#include "settings.h"
#include "protocol.h"

using namespace QSanProtocol;

UdpDetector::UdpDetector() {
    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead, this, &UdpDetector::onReadReady);
}

void UdpDetector::detect() {
    socket->bind(0, QUdpSocket::ShareAddress);

    char ask_str = S_SERVICE_DETECT_SERVER;
    socket->writeDatagram(&ask_str,
        1,
        QHostAddress::Broadcast,
        Config.ServerPort);
}

void UdpDetector::stop() {
    socket->close();
}

void UdpDetector::onReadReady() {
    char data[256];
    int length;

    while (socket->hasPendingDatagrams()) {
        QHostAddress from;
        length = socket->readDatagram(data, 256, &from);

        if (length > 1) {
            QString server_name = QString::fromUtf8(data + 1, length - 1);
            emit detected(server_name, from.toString());
        }
    }
}
