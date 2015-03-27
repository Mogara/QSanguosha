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

#include "clientsocket.h"
#include "settings.h"

#include <QTcpSocket>
#include <QTimer>

const qint64 ClientSocket::KEEP_ALIVE_INTERVAL = 30000;
const qint64 ClientSocket::TIMEOUT_LIMIT = 10000;

ClientSocket::ClientSocket(QObject *parent)
    : m_socket(new QTcpSocket(this))
{
    setParent(parent);
    init();
}

ClientSocket::ClientSocket(QTcpSocket *socket, QObject *parent)
    : m_socket(socket)
{
    setParent(parent);
    socket->setParent(this);
    init();
}

void ClientSocket::init() {
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientSocket::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientSocket::getMessage);
    connect(m_socket, (void (QTcpSocket::*)(QAbstractSocket::SocketError))(&QTcpSocket::error), this, &ClientSocket::raiseError);
    connect(m_socket, &QTcpSocket::connected, this, &ClientSocket::connected);

    m_isAlive = false;

    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setSingleShot(false);
    m_keepAliveTimer->setInterval(KEEP_ALIVE_INTERVAL);
    m_keepAliveTimer->start();
    connect(m_keepAliveTimer, &QTimer::timeout, this, &ClientSocket::keepAlive);
}

void ClientSocket::connectToHost(const QString &address)
{
    if (address.contains(':')) {
        QStringList texts = address.split(':');
        m_socket->connectToHost(texts.first(), texts.at(1).toUInt());
    } else {
        ushort port = Config.value("ServerPort", 9527u).toUInt();
        m_socket->connectToHost(address, port);
    }
}

void ClientSocket::connectToHost(const QHostAddress &address, ushort port)
{
    m_socket->connectToHost(address, port);
}

void ClientSocket::getMessage() {
    m_isAlive = true;
    m_keepAliveTimer->start();

    char type;
    while (m_socket->read(&type, 1) == 1) {
        switch (type) {
        case InlineTextPacket:
            if (m_socket->canReadLine()) {
                QByteArray text = m_socket->readLine();
        #ifndef QT_NO_DEBUG
                printf("recv: %s", text.constData());
        #endif
                emit messageGot(text);
            } else {
                m_socket->ungetChar(type);
                return;
            }
            break;
        case KeepAlivePacket:
            m_socket->putChar(AcknowledgePacket);
            m_socket->flush();
            break;
        case AcknowledgePacket:
            break;
        default:
            qDebug() << "Unknown packet: " << type;
        }
    }
}

void ClientSocket::disconnectFromHost() {
    m_socket->disconnectFromHost();
}

void ClientSocket::send(const QByteArray &message) {
    m_socket->putChar(InlineTextPacket);
    m_socket->write(message);
    if (!message.endsWith('\n')){
        m_socket->putChar('\n');
    }

#ifndef QT_NO_DEBUG
    printf(": %s\n", message.constData());
#endif
    m_socket->flush();
}

bool ClientSocket::isConnected() const{
    return m_socket->state() == QTcpSocket::ConnectedState;
}

QString ClientSocket::peerName() const{
    QString peer_name = m_socket->peerName();
    if (peer_name.isEmpty())
        peer_name = QString("%1:%2").arg(m_socket->peerAddress().toString()).arg(m_socket->peerPort());

    return peer_name;
}

QString ClientSocket::peerAddress() const{
    return m_socket->peerAddress().toString();
}

ushort ClientSocket::peerPort() const{
    return m_socket->peerPort();
}

void ClientSocket::raiseError(QAbstractSocket::SocketError socketError) {
    // translate error message
    QString reason;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        reason = tr("Connection was refused or timeout"); break;
    case QAbstractSocket::RemoteHostClosedError:
        reason = tr("Remote host close this connection"); break;
    case QAbstractSocket::HostNotFoundError:
        reason = tr("Host not found"); break;
    case QAbstractSocket::SocketAccessError:
        reason = tr("Socket access error"); break;
    case QAbstractSocket::NetworkError:
        return; // this error is ignored ...
    default: reason = tr("Unknown error"); break;
    }

    emit errorMessage(tr("Connection failed, error code = %1\n reason:\n %2").arg(socketError).arg(reason));
}

void ClientSocket::keepAlive()
{
    m_isAlive = false;
    m_socket->putChar(KeepAlivePacket);
    QTimer::singleShot(TIMEOUT_LIMIT, this, SLOT(checkConnectionState()));
}

void ClientSocket::checkConnectionState()
{
    if (!m_isAlive) {
        m_socket->abort();
        m_keepAliveTimer->stop();
    }
}

