#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"
#include "socket.h"
#include "clientstruct.h"

class NullificationDialog;
class Recorder;
class Replayer;
class QTextDocument;

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
        AskForAG,
        AskForPlayerChoose,
        AskForYiji,
        AskForGuanxing,
        AskForGongxin,
    };

    explicit Client(QObject *parent, const QString &filename = QString());

    void roomBegin(const QString &begin_str);
    void room(const QString &room_str);
    void roomEnd(const QString &);
    void roomCreated(const QString &idstr);
    void roomError(const QString &errorStr);
    void hallEntered(const QString &);

    void disconnectFromHost();
    void request(const QString &message);
    void useCard(const Card *card, const QList<const Player *> &targets = QList<const Player *>());
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;
    void responseCard(const Card *card);
    bool noTargetResponsing() const;
    void discardCards(const Card *card);
    void replyYiji(const Card *card, const Player *to);
    void replyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards);
    QList<const ClientPlayer *> getPlayers() const;
    void speakToServer(const QString &text);
    ClientPlayer *getPlayer(const QString &name);
    void surrender();
    void kick(const QString &to_kick);
    bool save(const QString &filename) const;
    void setLines(const QString &skill_name);
    QString getSkillLine() const;
    Replayer *getReplayer() const;
    QString getPlayerName(const QString &str);
    QString getPattern() const;
    QString getSkillNameToInvoke() const;
    void invokeSkill(bool invoke) ;

    QTextDocument *getLinesDoc() const;
    QTextDocument *getPromptDoc() const;

    typedef void (Client::*Callback)(const QString &);

    void checkVersion(const QString &server_version);
    void setup(const QString &setup_str);
    void addPlayer(const QString &player_info);
    void removePlayer(const QString &player_name);
    void drawCards(const QString &cards_str);
    void drawNCards(const QString &draw_str);
    void doChooseGeneral(const QString &generals_str);
    void doChooseGeneral2(const QString &generals_str);
    void startInXs(const QString &);
    void arrangeSeats(const QString &seats);
    void activate(const QString &focus_player);
    void startGame(const QString &);
    void hpChange(const QString &change_str);
    void playSkillEffect(const QString &play_str);
    void playCardEffect(const QString &play_str);
    void playAudio(const QString &name);
    void clearPile(const QString &);
    void setPileNumber(const QString &pile_num);
    void gameOver(const QString &result_str);
    void killPlayer(const QString &player_name);
    void revivePlayer(const QString &player_name);
    void warn(const QString &);
    void setMark(const QString &mark_str);
    void showCard(const QString &show_str);
    void doGuanxing(const QString &guanxing_str);
    void doGongxin(const QString &gongxin_str);
    void log(const QString &log_str);
    void speak(const QString &speak_data);
    void addHistory(const QString &card);
    void moveFocus(const QString &focus);
    void setEmotion(const QString &set_str);
    void skillInvoked(const QString &invoke_str);
    void acquireSkill(const QString &acquire_str);
    void animate(const QString &animate_str);
    void jilei(const QString &jilei_str);
    void judgeResult(const QString &result_str);
    void setScreenName(const QString &set_str);
    void setFixedDistance(const QString &set_str);
    void pile(const QString &pile_str);
    void transfigure(const QString &transfigure_tr);

    void moveCard(const QString &move_str);
    void moveNCards(const QString &move_str);

    void fillAG(const QString &cards_str);
    void askForAG(const QString &);
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
    void askForExchange(const QString &exchange_str);
    void askForSuit(const QString &);
    void askForKingdom(const QString &);
    void askForNullification(const QString &ask_str);
    void askForPindian(const QString &ask_str);
    void askForYiji(const QString &card_list);
    void askForCardChosen(const QString &ask_str);
    void askForPlayerChosen(const QString &ask_str);
    void askForGeneral(const QString &generals);

    // 3v3 & 1v1 methods
    void fillGenerals(const QString &generals);
    void askForGeneral3v3(const QString &);
    void takeGeneral(const QString &take_str);
    void startArrange(const QString &);
    void askForOrder(const QString &reason);
    void askForRole(const QString &role_str);
    void askForDirection(const QString &);
    void recoverGeneral(const QString &);
    void revealGeneral(const QString &);

    void attachSkill(const QString &skill_name);
    void detachSkill(const QString &skill_name);

    void askForAssign(const QString &);

    // public fields
    bool refusable;
    bool include_equip;
    int discard_num;
    QList<const Card*> discarded_list;
    QDialog *ask_dialog;
    QStringList players_to_choose;   

