#ifndef _CLIENT_H
#define _CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"
#include "socket.h"
#include "clientstruct.h"
#include "protocol.h"

class Recorder;
class Replayer;
class QTextDocument;

class Client: public QObject {
    Q_OBJECT
    Q_PROPERTY(Client::Status status READ getStatus WRITE setStatus)

    Q_ENUMS(Status)

public:
    enum Status {
        NotActive = 0x00,
        Responding = 0x01,
        Playing = 0x02,
        Discarding = 0x03,
        Exchanging = 0x04,
        ExecDialog = 0x05,
        AskForSkillInvoke = 0x06,
        AskForAG = 0x07,
        AskForPlayerChoose = 0x08,
        AskForYiji = 0x09,
        AskForGuanxing = 0x0A,
        AskForGongxin = 0x0B,
        AskForShowOrPindian = 0x0C,
        AskForGeneralTaken = 0x0D,
        AskForArrangement = 0x0E,

        RespondingUse = 0x11,
        RespondingForDiscard = 0x21,
        RespondingNonTrigger = 0x31,

        ClientStatusBasicMask = 0x0F
    };

    explicit Client(QObject *parent, const QString &filename = QString());
    ~Client();

    // cheat functions
    void requestCheatGetOneCard(int card_id);
    void requestCheatChangeGeneral(const QString &name, bool isSecondaryHero = false);
    void requestCheatKill(const QString &killer, const QString &victim);
    void requestCheatDamage(const QString &source, const QString &target, DamageStruct::Nature nature, int points);
    void requestCheatRevive(const QString &name);
    void requestCheatRunScript(const QString &script);

    // other client requests
    void requestSurrender();

