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
#include "RoomState.h"
#include <qmutex.h>

class Room : public QThread
{
    Q_OBJECT

public:
    friend class RoomThread;
    friend class RoomThread3v3;
    friend class RoomThread1v1;

    typedef void (Room::*Callback)(ServerPlayer *, const QString &);
    typedef bool (Room::*CallBack)(ServerPlayer *, const QSanProtocol::QSanGeneralPacket*);
    typedef bool (Room::*ResponseVerifyFunction)(ServerPlayer*, const Json::Value&, void*);

    explicit Room(QObject *parent, const QString &mode);
    ServerPlayer *addSocket(ClientSocket *socket);
    inline int getId() const { return _m_Id; } 
    bool isFull() const;
    bool isFinished() const;
    int getLack() const;
    QString getMode() const;
    const Scenario *getScenario() const;
    RoomThread *getThread() const;
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
    void detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name, bool is_equip = false);
    void setPlayerFlag(ServerPlayer *player, const QString &flag);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void setPlayerMark(ServerPlayer *player, const QString &mark, int value);
    void setPlayerCardLock(ServerPlayer *player, const QString &name);
    void clearPlayerCardLock(ServerPlayer *player);
    void setCardFlag(const Card *card, const QString &flag, ServerPlayer *who = NULL);
    void setCardFlag(int card_id, const QString &flag, ServerPlayer *who = NULL);
    void clearCardFlag(const Card *card, ServerPlayer *who = NULL);
    void clearCardFlag(int card_id, ServerPlayer *who = NULL);
    void useCard(const CardUseStruct &card_use, bool add_history = true);
    void damage(DamageStruct &data);
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
    void doGongxin(ServerPlayer *shenlvmeng, ServerPlayer *target);
    int drawCard();
    QList<int> drawCards(int num);
    const Card *peek();
    void fillAG(const QList<int> &card_ids, ServerPlayer *who = NULL);
    void takeAG(ServerPlayer *player, int card_id);
    void provide(const Card *card);
    QList<ServerPlayer *> getLieges(const QString &kingdom, ServerPlayer *lord) const;
    void sendLog(const LogMessage &log);
    void showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer = NULL);
    void showAllCards(ServerPlayer *player, ServerPlayer *to = NULL);   

    void retrial(const Card *card, ServerPlayer *player, JudgeStar judge,
                 const QString &skill_name, bool exchange = false);
   
    // Ask a player to send a server request and returns the client response. Call is blocking until client 
    // replies or server times out, whichever is earlier.
    // @param player
    //        The server player to carry out the command.
    // @param command
    //        Command to be executed on client side.
    // @param arg
    //        Command args.
    // @param timeOut
    //        Maximum milliseconds that server should wait for client response before returning.        
    // @param wait
    //        If true, return immediately after sending the request without waiting for client reply.
    // @return True if the a valid response is returned from client.  
    // Usage note: when you need a round trip request-response vector with a SINGLE client, use this command
    // with wait = true and read the reponse from player->getClientReply(). If you want to initiate a poll 
    // where more than one clients can respond simultaneously, you have to do it in two steps:
    // 1. Use this command with wait = false once for each client involved in the poll (or you can call this
    //    command only once in all with broadcast = true if the poll is to everypody).
    // 2. Call getResult(player, timeout) on each player to retrieve the result. Read manual for getResults
    //    before you use.
    bool doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, time_t timeOut, bool wait);
    bool doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, bool wait);

    // Broadcast a request to a list of players and get the client responses. Call is blocking until all client
    // replies or server times out, whichever is earlier. Check each player's m_isClientResponseReady to see if a valid
    // result has been received. The client response can be accessed by calling each player's getClientReply() function. 
    // @param players
    //        The list of server players to carry out the command.
    // @param command
    //        Command to be executed on client side. Command arguments should be stored in players->m_commandArgs.
    // @param timeOut
    //        Maximum total milliseconds that server will wait for all clients to respond before returning. Any client 
    //        response after the timeOut will be rejected.
    // @return True if the a valid response is returned from client.  
    bool doBroadcastRequest(QList<ServerPlayer*> &players, QSanProtocol::CommandType command, time_t timeOut);
    bool doBroadcastRequest(QList<ServerPlayer*> &players, QSanProtocol::CommandType command);

    // Broadcast a request to a list of players and get the first valid client response. Call is blocking until the first
    // client response is received or server times out, whichever is earlier. Any client response is verified by the validation
    // function and argument passed in. When a response is verified to be invalid, the function will continue to wait for
    // the next client response.
    // @param validateFunc
    //        Validation function that verifies whether the reply is a valid one. The first parameter passed to the function
    //        is the response sender, the second parameter is the response content, the third parameter is funcArg passed in.
    // @return The player that first send a legal request to the server. NULL if no such request is received.
    ServerPlayer* doBroadcastRaceRequest(QList<ServerPlayer*> &players, QSanProtocol::CommandType command, 
           time_t timeOut, ResponseVerifyFunction validateFunc = NULL, void* funcArg = NULL);
    
    // Notify a player of a event by sending S_SERVER_NOTIFICATION packets. No reply should be expected from
    // the client for S_SERVER_NOTIFICATION as it's a one way notice. Any message from the client in reply to this call
    // will be rejected.
    bool doNotify(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg); 

    // Broadcast a event to a list of players by sending S_SERVER_NOTIFICATION packets. No replies should be expected from
    // the clients for S_SERVER_NOTIFICATION as it's a one way notice. Any message from the client in reply to this call
    // will be rejected.    
    bool doBroadcastNotify(QSanProtocol::CommandType command, const Json::Value &arg);
    bool doBroadcastNotify(const QList<ServerPlayer*> &players, QSanProtocol::CommandType command, const Json::Value &arg);
    
    // Ask a server player to wait for the client response. Call is blocking until client replies or server times out, 
    // whichever is earlier.
    // @param player
    //        The server player to retrieve the client response.
    // @param timeOut
    //        Maximum milliseconds that server should wait for client response before returning.
    // @return True if the a valid response is returned from client.
    
    // Usage note: this function is only supposed to be either internally used by doRequest (wait = true) or externally
    // used in pair with doRequest (wait = false). Any other usage could result in unexpected synchronization errors. 
    // When getResult returns true, it's guaranteed that the expected client response has been stored and can be accessed by
    // calling player->getClientReply(). If getResult returns false, the value stored in player->getClientReply() could be
    // corrupted or in response to an unrelevant server request. Therefore, if the return value is false, do not poke into
    // player->getClientReply(), use the default value directly. If the return value is true, the reply value should still be
    // examined as a malicious client can have tampered with the content of the package for cheating purposes.
    bool getResult(ServerPlayer* player, time_t timeOut);
    ServerPlayer* getRaceResult(QList<ServerPlayer*> &players, QSanProtocol::CommandType command, time_t timeOut,
                                ResponseVerifyFunction validateFunc = NULL, void* funcArg = NULL);

    // Verification functions
    bool verifyNullificationResponse(ServerPlayer*, const Json::Value&, void*);

    // Notification functions
    bool notifyMoveFocus(ServerPlayer* player);
    bool notifyMoveFocus(ServerPlayer* player, QSanProtocol::CommandType command);
    bool notifyMoveFocus(const QList<ServerPlayer*> &players, QSanProtocol::CommandType command, QSanProtocol::Countdown countdown);

    // Notify client side to move cards from one place to another place. A movement should always be completed by
    // calling notifyMoveCards in pairs, one with isLostPhase equaling true followed by one with isLostPhase
    // equaling false. The two phase design is needed because the target player doesn't necessarily gets the 
    // cards that the source player lost. Any trigger during the movement can cause either the target player to
    // be dead or some of the cards to be moved to another place before the target player actually gets it. 
    // @param isLostPhase
    //        Specify whether this is a S_COMMAND_LOSE_CARD notification.
    // @param move
    //        Specify all movements need to be broadcasted.
    // @param forceVisible
    //        If true, all players will be able to see the face of card regardless of whether the movement is
    //        relevant or not.
    bool notifyMoveCards(bool isLostPhase, QList<CardsMoveStruct> move, bool forceVisible);
    bool notifyProperty(ServerPlayer* playerToNotify, const ServerPlayer* propertyOwner, const char *propertyName, const QString &value = QString());
    bool notifyUpdateCard(ServerPlayer* player, int cardId, const Card* newCard);
    bool broadcastUpdateCard(const QList<ServerPlayer*> &players, int cardId, const Card* newCard);
    bool notifyResetCard(ServerPlayer* player, int cardId);
    bool broadcastResetCard(const QList<ServerPlayer*> &players, int cardId);

    bool broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    bool broadcastSkillInvoke(const QString &skillName);
    bool broadcastSkillInvoke(const QString &skillName, const QString &category);
    bool broadcastSkillInvoke(const QString &skillName, int type);
    bool broadcastSkillInvoke(const QString &skillName, bool isMale, int type);

    void preparePlayers();
    void changePlayerGeneral(ServerPlayer *player, const QString &new_general);
    void changePlayerGeneral2(ServerPlayer *player, const QString &new_general);
    void filterCards(ServerPlayer *player, QList<const Card *> cards, bool refilter);

    void acquireSkill(ServerPlayer *player, const Skill *skill, bool open = true);
    void acquireSkill(ServerPlayer *player, const QString &skill_name, bool open = true);
    void adjustSeats();
    void swapPile();
    QList<int> getDiscardPile();
    inline QList<int>& getDrawPile() { return *m_drawPile; }
    inline const QList<int>& getDrawPile() const { return *m_drawPile; }
    int getCardFromPile(const QString &card_name);
    QList<ServerPlayer *> findPlayersBySkillName(const QString &skill_name, bool include_dead = false) const;
    ServerPlayer *findPlayer(const QString &general_name, bool include_dead = false) const;
    ServerPlayer *findPlayerBySkillName(const QString &skill_name, bool include_dead = false) const;
    void installEquip(ServerPlayer *player, const QString &equip_name);
    void resetAI(ServerPlayer *player);
    void changeHero(ServerPlayer *player, const QString &new_general, bool full_state, bool invoke_start = true,
                    bool isSecondaryHero = false, bool sendLog = true);
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
    void drawCards(QList<ServerPlayer*> players, int n, const QString &reason);
    void obtainCard(ServerPlayer *target, const Card *card, bool unhide = true);
    void obtainCard(ServerPlayer *target, int card_id, bool unhide = true);
    void obtainCard(ServerPlayer *target, const Card *card,  const CardMoveReason &reason, bool unhide = true);

    void throwCard(int card_id, ServerPlayer *who, ServerPlayer *thrower = NULL);
    void throwCard(const Card *card, ServerPlayer *who, ServerPlayer *thrower = NULL);
    void throwCard(const Card *card, const CardMoveReason &reason, ServerPlayer *who, ServerPlayer *thrower = NULL);

    void moveCardTo(const Card* card, ServerPlayer* dstPlayer, Player::Place dstPlace,
                    bool forceMoveVisible = false);
    void moveCardTo(const Card* card, ServerPlayer* dstPlayer, Player::Place dstPlace, const CardMoveReason &reason,
                    bool forceMoveVisible = false);
    void moveCardTo(const Card* card, ServerPlayer* srcPlayer, ServerPlayer* dstPlayer, Player::Place dstPlace, const CardMoveReason &reason,
                    bool forceMoveVisible = false);
    void moveCardTo(const Card* card, ServerPlayer* srcPlayer, ServerPlayer* dstPlayer, Player::Place dstPlace, const QString& pileName,
                    const CardMoveReason &reason, bool forceMoveVisible = false);
    void moveCardsAtomic(QList<CardsMoveStruct> cards_move, bool forceMoveVisible);
    void moveCardsAtomic(CardsMoveStruct cards_move, bool forceMoveVisible);
    void moveCards(CardsMoveStruct cards_move, bool forceMoveVisible, bool ignoreChanges = true);
    void moveCards(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible, bool ignoreChanges = true);
    QList<CardsMoveStruct> _breakDownCardMoves(QList<CardsMoveStruct> &cards_moves);

    // interactive methods
    void activate(ServerPlayer *player, CardUseStruct &card_use);
    Card::Suit askForSuit(ServerPlayer *player, const QString &reason);
    QString askForKingdom(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data = QVariant());
    QString askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices, const QVariant &data = QVariant());
    bool askForDiscard(ServerPlayer *target, const QString &reason, int discard_num, int min_num,
                       bool optional = false, bool include_equip = false, const QString &prompt = QString());
    const Card *askForExchange(ServerPlayer *player, const QString &reason, int discard_num, bool include_equip = false,
                       const QString &prompt = QString(), bool optional = false);
    bool askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    bool isCanceled(const CardEffectStruct &effect);
    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason);
    const Card *askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt,
                           const QVariant &data = QVariant(), TriggerEvent trigger_event = CardResponsed, ServerPlayer *to = NULL);
    bool askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt, int notice_index = -1);
    bool askForUseSlashTo(ServerPlayer *slasher, ServerPlayer *victim, const QString &prompt);
    bool askForUseSlashTo(ServerPlayer *slasher, QList<ServerPlayer *> victims, const QString &prompt);
    int askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable, const QString &reason);
    const Card *askForCardShow(ServerPlayer *player, ServerPlayer *requestor, const QString &reason);
    bool askForYiji(ServerPlayer *guojia, QList<int> &cards);
    const Card *askForPindian(ServerPlayer *player, ServerPlayer *from, ServerPlayer *to, const QString &reason);
    ServerPlayer *askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const QString &reason);
    QString askForGeneral(ServerPlayer *player, const QStringList &generals, QString default_choice = QString());    
    QString askForGeneral(ServerPlayer *player, const QString &generals, QString default_choice = QString());
    const Card *askForSinglePeach(ServerPlayer *player, ServerPlayer *dying);
    
    void toggleReadyCommand(ServerPlayer *player, const QString &);
    void speakCommand(ServerPlayer *player, const QString &arg);
    void trustCommand(ServerPlayer *player, const QString &arg);
    void kickCommand(ServerPlayer *player, const QString &arg);
    void processResponse(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket* arg);
    void addRobotCommand(ServerPlayer *player, const QString &arg);
    void fillRobotsCommand(ServerPlayer *player, const QString &arg);
    void broadcastInvoke(const QSanProtocol::QSanPacket* packet, ServerPlayer *except = NULL);
    void broadcastInvoke(const char *method, const QString &arg = ".", ServerPlayer *except = NULL);
    void networkDelayTestCommand(ServerPlayer *player, const QString &);
    inline virtual RoomState* getRoomState() { return &_m_roomState; }
    inline virtual Card* getCard(int cardId) const { return _m_roomState.getCard(cardId); }
    inline virtual void resetCard(int cardId) { _m_roomState.resetCard(cardId); }
    virtual void updateCardsOnLose(const CardsMoveStruct &move);
    virtual void updateCardsOnGet(const CardsMoveStruct &move);

