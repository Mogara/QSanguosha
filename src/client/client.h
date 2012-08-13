#ifndef CLIENT_H
#define CLIENT_H

#include "clientplayer.h"
#include "card.h"
#include "skill.h"
#include "socket.h"
#include "clientstruct.h"
#include "protocol.h"

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
        AskForShowOrPindian
    };

    explicit Client(QObject *parent, const QString &filename = QString());

    void roomBegin(const QString &begin_str);
    void room(const QString &room_str);
    void roomEnd(const QString &);
    void roomCreated(const QString &idstr);
    void roomError(const QString &errorStr);
    void hallEntered(const QString &);

    // cheat functions
    void requestCheatGetOneCard(int card_id);
    void requestCheatChangeGeneral(QString name);
    void requestCheatKill(const QString& killer, const QString& victim);
    void requestCheatDamage(const QString& source, const QString& target, DamageStruct::Nature nature, int points);
    void requestCheatRevive(const QString& name);
    void requestCheatRunScript(const QString& script);

    // other client requests
    void requestSurrender();

    void disconnectFromHost();
    void replyToServer(QSanProtocol::CommandType command, const Json::Value &arg = Json::Value::null);
    void requestToServer(QSanProtocol::CommandType command, const Json::Value &arg = Json::Value::null);
    void request(const QString &message);
    void onPlayerUseCard(const Card *card, const QList<const Player *> &targets = QList<const Player *>());
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;    
    bool hasNoTargetResponsing() const;
    void onPlayerResponseCard(const Card *card);
    void onPlayerInvokeSkill(bool invoke);
    void onPlayerDiscardCards(const Card *card);
    void onPlayerReplyYiji(const Card *card, const Player *to);
    void onPlayerReplyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards);
    void onPlayerAssignRole(const QList<QString> &names, const QList<QString> &roles);
    QList<const ClientPlayer *> getPlayers() const;
    void speakToServer(const QString &text);
    ClientPlayer *getPlayer(const QString &name);
    void kick(const QString &to_kick);
    bool save(const QString &filename) const;
    QList<QString> getRecords() const;
    void setLines(const QString &skill_name);
    QString getSkillLine() const;
    Replayer *getReplayer() const;
    QString getPlayerName(const QString &str);
    QString getSkillNameToInvoke() const;    

    QTextDocument *getLinesDoc() const;
    QTextDocument *getPromptDoc() const;

    typedef void (Client::*Callback)(const QString &);
    typedef void (Client::*CallBack)(const Json::Value &);

    void checkVersion(const QString &server_version);
    void setup(const QString &setup_str);
    void networkDelayTest(const QString&);
    void addPlayer(const QString &player_info);
    void removePlayer(const QString &player_name);
    void drawCards(const QString &cards_str);
    void drawNCards(const QString &draw_str);    
    void startInXs(const QString &);
    void arrangeSeats(const QString &seats);
    void activate(const Json::Value &playerId);
    void startGame(const QString &);
    void hpChange(const QString &change_str);
    void resetPiles(const QString &);
    void setPileNumber(const QString &pile_num);
    void gameOver(const Json::Value &);
    void loseCards(const Json::Value &);
    void getCards(const Json::Value &);
    void updateProperty(const Json::Value &);
    void killPlayer(const QString &player_name);
    void revivePlayer(const QString &player_name);
    void warn(const QString &);
    void setMark(const QString &mark_str);
    void showCard(const Json::Value &show_str);    
    void log(const QString &log_str);
    void speak(const QString &speak_data);
    void addHistory(const QString &card);
    void moveFocus(const Json::Value &focus);
    void setEmotion(const QString &set_str);
    void skillInvoked(const Json::Value &invoke_str);
    void animate(const QString &animate_str);
    void jilei(const QString &jilei_str);
    void cardLock(const QString &card_str);
    void setScreenName(const QString &set_str);
    void setFixedDistance(const QString &set_str);
    void pile(const QString &pile_str);
    void updateStateItem(const QString &state_str);
    void setCardFlag(const QString &pattern_str);
    void playSystemAudioEffect(const QString &effect_str);
    void updateCard(const Json::Value &arg);

    void fillAG(const QString &cards_str);    
    void takeAG(const QString &take_str);
    void clearAG(const QString &);

    //interactive server callbacks
    void askForCard(const Json::Value&);
    void askForUseCard(const Json::Value&);
    void askForAG(const Json::Value&);
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
    void askForGongxin(const Json::Value &);
    void askForAssign(const Json::Value &); // Assign roles at the beginning of game
    void askForSurrender(const Json::Value &);
    void handleGameEvent(const Json::Value &);
    //3v3 & 1v1
    void askForOrder(const Json::Value &);
    void askForRole3v3(const Json::Value &);
    void askForDirection(const Json::Value &);    

    // 3v3 & 1v1 methods
    void fillGenerals(const QString &generals);
    void askForGeneral3v3(const QString &);
    void takeGeneral(const QString &take_str);
    void startArrange(const QString &);    
    
    void recoverGeneral(const QString &);
    void revealGeneral(const QString &);

    void attachSkill(const QString &skill_name);
    
    inline virtual RoomState* getRoomState() { return &_m_roomState; }
    inline virtual Card* getCard(int cardId) const { return _m_roomState.getCard(cardId); }

    inline void setCountdown(QSanProtocol::Countdown countdown) 
    {
        m_mutexCountdown.lock();
        m_countdown = countdown;
        m_mutexCountdown.unlock();
    }
    inline QSanProtocol::Countdown getCountdown()
    {
        m_mutexCountdown.lock();
        QSanProtocol::Countdown countdown = m_countdown;
        m_mutexCountdown.unlock();
        return countdown;
    }

    // public fields
    bool m_isDiscardActionRefusable;
    bool m_canDiscardEquip;
    int discard_num;
    int min_num;
    QString skill_name;
    QList<const Card*> discarded_list;
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
    QList<const ClientPlayer*> players;
    bool m_isUseCard;
    QStringList ban_packages;
    Recorder *recorder;
    Replayer *replayer;
    QTextDocument *lines_doc, *prompt_doc;
    int pile_num;
    QString skill_title, skill_line;
    QString skill_to_invoke;

    unsigned int _m_lastServerSerial;

    void updatePileNum();
    void setPromptList(const QStringList &text);
    QString _processCardPattern(const QString &pattern);
    void commandFormatWarning(const QString &str, const QRegExp &rx, const char *command);

    void _askForCardOrUseCard(const Json::Value&);
    bool _loseSingleCard(int card_id, CardsMoveStruct move);
    bool _getSingleCard(int card_id, CardsMoveStruct move);