public slots:
    void signup();
    void chooseItem(const QString &_name);
    void selectChoice();
    void chooseCard(int card_id = -2);
    void choosePlayer(const Player *player);
    void trust();
    void requestCard(int card_id);
    void addRobot();
    void fillRobots();
    void arrange(const QStringList &order);
    void chooseAG(int card_id);
    void replyGongxin(int card_id = -1);

private:
    ClientSocket *socket;
    Status status;
    int alive_count;
    QHash<QString, Callback> callbacks;
    QList<const ClientPlayer*> players;
    bool use_card;
    QStringList ban_packages;
    Recorder *recorder;
    Replayer *replayer;
    QTextDocument *lines_doc, *prompt_doc;
    int pile_num;
    QString skill_title, skill_line;
    QString choose_command;
    QString card_pattern;
    QString skill_to_invoke;
    int swap_pile;

    void updatePileNum();
    void setPromptList(const QStringList &text);
    void commandFormatWarning(const QString &str, const QRegExp &rx, const char *command);

private slots:
    void processCommand(const QString &cmd);
    void processReply(char *reply);
    void notifyRoleChange(const QString &new_role);
    void chooseSuit();
    void chooseKingdom();
    void clearTurnTag();
    void selectOrder();
    void selectRole();

signals:
    void version_checked(const QString &server_version);
    void server_connected();
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);
    void generals_got(const QStringList &generals);
    void seats_arranged(const QList<const ClientPlayer*> &seats);
    void hp_changed(const QString &who, int delta, DamageStruct::Nature nature);
    void status_changed(Client::Status new_status);
    void avatars_hiden();
    void pile_cleared();
    void player_killed(const QString &who);
    void player_revived(const QString &who);
    void card_shown(const QString &player_name, int card_id);
    void log_received(const QString &log_str);
    void guanxing(const QList<int> &card_ids, bool up_only);
    void gongxin(const QList<int> &card_ids, bool enable_heart);
    void focus_moved(const QString &focus);
    void emotion_set(const QString &target, const QString &emotion);
    void skill_invoked(const QString &who, const QString &skill_name);
    void skill_acquired(const ClientPlayer *player, const QString &skill_name);
    void animated(const QString &name, const QStringList &args);
    void text_spoken(const QString &text);
    void line_spoken(const QString &line);
    void judge_result(const QString &who, const QString &result);

    void game_started();
    void game_over();
    void standoff();

    void cards_drawed(const QList<const Card *> &cards);
    void n_cards_drawed(ClientPlayer *player, int n);

    void card_moved(const CardMoveStructForClient &move);
    void n_cards_moved(int n, const QString &from, const QString &to);

    void skill_attached(const QString &skill_name, bool from_left);
    void skill_detached(const QString &skill_name);

    void ag_filled(const QList<int> &card_ids);
    void ag_taken(const ClientPlayer *taker, int card_id);
    void ag_cleared();

    void generals_filled(const QStringList &general_names);
    void general_taken(const QString &who, const QString &name);
    void general_asked();
    void arrange_started();
    void general_recovered(int index, const QString &name);
    void general_revealed(bool self, const QString &general);

    void assign_asked();
};

extern Client *ClientInstance;

#endif // CLIENT_H