    void disconnectFromHost();
    void replyToServer(QSanProtocol::CommandType command, const Json::Value &arg = Json::Value::null);
    void requestToServer(QSanProtocol::CommandType command, const Json::Value &arg = Json::Value::null);
    void request(const QString &message);
    void onPlayerResponseCard(const Card *card, const QList<const Player *> &targets = QList<const Player *>());
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;
    void onPlayerInvokeSkill(bool invoke);
    void onPlayerDiscardCards(const Card *card);
    void onPlayerReplyYiji(const Card *card, const Player *to);
    void onPlayerReplyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards);
    void onPlayerAssignRole(const QList<QString> &names, const QList<QString> &roles);
    QList<const ClientPlayer *> getPlayers() const;
    void speakToServer(const QString &text);
    ClientPlayer *getPlayer(const QString &name);
    bool save(const QString &filename) const;
    QList<QString> getRecords() const;
    QString getReplayPath() const;
    void setLines(const QString &skill_name);
    Replayer *getReplayer() const;
    QString getPlayerName(const QString &str);
    QString getSkillNameToInvoke() const;

    QTextDocument *getLinesDoc() const;
    QTextDocument *getPromptDoc() const;

    typedef void (Client::*Callback) (const QString &);
    typedef void (Client::*CallBack) (const Json::Value &);

    void checkVersion(const QString &server_version);
    void setup(const QString &setup_str);
    void networkDelayTest(const QString &);
    void addPlayer(const QString &player_info);
    void removePlayer(const QString &player_name);
    void startInXs(const QString &);
    void arrangeSeats(const QString &seats);
    void activate(const Json::Value &playerId);
    void startGame(const Json::Value &);
    void hpChange(const Json::Value &change_str);
    void maxhpChange(const Json::Value &change_str);
    void resetPiles(const Json::Value &);
    void setPileNumber(const Json::Value &pile_str);
    void gameOver(const Json::Value &);
    void loseCards(const Json::Value &);
    void getCards(const Json::Value &);
    void updateProperty(const Json::Value &);
    void killPlayer(const Json::Value &player_arg);
    void revivePlayer(const Json::Value &player_arg);
    void setDashboardShadow(const Json::Value &player_arg);
    void warn(const QString &);
    void setMark(const Json::Value &mark_str);
    void showCard(const Json::Value &show_str);
    void log(const Json::Value &log_str);
    void speak(const QString &speak_data);
    void addHistory(const Json::Value &history);
    void moveFocus(const Json::Value &focus);
    void setEmotion(const Json::Value &set_str);
    void skillInvoked(const Json::Value &invoke_str);
    void animate(const Json::Value &animate_str);
    void cardLimitation(const Json::Value &limit);
    void setNullification(const Json::Value &str);
    void enableSurrender(const Json::Value &enabled);
    void exchangeKnownCards(const Json::Value &players);
    void setKnownCards(const Json::Value &set_str);
    void viewGenerals(const Json::Value &str);
    void setFixedDistance(const Json::Value &set_str);
    void updateStateItem(const Json::Value &state_str);
    void setAvailableCards(const Json::Value &pile);
    void setCardFlag(const Json::Value &pattern_str);
    void updateCard(const Json::Value &arg);

    void fillAG(const Json::Value &cards_str);
    void takeAG(const Json::Value &take_str);
    void clearAG(const Json::Value &);

    //interactive server callbacks
    void askForCardOrUseCard(const Json::Value &);
    void askForAG(const Json::Value &);
    void askForSinglePeach(const Json::Value &);
    void askForCardShow(const Json::Value &);
    void askForSkillInvoke(const Json::Value &);
    void askForChoice(const Json::Value &);
    void askForDiscard(const Json::Value &);
    void askForExchange(const Json::Value &);
    void askForSuit(const Json::Value &);
    void askForKingdom(const Json::Value &);
    void askForNullification(const Json::Value &);
    void askForPindian(const Json::Value &);
    void askForCardChosen(const Json::Value &);
    void askForPlayerChosen(const Json::Value &);
    void askForGeneral(const Json::Value &);
    void askForYiji(const Json::Value &);
    void askForGuanxing(const Json::Value &);
    void showAllCards(const Json::Value &);
    void askForGongxin(const Json::Value &);
    void askForAssign(const Json::Value &); // Assign roles at the beginning of game
    void askForSurrender(const Json::Value &);
    void askForLuckCard(const Json::Value &);
    void handleGameEvent(const Json::Value &);
    //3v3 & 1v1
    void askForOrder(const Json::Value &);
    void askForRole3v3(const Json::Value &);
    void askForDirection(const Json::Value &);

    // 3v3 & 1v1 methods
    void fillGenerals(const Json::Value &generals);
    void askForGeneral3v3(const Json::Value &);
    void takeGeneral(const Json::Value &take_str);
    void startArrange(const Json::Value &to_arrange);

    void recoverGeneral(const Json::Value &);
    void revealGeneral(const Json::Value &);

    void attachSkill(const Json::Value &skill);

    inline virtual RoomState *getRoomState() { return &_m_roomState; }
    inline virtual Card *getCard(int cardId) const{ return _m_roomState.getCard(cardId); }

    inline void setCountdown(QSanProtocol::Countdown countdown) {
        m_mutexCountdown.lock();
        m_countdown = countdown;
        m_mutexCountdown.unlock();
    }

    inline QSanProtocol::Countdown getCountdown() {
        m_mutexCountdown.lock();
        QSanProtocol::Countdown countdown = m_countdown;
        m_mutexCountdown.unlock();
        return countdown;
    }

    inline QList<int> getAvailableCards() const{ return available_cards; }

    // public fields
    bool m_isDiscardActionRefusable;
    bool m_canDiscardEquip;
    bool m_noNullificationThisTime;
    QString m_noNullificationTrickName;
    int discard_num;
    int min_num;
    QString skill_name;
    QList<const Card *> discarded_list;
    QStringList players_to_choose;

public slots:
    void signup();
    void onPlayerChooseGeneral(const QString &_name);
    void onPlayerMakeChoice();
    void onPlayerChooseCard(int card_id = -2);
    void onPlayerChooseAG(int card_id);
    void onPlayerChoosePlayer(const Player *player);
    void trust();
    void addRobot();
    void fillRobots();
    void arrange(const QStringList &order);

    void onPlayerReplyGongxin(int card_id = -1);

protected:
    // operation countdown
    QSanProtocol::Countdown m_countdown;
    // sync objects
    QMutex m_mutexCountdown;
    Status status;
    int alive_count;
    int swap_pile;
    RoomState _m_roomState;

