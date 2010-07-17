#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"

#include <QTcpSocket>

class Client : public QTcpSocket
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    void signup();
    const ClientPlayer *getPlayer() const;
    void request(const QString &message);

    Q_INVOKABLE void addPlayer(const QString &player_info);
    Q_INVOKABLE void removePlayer(const QString &player_name);
    Q_INVOKABLE void drawCards(const QString &cards_str);
    Q_INVOKABLE void drawNCards(const QString &draw_str);
    Q_INVOKABLE void getLords(const QString &lords_str);
    Q_INVOKABLE void getGenerals(const QString &generals_str);
    Q_INVOKABLE void startInXs(const QString &);
    Q_INVOKABLE void duplicationError(const QString &);
    Q_INVOKABLE void arrangeSeats(const QString &seats);
    Q_INVOKABLE void activate(const QString &player_name);
    Q_INVOKABLE void moveCard(const QString &move_str);

public slots:    
    void itemChosen(const QString &item_name);
    void useCard(const Card *card);

private:
    QObject *room;
    ClientPlayer *self;
    ClientPlayer *focus_player;

private slots:
    void processReply();
    void raiseError(QAbstractSocket::SocketError socket_error);
    void notifyRoleChange(const QString &new_role);

signals:
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);
    void cards_drawed(const QList<Card *> &cards);
    void lords_got(const QList<const General*> &lords);
    void generals_got(const General *lord, const QList<const General *> &generals);
    void prompt_changed(const QString &prompt_str);
    void seats_arranged(const QList<const ClientPlayer*> &seats);
    void n_card_drawed(ClientPlayer *player, int n);
    void activity_set(bool active);
    void card_moved(const QString &src, const QString &dest, int card_id);
};

#endif // CLIENT_H
