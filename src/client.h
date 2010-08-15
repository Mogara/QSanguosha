#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"
#include "cardpattern.h"

#include <QTcpSocket>

class Client : public QTcpSocket
{
    Q_OBJECT
    Q_PROPERTY(Client::Status status READ getStatus WRITE setStatus)

    Q_ENUMS(Status)

public:
    enum Status{
        NotActive,
        Responsing,
        Playing,
        Discarding
    };

    explicit Client(QObject *parent = 0);
    void signup();
    const ClientPlayer *getPlayer() const;
    void request(const QString &message);
    void useCard(const Card *card, const QList<const ClientPlayer *> &targets);
    void useCard(const Card *card);
    void askForJudge(const QString &player_name = QString());
    void ackForHpChange(int delta);
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;

    Q_INVOKABLE void addPlayer(const QString &player_info);
    Q_INVOKABLE void removePlayer(const QString &player_name);
    Q_INVOKABLE void drawCards(const QString &cards_str);
    Q_INVOKABLE void drawNCards(const QString &draw_str);
    Q_INVOKABLE void getLords(const QString &lords_str);
    Q_INVOKABLE void getGenerals(const QString &generals_str);
    Q_INVOKABLE void startInXs(const QString &);
    Q_INVOKABLE void duplicationError(const QString &);
    Q_INVOKABLE void arrangeSeats(const QString &seats);
    Q_INVOKABLE void moveCard(const QString &move_str);
    Q_INVOKABLE void activate(const QString &focus_player);
    Q_INVOKABLE void startGame(const QString &);
    Q_INVOKABLE void hpDamage(const QString &damage_str);
    Q_INVOKABLE void hpFlow(const QString &flow_str);
    Q_INVOKABLE void hpRecover(const QString &recover_str);
    Q_INVOKABLE void judge(const QString &judge_str);
    Q_INVOKABLE void requestForCard(const QString &request_str);
    Q_INVOKABLE void askForSkillInvoke(const QString &ask_str);
    Q_INVOKABLE void playSkillEffect(const QString &play_str);
    Q_INVOKABLE void askForNullification(const QString &ask_str);

    CardPattern *pattern;
    QVariantMap tag;
    QList<CardPattern *> enable_patterns, disable_patterns;
    QList<const Card*> discarded_list;

public slots:    
    void itemChosen(const QString &item_name);
    void updateFrequentFlags(int state);
    void replyNullification(int card_id = -1);

private:
    QObject *room;
    ClientPlayer *self;
    Status status;
    QSet<QString> frequent_flags;
    int alive_count;

private slots:
    void processReply();
    void raiseError(QAbstractSocket::SocketError socket_error);
    void notifyRoleChange(const QString &new_role);

signals:
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);
    void cards_drawed(const QList<const Card *> &cards);
    void lords_got(const QList<const General*> &lords);
    void generals_got(const General *lord, const QList<const General *> &generals);
    void prompt_changed(const QString &prompt_str);
    void seats_arranged(const QList<const ClientPlayer*> &seats);
    void n_card_drawed(ClientPlayer *player, int n);
    void card_requested(const QString pattern);
    void hp_changed(const QString &target, int delta);
    void card_moved(ClientPlayer *src, Player::Place src_place,
                    ClientPlayer *dest, Player::Place dest_place,
                    int card_id);
    void status_changed(Client::Status new_status);
};

extern Client *ClientInstance;

#endif // CLIENT_H