private slots:
    void processServerPacket(const QString &cmd);
    void processServerPacket(const char *cmd);
    bool processServerRequest(const QSanProtocol::QSanGeneralPacket& packet);
    void processObsoleteServerPacket(const QString &cmd);
    void notifyRoleChange(const QString &new_role);
    void onPlayerChooseSuit();
    void onPlayerChooseKingdom();
    void clearTurnTag();
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
    void cards_got(const ClientPlayer *player, const QString &flags, const QString &reason);
    void roles_got(const QString &scheme, const QStringList &roles);
    void directions_got();    
    void orders_got(QSanProtocol::Game3v3ChooseOrderCommand reason);
    
    void seats_arranged(const QList<const ClientPlayer*> &seats);
    void hp_changed(const QString &who, int delta, DamageStruct::Nature nature, bool losthp);
    void status_changed(Client::Status oldStatus, Client::Status newStatus);
    void avatars_hiden();
    void pile_reset();
    void player_killed(const QString &who);
    void player_revived(const QString &who);
    void card_shown(const QString &player_name, int card_id);
    void log_received(const QString &log_str);
    void guanxing(const QList<int> &card_ids, bool up_only);
    void gongxin(const QList<int> &card_ids, bool enable_heart);
    void focus_moved(const QStringList &focus, QSanProtocol::Countdown countdown);
    void emotion_set(const QString &target, const QString &emotion);
    void skill_invoked(const QString &who, const QString &skill_name);
    void skill_acquired(const ClientPlayer *player, const QString &skill_name);
    void animated(const QString &name, const QStringList &args);
    void text_spoken(const QString &text);
    void line_spoken(const QString &line);
    void card_used();

    void game_started();
    void game_over();
    void standoff();
    void event_received(const Json::Value&);
        
    void move_cards_lost(int moveId, QList<CardsMoveStruct> moves);
    void move_cards_got(int moveId, QList<CardsMoveStruct> moves);

    void skill_attached(const QString &skill_name, bool from_left);
    void skill_detached(const QString &skill_name);
    void do_filter();

    void ag_filled(const QList<int> &card_ids);
    void ag_taken(ClientPlayer *taker, int card_id);
    void ag_cleared();

    void generals_filled(const QStringList &general_names);
    void general_taken(const QString &who, const QString &name);
    void general_asked();
    void arrange_started();
    void general_recovered(int index, const QString &name);
    void general_revealed(bool self, const QString &general);

    void role_state_changed(const QString & state_str);

    void assign_asked();
    void start_in_xs();
};

extern Client *ClientInstance;

#endif // CLIENT_H
