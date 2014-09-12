#include "LobbyPlayer.h"
#include "room.h"
#include "LobbyServer.h"
#include "json.h"

using namespace QSanProtocol;

LobbyPlayer::LobbyPlayer(LobbyServer *parent) :
    QObject(parent), server(parent), socket(NULL)
{
    callbacks[S_COMMAND_SPEAK] = &LobbyPlayer::speakCommand;
    callbacks[S_COMMAND_ROOM_LIST] = &LobbyPlayer::roomListCommand;
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
        connect(socket, SIGNAL(message_got(QByteArray)), SLOT(processMessage(QByteArray)));
    }
}

void LobbyPlayer::notify(CommandType command, const QVariant &data)
{
    Packet packet(S_SRC_LOBBY | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(data);
    unicast(packet.toJson());
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

void LobbyPlayer::speakCommand(const QVariant &message)
{
    JsonArray body;
    body << screenName;
    body << message;
    server->broadcastNotification(S_COMMAND_SPEAK, body);
}

void LobbyPlayer::roomListCommand(const QVariant &data)
{
    int page = data.toInt();
    server->sendRoomListTo(this, page);
}
