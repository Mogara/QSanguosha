#include "LobbyPlayer.h"
#include "room.h"
#include "LobbyServer.h"

using namespace QSanProtocol;

LobbyPlayer::LobbyPlayer(LobbyServer *parent) :
    QObject(parent), server(parent), socket(NULL)
{
}

void LobbyPlayer::setSocket(ClientSocket *new_socket)
{
    if (socket != NULL) {
        socket->disconnect(this);
        this->disconnect(socket);
        socket->deleteLater();
    }

    if (new_socket) {
        socket = new_socket;
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    }
}

void LobbyPlayer::processMessage(const QByteArray &message)
{
    Packet packet;
    if (packet.parse(message)) {
        Callback func = callbacks.value(packet.getCommandType());
        if (func) {
            (this->*func)(packet.getMessageBody());
        } else {
            emit errorMessage(tr("Packet with an invalid command: %1, from %2(%3)")
                              .arg(QString::fromUtf8(message))
                              .arg(screenName)
                              .arg(socket->peerName()));
        }
    } else {
        emit errorMessage(tr("Invalid packet %1, from %2(%3)")
                          .arg(QString::fromUtf8(message))
                          .arg(screenName)
                          .arg(socket->peerName()));
    }
}