private:
    ClientSocket *socket;
    bool m_isGameOver;
    QHash<QString, Callback> callbacks;
    QHash<QSanProtocol::CommandType, CallBack> m_interactions;
    QHash<QSanProtocol::CommandType, CallBack> m_callbacks;
    QList<const ClientPlayer *> players;
    QStringList ban_packages;
    Recorder *recorder;
    Replayer *replayer;
    QTextDocument *lines_doc, *prompt_doc;
    int pile_num;
    QString skill_to_invoke;
    QList<int> available_cards;

    unsigned int _m_lastServerSerial;

    void updatePileNum();
    QString setPromptList(const QStringList &text);
    QString _processCardPattern(const QString &pattern);
    void commandFormatWarning(const QString &str, const QRegExp &rx, const char *command);

    bool _loseSingleCard(int card_id, CardsMoveStruct move);
    bool _getSingleCard(int card_id, CardsMoveStruct move);

private slots:
    void processServerPacket(const QString &cmd);
    void processServerPacket(const char *cmd);
    bool processServerRequest(const QSanProtocol::QSanGeneralPacket &packet);
    void processObsoleteServerPacket(const QString &cmd);
    void notifyRoleChange(const QString &new_role);
    void onPlayerChooseSuit();
    void onPlayerChooseKingdom();
    void alertFocus();
    void onPlayerChooseOrder();
    void onPlayerChooseRole3v3();

signals:
    void version_checked(const QString &version_number, const QString &mod_name);
    void server_connected();
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);
    // choice signal
    void generals_got(const QStringList &generals);
    void kingdoms_got(const QStringList &kingdoms);
    void suits_got(const QStringList &suits);
    void options_got(const QString &skillName, const QStringList &options);
    void cards_got(const ClientPlayer *player, const QString &flags, const QString &reason, bool handcard_visible,
                    Card::HandlingMethod method, QList<int> disabled_ids);
    void roles_got(const QString &scheme, const QStringList &roles);
    void directions_got();
    void orders_got(QSanProtocol::Game3v3ChooseOrderCommand reason);

    void seats_arranged(const QList<const ClientPlayer *> &seats);
    void hp_changed(const QString &who, int delta, DamageStruct::Nature nature, bool losthp);
    void maxhp_changed(const QString &who, int delta);
    void status_changed(Client::Status oldStatus, Client::Status newStatus);
    void avatars_hiden();
    void pile_reset();
    void player_killed(const QString &who);
    void player_revived(const QString &who);
    void dashboard_death(const QString &who);
    void card_shown(const QString &player_name, int card_id);
    void log_received(const QStringList &log_str);
    void guanxing(const QList<int> &card_ids, bool up_only);
    void gongxin(const QList<int> &card_ids, bool enable_heart, QList<int> enabled_ids);
    void focus_moved(const QStringList &focus, QSanProtocol::Countdown countdown);
    void emotion_set(const QString &target, const QString &emotion);
    void skill_invoked(const QString &who, const QString &skill_name);
    void skill_acquired(const ClientPlayer *player, const QString &skill_name);
    void animated(int name, const QStringList &args);
    void text_spoken(const QString &text);
    void line_spoken(const QString &line);
    void card_used();

    void game_started();
    void game_over();
    void standoff();
    void event_received(const Json::Value &);

    void move_cards_lost(int moveId, QList<CardsMoveStruct> moves);
    void move_cards_got(int moveId, QList<CardsMoveStruct> moves);

    void skill_attached(const QString &skill_name, bool from_left);
    void skill_detached(const QString &skill_name);
    void do_filter();

    void nullification_asked(bool asked);
    void surrender_enabled(bool enabled);

    void ag_filled(const QList<int> &card_ids, const QList<int> &disabled_ids);
    void ag_taken(ClientPlayer *taker, int card_id, bool move_cards);
    void ag_cleared();

    void generals_filled(const QStringList &general_names);
    void general_taken(const QString &who, const QString &name, const QString &rule);
    void general_asked();
    void arrange_started(const QString &to_arrange);
    void general_recovered(int index, const QString &name);
    void general_revealed(bool self, const QString &general);

    void role_state_changed(const QString &state_str);
    void generals_viewed(const QString &reason, const QStringList &names);

    void assign_asked();
    void start_in_xs();
};

extern Client *ClientInstance;

#endif

