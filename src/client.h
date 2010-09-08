#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"

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
    void request(const QString &message);
    void useCard(const Card *card, const QList<const ClientPlayer *> &targets);
    void useCard(const Card *card);
    void ackForHpChange(int delta);
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;
    void responseCard(const Card *card);
    void responseCard(const Card *card, const QList<const ClientPlayer *> &targets);
    bool noTargetResponsing() const;
    void discardCards(const Card *card);
    void chooseAG(int card_id);
    QList<ClientPlayer *> getPlayers() const;

    typedef void (Client::*Callback)(const QString &);

    void addPlayer(const QString &player_info);
    void removePlayer(const QString &player_name);
    void drawCards(const QString &cards_str);
    void drawNCards(const QString &draw_str);
    void getGenerals(const QString &generals_str);
    void startInXs(const QString &);
    void duplicationError(const QString &);
    void arrangeSeats(const QString &seats);
    void moveCard(const QString &move_str);
    void activate(const QString &focus_player);
    void startGame(const QString &);
    void hpChange(const QString &change_str);
    void askForCard(const QString &request_str);
    void askForSkillInvoke(const QString &ask_str);
    void playSkillEffect(const QString &play_str);
    void askForNullification(const QString &ask_str);
    void askForCardChosen(const QString &ask_str);
    void playCardEffect(const QString &play_str);
    void prompt(const QString &prompt_str);
    void clearPile(const QString &);
    void setPileNumber(const QString &pile_num);
    void askForDiscard(const QString &discard_str);
    void gameOver(const QString &result_str);
    void killPlayer(const QString &player_name);
    void gameOverWarn(const QString &);
    void askForSuit(const QString &);
    void fillAG(const QString &cards_str);
    void askForAG(const QString &);
    void takeAG(const QString &take_str);


    // public fields
    QString card_pattern;
    int discard_num;
    QVariantMap tag, turn_tag;
    QList<const Card*> discarded_list;

public slots:    
    void itemChosen(const QString &item_name);
    void updateFrequentFlags(int state);
    void replyNullification(int card_id = -1);
    void chooseCard(int card_id);

private:
    QObject *room;
    Status status;
    QSet<QString> frequent_flags;
    int alive_count;
    QHash<QString, Callback> callbacks;
    QList<ClientPlayer*> players;

private slots:
    void processReply();
    void raiseError(QAbstractSocket::SocketError socket_error);
    void notifyRoleChange(const QString &new_role);
    void chooseSuit();

signals:
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);
    void cards_drawed(const QList<const Card *> &cards);
    void generals_got(const QList<const General *> &generals);
    void message_changed(const QString &message);
    void prompt_changed(const QString &prompt_str);
    void seats_arranged(const QList<const ClientPlayer*> &seats);
    void n_card_drawed(ClientPlayer *player, int n);
    void hp_changed(const QString &who, int delta);
    void card_moved(const CardMoveStructForClient &move);
    void status_changed(Client::Status new_status);
    void avatars_hiden();
    void pile_cleared();
    void pile_num_set(int n);
    void game_over(bool victory, const QList<bool> &result_list);
    void player_killed(const QString &who);
    void ag_filled(const QList<int> &card_ids);
    void ag_taken(const QString &general_name, int card_id);
};

extern Client *ClientInstance;

#endif // CLIENT_H
