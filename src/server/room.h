#ifndef ROOM_H
#define ROOM_H

class TriggerSkill;
class ProhibitSkill;
class Scenario;
class RoomThread3v3;
class RoomThread1v1;
class TrickCard;

struct lua_State;
struct LogMessage;

#include "serverplayer.h"
#include "roomthread.h"
#include "protocol.h"
#include <qmutex.h>

class Room : public QThread{
    Q_OBJECT

public:
    friend class RoomThread;
    friend class RoomThread3v3;
    friend class RoomThread1v1;

    typedef void (Room::*Callback)(ServerPlayer *, const QString &);
    typedef bool (Room::*CallBack)(ServerPlayer *, const QSanProtocol::QSanGeneralPacket*);

    explicit Room(QObject *parent, const QString &mode);
    QString createLuaState();
    ServerPlayer *addSocket(ClientSocket *socket);
    bool isFull() const;
    bool isFinished() const;
    int getLack() const;
    QString getMode() const;
    const Scenario *getScenario() const;
    RoomThread *getThread() const;
    void playSkillEffect(const QString &skill_name, int index = -1);
    ServerPlayer *getCurrent() const;
    void setCurrent(ServerPlayer *current);
    int alivePlayerCount() const;
    QList<ServerPlayer *> getOtherPlayers(ServerPlayer *except) const;
    QList<ServerPlayer *> getPlayers() const;
    QList<ServerPlayer *> getAllPlayers() const;
    QList<ServerPlayer *> getAlivePlayers() const;
    void output(const QString &message);
    void outputEventStack();
    void enterDying(ServerPlayer *player, DamageStruct *reason);
    void killPlayer(ServerPlayer *victim, DamageStruct *reason = NULL);
    void revivePlayer(ServerPlayer *player);
    QStringList aliveRoles(ServerPlayer *except = NULL) const;
    void gameOver(const QString &winner);
    void slashEffect(const SlashEffectStruct &effect);
    void slashResult(const SlashEffectStruct &effect, const Card *jink);
    void attachSkillToPlayer(ServerPlayer *player, const QString &skill_name);
    void detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name);
    bool obtainable(const Card *card, ServerPlayer *player);
    void setPlayerFlag(ServerPlayer *player, const QString &flag);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void setPlayerMark(ServerPlayer *player, const QString &mark, int value);
    void setPlayerCardLock(ServerPlayer *player, const QString &name);
    void setPlayerStatistics(ServerPlayer *player, const QString &property_name, const QVariant &value);
    void useCard(const CardUseStruct &card_use, bool add_history = true);
    void damage(const DamageStruct &data);
    void sendDamageLog(const DamageStruct &data);
    void loseHp(ServerPlayer *victim, int lose = 1);
    void loseMaxHp(ServerPlayer *victim, int lose = 1);
    void applyDamage(ServerPlayer *victim, const DamageStruct &damage);
    void recover(ServerPlayer *player, const RecoverStruct &recover, bool set_emotion = false);
    bool cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to);
    bool cardEffect(const CardEffectStruct &effect);
    void judge(JudgeStruct &judge_struct);
    void sendJudgeResult(const JudgeStar judge);
    QList<int> getNCards(int n, bool update_pile_number = true);
    ServerPlayer *getLord() const;
    void askForGuanxing(ServerPlayer *zhuge, const QList<int> &cards, bool up_only);
    void doGongxin(ServerPlayer *shenlumeng, ServerPlayer *target);
    int drawCard();
    const Card *peek();
    void fillAG(const QList<int> &card_ids, ServerPlayer *who = NULL);
    void takeAG(ServerPlayer *player, int card_id);
    void provide(const Card *card);
    QList<ServerPlayer *> getLieges(const QString &kingdom, ServerPlayer *lord) const;
    void sendLog(const LogMessage &log);
    void showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer = NULL);
    void showAllCards(ServerPlayer *player, ServerPlayer *to = NULL);   
   
    // Ask a server player to execute a command and returns the client response. Call is blocking until client replies or
    // server times out, whichever is earlier.
    // @param player
    //        The server player to carry out the command.
    // @param packet
    //        Packet to be forwarded to the client.
    // @param timeOut
    //        Maximum milliseconds that server should wait for client response before returning.
    // @param broadcast
    //        Specifies whether other players should also be notified about the event.
    // @param moveFocus
    //        Suggests whether the all clients' UI should move focus to the player specified.
    // @param wait
    //        If ture, executeCommand will return immediately without waiting for client reply.
    // @return True if the a valid response is returned from client.  
    bool executeCommand(ServerPlayer* player, const QSanProtocol::QSanGeneralPacket* packet, time_t timeOut,
        bool broadcast = false, bool moveFocus = true, bool wait = true);
    
    // Ditto, except that default timeout from configuration is used.
    bool executeCommand(ServerPlayer* player, const QSanProtocol::QSanGeneralPacket* packet,
        bool broadcast = false, bool move_focus = true, bool wait = true);

    // Ditto, a specialization of executeCommand for S_SERVER_REQUEST packets.
    // Usage note: when you need a round trip request-response vector with a SINGLE client, use this command
    // with wait = true and read the reponse from player->getClientReply(). If you want to initiate a poll 
    // where more than one clients can respond simultaneously, you have to do it in two steps:
    // 1. Use this command with wait = false once for each client involved in the poll (or you can call this
    //    command only once in all with broadcast = true if the poll is to everypody).
    // 2. Call getResult(player, timeout) on each player to retrieve the result. Read manual for getResults
    //    before you use.
    bool doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, 
                            bool broadcast = false, bool moveFocus = true, bool wait = true);
    bool doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, time_t timeOut,
                            bool broadcast = false, bool moveFocus = true, bool wait = true);
    
    // Ditto, a specialization of executeCommand for S_SERVER_NOTIFICATION packets. No reply should be expected from
    // the client for S_SERVER_NOTIFICATION as it's a one way notice. Any message from the client in reply to this call
    // will be rejected.
    bool doNotify(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, 
                            bool broadcast = false); 


    // Ask a server player to execute a command and returns the client response. Call is blocking until client replies or
    // server times out, whichever is earlier.
    // @param player
    //        The server player to retrieve the client response.
    // @param timeOut
    //        Maximum milliseconds that server should wait for client response before returning.
    // @return True if the a valid response is returned from client.
    
    // Usage note: this function is only supposed to be either internally used by executeCommand (wait = true) or externally
    // used in pair with executeCommand (wait = false). Any other usage could result in unexpected synchronization errors. 
    // When getResult returns true, it's guaranteed that the expected client response has been stored and can be accessed by
    // calling player->getClientReply(). If getResult returns false, the value stored in player->getClientReply() could be
    // corrupted or in response to an unrelevant server request. Therefore, if the return value is false, do not poke into
    // player->getClientReply(), use the default value directly. If the return value is true, the reply value should still be
    // examined as a malicious client can have tampered with the content of the package for cheating purposes.
    bool getResult(ServerPlayer* player, time_t timeOut);

    void acquireSkill(ServerPlayer *player, const Skill *skill, bool open = true);
    void acquireSkill(ServerPlayer *player, const QString &skill_name, bool open = true);
    void adjustSeats();
    void swapPile();
    QList<int> getDiscardPile();
    QList<int> getDrawPile();
    int getCardFromPile(const QString &card_name);
    QList<ServerPlayer *> findPlayersBySkillName(const QString &skill_name, bool include_dead = false) const;
    ServerPlayer *findPlayer(const QString &general_name, bool include_dead = false) const;
    ServerPlayer *findPlayerBySkillName(const QString &skill_name, bool include_dead = false) const;
    void installEquip(ServerPlayer *player, const QString &equip_name);
    void resetAI(ServerPlayer *player);
    void transfigure(ServerPlayer *player, const QString &new_general, bool full_state, bool invoke_start = true, const QString &old_general = QString(""));
    void swapSeat(ServerPlayer *a, ServerPlayer *b);
    lua_State *getLuaState() const;
    void setFixedDistance(Player *from, const Player *to, int distance);
    void reverseFor3v3(const Card *card, ServerPlayer *player, QList<ServerPlayer *> &list);
    bool hasWelfare(const ServerPlayer *player) const;
    ServerPlayer *getFront(ServerPlayer *a, ServerPlayer *b) const;
    void signup(ServerPlayer *player, const QString &screen_name, const QString &avatar, bool is_robot);
    ServerPlayer *getOwner() const;
    void updateStateItem();

    void reconnect(ServerPlayer *player, ClientSocket *socket);
    void marshal(ServerPlayer *player);

    bool isVirtual();
    void setVirtual();
    void copyFrom(Room* rRoom);
    Room* duplicate();

    const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card) const;

    void setTag(const QString &key, const QVariant &value);
    QVariant getTag(const QString &key) const;
    void removeTag(const QString &key);

    void setEmotion(ServerPlayer *target, const QString &emotion);

    Player::Place getCardPlace(int card_id) const;
    ServerPlayer *getCardOwner(int card_id) const;
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);

    void drawCards(ServerPlayer *player, int n, const QString &reason = QString());
    void obtainCard(ServerPlayer *target, const Card *card);
    void obtainCard(ServerPlayer *target, int card_id);

    void throwCard(const Card *card, ServerPlayer *who = NULL);
    void throwCard(int card_id, ServerPlayer *who = NULL);
    void moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open = true);
    void doMove(const CardMoveStruct &move, const QSet<ServerPlayer *> &scope);

    // interactive methods
    void activate(ServerPlayer *player, CardUseStruct &card_use);
    Card::Suit askForSuit(ServerPlayer *player, const QString &reason);
    QString askForKingdom(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data = QVariant());
    QString askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices);
    bool askForDiscard(ServerPlayer *target, const QString &reason, int discard_num, bool optional = false, bool include_equip = false);
    const Card *askForExchange(ServerPlayer *player, const QString &reason, int discard_num);
    bool askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    bool isCanceled(const CardEffectStruct &effect);
    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason);
    const Card *askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt, const QVariant &data = QVariant());
    bool askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt);
    int askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable, const QString &reason);
    const Card *askForCardShow(ServerPlayer *player, ServerPlayer *requestor, const QString &reason);
    bool askForYiji(ServerPlayer *guojia, QList<int> &cards);
    const Card *askForPindian(ServerPlayer *player, ServerPlayer *from, ServerPlayer *to, const QString &reason);
    ServerPlayer *askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const QString &reason);
    QString askForGeneral(ServerPlayer *player, const QStringList &generals, QString default_choice = QString());
    void askForGeneralAsync(ServerPlayer *player);    
    const Card *askForSinglePeach(ServerPlayer *player, ServerPlayer *dying);
    
    //Get the timeout allowance for a command. Server countdown is more lenient than the client.
    //@param command: type of command
    //@return countdown for command in milliseconds.
    time_t getCommandTimeout(QSanProtocol::CommandType command);
    void toggleReadyCommand(ServerPlayer *player, const QString &);
    void speakCommand(ServerPlayer *player, const QString &arg);
    void trustCommand(ServerPlayer *player, const QString &arg);
    void kickCommand(ServerPlayer *player, const QString &arg);
    void surrenderCommand(ServerPlayer *player, const QString &);
    void interactiveCommand(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket* arg);
    void addRobotCommand(ServerPlayer *player, const QString &arg);
    void fillRobotsCommand(ServerPlayer *player, const QString &arg);
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const QSanProtocol::QSanPacket* packet, ServerPlayer *except = NULL);
    void broadcastInvoke(const char *method, const QString &arg = ".", ServerPlayer *except = NULL);
    void startTest(const QString &to_test);
    void networkDelayTestCommand(ServerPlayer *player, const QString &);

