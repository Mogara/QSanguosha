#ifndef LOBBYPLAYER_H
#define LOBBYPLAYER_H

#include "protocol.h"
#include "nativesocket.h"

class ClientSocket;
class LobbyServer;

#include <QObject>

class LobbyPlayer : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString screenName READ getScreenName WRITE setScreenName)
    Q_PROPERTY(QString avatar READ getAvatar WRITE setAvatar)

    explicit LobbyPlayer(LobbyServer *parent = 0);

    void setSocket(ClientSocket *new_socket);
    QString getIP() const { return socket->peerAddress(); }
    QString getSocketName() const { return socket->peerName(); }

    void notify(QSanProtocol::CommandType command, const QVariant &data = QVariant());
    void unicast(const QByteArray &message) { socket->send(message); }
    void unicast(const QSanProtocol::Packet *packet) { socket->send(packet->toJson()); }

    QString getScreenName() const { return screenName; }
    void setScreenName(const QString &name) { screenName = name; }

    QString getAvatar() const{ return avatar; }
    void setAvatar(const QString &new_avatar) { avatar = new_avatar; }

public slots:
    void processMessage(const QByteArray &message);

signals:
    void disconnected();
    void errorMessage(const QString &message);

protected:
    void speakCommand(const QVariant &message);
    void roomListCommand(const QVariant &data);

    LobbyServer *server;
    QString screenName;
    QString avatar;
    ClientSocket *socket;

    typedef void (LobbyPlayer::*Callback)(const QVariant &data);
    static QHash<QSanProtocol::CommandType, Callback> callbacks;
};

#endif // LOBBYPLAYER_H
