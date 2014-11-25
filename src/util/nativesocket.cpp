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

#include "nativesocket.h"
#include "settings.h"
#include "clientplayer.h"

#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QUdpSocket>
#include <QTimer>

const qint64 NativeClientSocket::KEEP_ALIVE_INTERVAL = 30000;
const qint64 NativeClientSocket::TIMEOUT_LIMIT = 10000;

NativeServerSocket::NativeServerSocket(QObject *parent)
{
    setParent(parent);
    server = new QTcpServer(this);
    daemon = NULL;
    connect(server, &QTcpServer::newConnection, this, &NativeServerSocket::processNewConnection);
}

bool NativeServerSocket::listen(const QHostAddress &address, ushort port)
{
    return server->listen(address, port);
}

ushort NativeServerSocket::serverPort() const
{
    return server->serverPort();
}

void NativeServerSocket::processNewConnection() {
    QTcpSocket *socket = server->nextPendingConnection();
    NativeClientSocket *connection = new NativeClientSocket(socket);
    emit new_connection(connection);
}

// ---------------------------------

NativeClientSocket::NativeClientSocket(QObject *parent)
    : socket(new QTcpSocket(this))
{
    setParent(parent);
    init();
}

NativeClientSocket::NativeClientSocket(QTcpSocket *socket, QObject *parent)
    : socket(socket)
{
    setParent(parent);
    socket->setParent(this);
    init();
}

void NativeClientSocket::init() {
    connect(socket, &QTcpSocket::disconnected, this, &NativeClientSocket::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NativeClientSocket::getMessage);
    connect(socket, (void (QTcpSocket::*)(QAbstractSocket::SocketError))(&QTcpSocket::error), this, &NativeClientSocket::raiseError);
    connect(socket, &QTcpSocket::connected, this, &NativeClientSocket::connected);

    is_alive = false;

    keep_alive_timer = new QTimer(this);
    keep_alive_timer->setSingleShot(false);
    keep_alive_timer->setInterval(KEEP_ALIVE_INTERVAL);
    keep_alive_timer->start();
    connect(keep_alive_timer, &QTimer::timeout, this, &NativeClientSocket::keepAlive);
}

void NativeClientSocket::connectToHost(const QString &address)
{
    if (address.contains(':')) {
        QStringList texts = address.split(':');
        socket->connectToHost(texts.first(), texts.at(1).toUInt());
    } else {
        ushort port = Config.value("ServerPort", 9527u).toUInt();
        socket->connectToHost(address, port);
    }
}

void NativeClientSocket::connectToHost(const QHostAddress &address, ushort port)
{
    socket->connectToHost(address, port);
}

void NativeClientSocket::getMessage() {
    is_alive = true;
    keep_alive_timer->start();

    char type;
    while (socket->read(&type, 1) == 1) {
        switch (type) {
        case InlineTextPacket:
            if (socket->canReadLine()) {
                QByteArray text = socket->readLine();
        #ifndef QT_NO_DEBUG
                printf("recv: %s", text.constData());
        #endif
                emit message_got(text);
            } else {
                socket->ungetChar(type);
                return;
            }
            break;
        case KeepAlivePacket:
            socket->putChar(InlineTextPacket);
            socket->flush();
            break;
        default:
            qDebug() << "Unknown packet: " << type;
        }
    }
}

void NativeClientSocket::disconnectFromHost() {
    socket->disconnectFromHost();
}

void NativeClientSocket::send(const QByteArray &message) {
    socket->putChar(InlineTextPacket);
    socket->write(message);
    if (!message.endsWith('\n')){
        socket->putChar('\n');
    }

#ifndef QT_NO_DEBUG
    printf(": %s\n", message.constData());
#endif
    socket->flush();
}

bool NativeClientSocket::isConnected() const{
    return socket->state() == QTcpSocket::ConnectedState;
}

QString NativeClientSocket::peerName() const{
    QString peer_name = socket->peerName();
    if (peer_name.isEmpty())
        peer_name = QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());

    return peer_name;
}

QString NativeClientSocket::peerAddress() const{
    return socket->peerAddress().toString();
}

ushort NativeClientSocket::peerPort() const{
    return socket->peerPort();
}

void NativeClientSocket::raiseError(QAbstractSocket::SocketError socket_error) {
    // translate error message
    QString reason;
    switch (socket_error) {
    case QAbstractSocket::ConnectionRefusedError:
        reason = tr("Connection was refused or timeout"); break;
    case QAbstractSocket::RemoteHostClosedError:{
        if (Self && Self->hasFlag("is_kicked"))
            reason = tr("You are kicked from server");
        else
            reason = tr("Remote host close this connection");
        break;
    }
    case QAbstractSocket::HostNotFoundError:
        reason = tr("Host not found"); break;
    case QAbstractSocket::SocketAccessError:
        reason = tr("Socket access error"); break;
    case QAbstractSocket::NetworkError:
        return; // this error is ignored ...
    default: reason = tr("Unknown error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}

void NativeClientSocket::keepAlive()
{
    is_alive = false;
    socket->putChar(KeepAlivePacket);
    QTimer::singleShot(TIMEOUT_LIMIT, this, SLOT(checkConnectionState()));
}

void NativeClientSocket::checkConnectionState()
{
    if (!is_alive) {
        socket->abort();
        keep_alive_timer->stop();
    }
}

NativeUdpSocket::NativeUdpSocket(QObject *parent)
    : socket(new QUdpSocket(this))
{
    setParent(parent);
    connect(socket, &QUdpSocket::readyRead, this, &NativeUdpSocket::processNewDatagram);
}

void NativeUdpSocket::bind(const QHostAddress &address, ushort port)
{
    socket->bind(address, port);
}

void NativeUdpSocket::writeDatagram(const QByteArray &data, const QString &to)
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

    socket->writeDatagram(data, address, port);
}

void NativeUdpSocket::writeDatagram(const QByteArray &data, const QHostAddress &to, ushort port)
{
    socket->writeDatagram(data, to, port);
}

void NativeUdpSocket::processNewDatagram() {
    while (socket->hasPendingDatagrams()) {
        QHostAddress from;
        QByteArray data;
        quint16 port;

        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size(), &from, &port);
        emit new_datagram(data, from, port);
    }
}
