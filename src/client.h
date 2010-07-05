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
    void signup();
    const Player *getPlayer() const;

    Q_INVOKABLE void addPlayer(const QString &player_info);
    Q_INVOKABLE void removePlayer(const QString &player_name);
    Q_INVOKABLE void drawCards(const QString &cards_str);
    Q_INVOKABLE void getLords(const QString &lords_str);

public slots:
    void request(const QString &message);
    void itemChosen(const QString &item_name);

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
    void lords_got(const QList<const General*> &lords);
};

#endif // CLIENT_H