protected:
    virtual void run();

private:
    QString _chooseDefaultGeneral(ServerPlayer* player) const;
    bool _setPlayerGeneral(ServerPlayer* player, const QString& generalName, bool isFirst);
    QString mode;
    QList<ServerPlayer*> players, alive_players;
    int player_count;
    ServerPlayer *current;
    QList<int> pile1, pile2;
    QList<int> table_cards;
    QList<int> *draw_pile, *discard_pile;
    bool game_started;
    bool game_finished;
    lua_State *L;
    QList<AI *> ais;

    RoomThread *thread;
    RoomThread3v3 *thread_3v3;
    RoomThread1v1 *thread_1v1;
    QSemaphore *sem;
    QMutex _m_mutexInteractive;    

    
    QHash<QString, Callback> callbacks;
    QHash<QSanProtocol::CommandType, CallBack> m_callbacks;
    QHash<QSanProtocol::CommandType, QSanProtocol::CommandType> m_requestResponsePair;

    QMap<int, Player::Place> place_map;
    QMap<int, ServerPlayer*> owner_map;

    const Card *provided;
    bool has_provided;

    QVariantMap tag;
    const Scenario *scenario;

    bool _virtual;

    static QString generatePlayerName();
    void prepareForStart();
    void assignGeneralsForPlayers(const QList<ServerPlayer *> &to_assign);
    void chooseGenerals();
    AI *cloneAI(ServerPlayer *player);
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    void initCallbacks();
    void arrangeCommand(ServerPlayer *player, const QString &arg);
    void takeGeneralCommand(ServerPlayer *player, const QString &arg);
    QString askForOrder(ServerPlayer *player);
    QString askForRole(ServerPlayer *player, const QStringList &roles, const QString &scheme);

    //process client requests
    bool processRequestCheat(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket *packet);

    bool makeCheat(ServerPlayer* player);
    void makeDamage(const QString& source, const QString& target, DamageStruct::Nature nature, int point);
    void makeKilling(const QString& killer, const QString& victim);
    void makeReviving(const QString &name);
    void doScript(const QString &script);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &msg);
    void game_start();
    void game_over(const QString &winner);
};

#endif // ROOM_H
