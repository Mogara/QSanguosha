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

#include <QApplication>

UdpDetector::UdpDetector() {
    socket = new QUdpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadReady()));
}

void UdpDetector::detect() {
    socket->bind(Config.DetectorPort, QUdpSocket::ShareAddress);

    const char *ask_str = "whoIsServer";
    socket->writeDatagram(ask_str,
        strlen(ask_str) + 1,
        QHostAddress::Broadcast,
        Config.ServerPort);
}

void UdpDetector::stop() {
    socket->close();
}

void UdpDetector::onReadReady() {
    while (socket->hasPendingDatagrams()) {
        QHostAddress from;
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size(), &from);

        QString server_name = QString::fromUtf8(data);
        emit detected(server_name, from.toString());
    }
}

