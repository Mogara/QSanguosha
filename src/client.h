#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"

class NullificationDialog;

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
        Discarding,
        AskForAG,
        AskForYiji
    };

    explicit Client(QObject *parent = 0);
    void signup();
    void request(const QString &message);
    void useCard(const Card *card, const QList<const ClientPlayer *> &targets);
    void useCard(const Card *card);
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;
    void responseCard(const Card *card);
    void responseCard(const Card *card, const QList<const ClientPlayer *> &targets);
    bool noTargetResponsing() const;
    void discardCards(const Card *card);

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
    void activate(const QString &focus_player);
    void startGame(const QString &);
    void hpChange(const QString &change_str);
    void playSkillEffect(const QString &play_str);
    void closeNullification(const QString &);
    void playCardEffect(const QString &play_str);
    void prompt(const QString &prompt_str);
    void clearPile(const QString &);
    void setPileNumber(const QString &pile_num);
    void gameOver(const QString &result_str);
    void killPlayer(const QString &player_name);
    void gameOverWarn(const QString &);
    void setMark(const QString &mark_str);
    void showCard(const QString &show_str);
    void doGuanxing(const QString &guanxing_str);
    void replyYiji(const Card *card, const ClientPlayer *to);

    void moveCard(const QString &move_str);
    void moveNCards(const QString &move_str);

    void fillAG(const QString &cards_str);
    void askForAG(const QString &);
    void chooseAG(int card_id);
    void takeAG(const QString &take_str);
    void clearAG(const QString &);

    void askForSinglePeach(const QString &ask_str);
    void askForCardShow(const QString &requestor);
    void askForCard(const QString &request_str);
    void askForSkillInvoke(const QString &skill_name);
    void askForChoice(const QString &ask_str);
    void askForDiscard(const QString &discard_str);
    void askForSuit(const QString &);
    void askForNullification(const QString &ask_str);
    void askForCardChosen(const QString &ask_str);
    void askForPindian(const QString &ask_str);
    void askForYiji(const QString &card_list);

    void attachSkill(const QString &skill_name);
    void detachSkill(const QString &skill_name);

    // public fields
    QString card_pattern;
    bool refusable;

    int discard_num;
    QVariantMap tag, turn_tag;
    QList<const Card*> discarded_list;

public slots:    
    void itemChosen(const QString &item_name);
    void updateFrequentFlags(int state);
    void replyNullification(int card_id = -1);
    void chooseCard(int card_id = -2);

#ifndef QT_NO_DEBUG
    void cheatChoose();
#endif

private:
    QObject *room;
    Status status;
    QSet<QString> frequent_flags;
    int alive_count;
    QHash<QString, Callback> callbacks;
    QList<ClientPlayer*> players;
    NullificationDialog *nullification_dialog;

private slots:
    void processReply();
    void raiseError(QAbstractSocket::SocketError socket_error);
    void notifyRoleChange(const QString &new_role);
    void chooseSuit();
    void clearTurnTag();

signals:
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);    
    void generals_got(const QList<const General *> &generals);
    void message_changed(const QString &message);
    void prompt_changed(const QString &prompt_str);
    void seats_arranged(const QList<const ClientPlayer*> &seats);    
    void hp_changed(const QString &who, int delta);    
    void status_changed(Client::Status new_status);
    void avatars_hiden();
    void pile_cleared();
    void pile_num_set(int n);
    void game_over(bool victory, const QList<bool> &result_list);
    void player_killed(const QString &who);
    void card_shown(const QString &player_name, int card_id);
    void guanxing(const QList<int> &card_ids);

    void cards_drawed(const QList<const Card *> &cards);
    void n_cards_drawed(ClientPlayer *player, int n);

    void card_moved(const CardMoveStructForClient &move);
    void n_cards_moved(int n, const QString &from, const QString &to);

    void skill_attached(const QString &skill_name);
    void skill_detached(const QString &skill_name);

    void ag_filled(const QList<int> &card_ids);
    void ag_taken(const ClientPlayer *taker, int card_id);
    void ag_cleared();
};

extern Client *ClientInstance;

#endif // CLIENT_H
