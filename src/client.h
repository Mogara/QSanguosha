#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"

class NullificationDialog;

#include <QTcpSocket>

class Client : public QObject
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
        ExecDialog,
        AskForSkillInvoke,
        AskForChoices,        
        AskForAG,
        AskForPlayerChoose,
        AskForYiji,
        AskForGuanxing,
        AskForGongxin
    };

    explicit Client(QObject *parent);

    void disconnectFromHost();
    void signup();
    void request(const QString &message);
    void useCard(const Card *card, const QList<const ClientPlayer *> &targets);
    void useCard(const Card *card);
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;
    void invokeSkill(bool invoke);
    void selectChoice(int choice);
    void responseCard(const Card *card);
    void responseCard(const Card *card, const QList<const ClientPlayer *> &targets);
    bool noTargetResponsing() const;
    void discardCards(const Card *card);
    void replyYiji(const Card *card, const ClientPlayer *to);
    void replyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards);
    void replyGongxin(int card_id = -1);
    QList<ClientPlayer *> getPlayers() const;
    void speakToServer(const QString &text);    
    void prompt(const QString &prompt_str);

    typedef void (Client::*Callback)(const QString &);

    void checkVersion(const QString &server_version);
    void setup(const QString &setup_str);
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
    void clearPile(const QString &);
    void setPileNumber(const QString &pile_num);
    void gameOver(const QString &result_str);
    void killPlayer(const QString &player_name);
    void gameOverWarn(const QString &);
    void setMark(const QString &mark_str);
    void showCard(const QString &show_str);
    void doGuanxing(const QString &guanxing_str);
    void doGongxin(const QString &gongxin_str);
    void log(const QString &log_str);
    void speak(const QString &speak_data);    
    void increaseSlashCount(const QString & = QString());
    void moveFocus(const QString &focus);

    void moveCard(const QString &move_str);
    void moveNCards(const QString &move_str);
    void moveCardToDrawPile(const QString &from);

    void fillAG(const QString &cards_str);
    void askForAG(const QString &);
    void chooseAG(int card_id);
    void takeAG(const QString &take_str);
    void clearAG(const QString &);

    void askForCard(const QString &request_str);
    void askForUseCard(const QString &request_str);
    void askForCardOrUseCard(const QString &request_str);

    void askForSinglePeach(const QString &ask_str);
    void askForCardShow(const QString &requestor);
    void askForSkillInvoke(const QString &skill_name);
    void askForChoice(const QString &ask_str);
    void askForDiscard(const QString &discard_str);
    void askForSuit(const QString &);
    void askForNullification(const QString &ask_str);
    void askForCardChosen(const QString &ask_str);
    void askForPindian(const QString &ask_str);
    void askForYiji(const QString &card_list);
    void askForPlayerChosen(const QString &ask_str);

    void attachSkill(const QString &skill_name);
    void detachSkill(const QString &skill_name);

    // public fields
    QString card_pattern;
    bool refusable;
    bool include_equip;
    int discard_num;
    Card::Suit discard_suit;
    QVariantMap tag, turn_tag;
    QList<const Card*> discarded_list;
    QDialog *ask_dialog;
    QStringList players_to_choose;

public slots:    
    void itemChosen(const QString &item_name);
    void updateFrequentFlags(int state);
    void replyNullification(int card_id = -1);
    void chooseCard(int card_id = -2);
    void choosePlayer(const ClientPlayer *player);
    void trust();

#ifndef QT_NO_DEBUG
    void cheatChoose();
    void requestCard(int card_id);
#endif

private:
    QTcpSocket *socket;
    Status status;
    QSet<QString> frequent_flags;
    int alive_count;
    QHash<QString, Callback> callbacks;
    QList<ClientPlayer*> players;
    NullificationDialog *nullification_dialog;
    QString first_choice;
    QString second_choice;
    bool use_card;

private slots:
    void processReply(char *reply);
    void emitReplies();
    void raiseError(QAbstractSocket::SocketError socket_error);
    void notifyRoleChange(const QString &new_role);
    void chooseSuit();
    void clearTurnTag();

signals:
    void reply_got(char *reply);
    void server_connected();
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
    void player_killed(const QString &who);
    void card_shown(const QString &player_name, int card_id);
    void log_received(const QString &log_str);
    void guanxing(const QList<int> &card_ids);
    void gongxin(const QList<int> &card_ids);
    void words_spoken(const QString &who, const QString &text);
    void focus_moved(const QString &focus);

    void game_started();
    void game_over(bool victory, const QList<bool> &result_list);
    void standoff();

    void cards_drawed(const QList<const Card *> &cards);
    void n_cards_drawed(ClientPlayer *player, int n);
    void card_moved_to_draw_pile(const QString &from);

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
