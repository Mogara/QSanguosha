#ifndef CLIENT_H
#define CLIENT_H

#include "player.h"
#include "card.h"

#include <QTcpSocket>

class Client : public QTcpSocket
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    void request(const QString &message);
    void signup();
    const Player *getPlayer() const;

    Q_INVOKABLE void addPlayer(const QString &player_info);
    Q_INVOKABLE void removePlayer(const QString &player_name);
    Q_INVOKABLE void drawCards(const QString &card_str);

private:
    QObject *room;
    Player *self;

private slots:
    void processReply();
    void raiseError(QAbstractSocket::SocketError socket_error);

signals:
    void error_message(const QString &msg);
    void player_added(Player *new_player);
    void player_removed(const QString &player_name);
    void cards_drawed(const QList<Card *> &cards);
};

#endif // CLIENT_H