protected:
    virtual void run();
    int _m_Id;

private:
    struct _MoveSourceClassifier
    {
        inline _MoveSourceClassifier(const CardsMoveStruct &move)
        {
            m_from = move.from; m_from_place = move.from_place; 
            m_from_pile_name = move.from_pile_name; m_from_player_name = move.from_player_name;
        }
        inline void copyTo(CardsMoveStruct & move)
        {
            move.from = m_from; move.from_place = m_from_place;
            move.from_pile_name = m_from_pile_name; move.from_player_name = m_from_player_name;
        }
        inline bool operator == (const _MoveSourceClassifier &other) const
        {
            return m_from == other.m_from && m_from_place == other.m_from_place &&
                m_from_pile_name == other.m_from_pile_name && m_from_player_name == other.m_from_player_name;
        }
        inline bool operator < (const _MoveSourceClassifier &other) const
        {
            return m_from < other.m_from || m_from_place < other.m_from_place ||
                m_from_pile_name < other.m_from_pile_name || m_from_player_name < other.m_from_player_name;
        }
        Player* m_from;
        Player::Place m_from_place;
        QString m_from_pile_name;
        QString m_from_player_name; 
    };

    struct _MoveMergeClassifier
    {
        inline _MoveMergeClassifier(const CardsMoveStruct &move)
        {
            m_from = move.from; m_to = move.to;
        }
        inline bool operator == (const _MoveMergeClassifier &other) const
        {
            return m_from == other.m_from && m_to == other.m_to;
        }
        inline bool operator < (const _MoveMergeClassifier &other) const
        {
            return m_from < other.m_from || m_to < other.m_to;
        }
        Player* m_from;
        Player* m_to;
    };

    int _m_lastMovementId;
    void _fillMoveInfo(CardMoveStruct &move) const;
    void _fillMoveInfo(CardsMoveStruct &moves, int card_index) const;
    QList<CardsMoveOneTimeStruct> _mergeMoves(QList<CardsMoveStruct> cards_moves);
    void _moveCards(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible, bool ignoreChanges);
    QString _chooseDefaultGeneral(ServerPlayer* player) const;
    bool _setPlayerGeneral(ServerPlayer* player, const QString& generalName, bool isFirst);
    QString mode;
    QList<ServerPlayer*> m_players, m_alivePlayers;
    int player_count;
    ServerPlayer *current;
    QList<int> pile1, pile2;
    QList<int> table_cards;
    QList<int> *m_drawPile, *m_discardPile;
    bool game_started;
    bool game_finished;
    lua_State *L;
    QList<AI *> ais;

    RoomThread *thread;
    RoomThread3v3 *thread_3v3;
    RoomThread1v1 *thread_1v1;
    QSemaphore *sem; // Legacy semaphore, expected to be reomved after new synchronization is fully deployed.
    QSemaphore _m_semRaceRequest; // When race starts, server waits on his semaphore for the first replier
    QSemaphore _m_semRoomMutex; // Provide per-room  (rather than per-player) level protection of any shared variables

    
    QHash<QString, Callback> callbacks; // Legacy protocol callbacks
    QHash<QSanProtocol::CommandType, CallBack> m_callbacks; // Stores the callbacks for client request. Do not use this
                                                            // this map for anything else but S_CLIENT_REQUEST!!!!!
    QHash<QSanProtocol::CommandType, QSanProtocol::CommandType> m_requestResponsePair; 
        // Stores the expected client response for each server request, any unmatched client response will be discarded.

    QTime _m_timeSinceLastSurrenderRequest; // Timer used to ensure that surrender polls are not initiated too frequently
    bool _m_isFirstSurrenderRequest; // We allow the first surrender poll to go through regardless of the timer.
    
    //helper variables for race request function
    bool _m_raceStarted; 
    ServerPlayer* _m_raceWinner;

    QMap<int, Player::Place> place_map;
    QMap<int, ServerPlayer*> owner_map;

    const Card *provided;
    bool has_provided;

    QVariantMap tag;
    const Scenario *scenario;

    bool m_surrenderRequestReceived;
    bool _virtual;
    RoomState _m_roomState;

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
    bool processRequestSurrender(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket *packet);

    bool makeSurrender(ServerPlayer* player);
    bool makeCheat(ServerPlayer* player);
    void makeDamage(const QString& source, const QString& target, QSanProtocol::CheatCategory nature, int point);
    void makeKilling(const QString& killer, const QString& victim);
    void makeReviving(const QString &name);
    void doScript(const QString &script);

    //helper functions and structs
    struct _NullificationAiHelper
    {
        const TrickCard* m_trick;
        ServerPlayer* m_from;
        ServerPlayer* m_to;
    };
    bool _askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive, _NullificationAiHelper helper);
    void _setupChooseGeneralRequestArgs(ServerPlayer *player);    

private slots:
    void reportDisconnection();
    void processClientPacket(const QString &packet);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &msg);
    void game_start();
    void game_over(const QString &winner);
};

typedef Room *RoomStar;
Q_DECLARE_METATYPE(RoomStar)

#endif // ROOM_H
