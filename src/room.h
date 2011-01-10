#ifndef ROOM_H
#define ROOM_H

class TriggerSkill;
class ProhibitSkill;
class Scenario;
struct lua_State;

#include "serverplayer.h"
#include "roomthread.h"

struct LogMessage{
    LogMessage();
    QString toString() const;

    QString type;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    QString card_str;
    QString arg;
    QString arg2;
};

class Room : public QObject{
    Q_OBJECT

public:
    friend class RoomThread;
    typedef void (Room::*Callback)(ServerPlayer *, const QString &);

    explicit Room(QObject *parent, const QString &mode);
    void addSocket(ClientSocket *socket);
    bool isFull() const;
    bool isFinished() const;
    int getLack() const;
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    RoomThread *getThread() const;
    void playSkillEffect(const QString &skill_name, int index = -1);
    ServerPlayer *getCurrent() const;
    int alivePlayerCount() const;
    QList<ServerPlayer *> getOtherPlayers(ServerPlayer *except) const;
    QList<ServerPlayer *> getAllPlayers() const;
    QList<ServerPlayer *> getAlivePlayers() const;
    void nextPlayer();
    void output(const QString &message);
    void killPlayer(ServerPlayer *victim, ServerPlayer *killer = NULL);
    QStringList aliveRoles(ServerPlayer *except = NULL) const;
    void gameOver(const QString &winner);
    void slashEffect(const SlashEffectStruct &effect);
    void slashResult(const SlashEffectStruct &effect, bool hit);
    void attachSkillToPlayer(ServerPlayer *player, const QString &skill_name);
    void detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name);
    bool obtainable(const Card *card, ServerPlayer *player);
    void promptUser(ServerPlayer *to, const QString &prompt_str);
    void setPlayerFlag(ServerPlayer *player, const QString &flag);
    void setPlayerCorrect(ServerPlayer *player, const QString &correct_str);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void setPlayerMark(ServerPlayer *player, const QString &mark, int value);
    void useCard(const CardUseStruct &card_use);
    void damage(const DamageStruct &data);
    void sendDamageLog(const DamageStruct &data);
    void loseHp(ServerPlayer *victim, int lose = 1);
    void loseMaxHp(ServerPlayer *victim, int lose = 1);
    void damage(ServerPlayer *victim, int damage = 1);
    void recover(ServerPlayer *player, int recover = 1, bool set_emotion = false);
    void playCardEffect(const QString &card_name, bool is_male);
    bool cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to);
    bool cardEffect(const CardEffectStruct &effect);
    const Card *getJudgeCard(ServerPlayer *player);
    QList<int> getNCards(int n, bool update_pile_number = true);
    ServerPlayer *getLord() const;
    void doGuanxing(ServerPlayer *zhuge);
    void doGongxin(ServerPlayer *shenlumeng, ServerPlayer *target);
    int drawCard();   
    void takeAG(ServerPlayer *player, int card_id);
    void provide(const Card *card);
    QList<ServerPlayer *> getLieges(const QString &kingdom, ServerPlayer *lord) const;
    void sendLog(const LogMessage &log);
    void showCard(ServerPlayer *player, int card_id);   
    void getResult(const QString &reply_func, ServerPlayer *reply_player, bool move_focus = true);
    void acquireSkill(ServerPlayer *player, const Skill *skill, bool open = true);
    void acquireSkill(ServerPlayer *player, const QString &skill_name, bool open = true);
    void adjustSeats();
    void swapPile();
    int getCardFromPile(const QString &card_name);
    ServerPlayer *findPlayer(const QString &general_name, bool include_dead = false) const;
    ServerPlayer *findPlayerBySkillName(const QString &skill_name, bool include_dead = false) const;
    void installEquip(ServerPlayer *player, const QString &equip_name);
    void transfigure(ServerPlayer *player, const QString &new_general, bool full_state);
    lua_State *getLuaState() const;

    void addProhibitSkill(const ProhibitSkill *skill);
    const ProhibitSkill *isProhibited(Player *from, Player *to, const Card *card) const;

    void setTag(const QString &key, const QVariant &value);
    QVariant getTag(const QString &key) const;

    void skip(Player::Phase phase);
    bool isSkipped(Player::Phase phase);
    void resetSkipSet();

    enum TargetType{
        Killer,
        Victim,
        DuelA,
        DuelB,
        Good,
        Bad,
        Normal,
        Recover,
        DrawCard,
    };

    void setEmotion(ServerPlayer *target, TargetType type);

    // related to card transfer
    Player::Place getCardPlace(int card_id) const;
    ServerPlayer *getCardOwner(int card_id) const;
    ServerPlayer *getCardOwner(const Card *card) const;
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);

    void drawCards(ServerPlayer *player, int n);
    void obtainCard(ServerPlayer *target, const Card *card);
    void obtainCard(ServerPlayer *target, int card_id);

    void throwCard(const Card *card);
    void throwCard(int card_id);
    int throwSpecialCard();
    void moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open = true);
    void moveCardTo(int card_id, ServerPlayer *to, Player::Place place, bool open);

    // interactive methods
    void activate(ServerPlayer *player, CardUseStruct &card_use);
    Card::Suit askForSuit(ServerPlayer *player);
    QString askForKingdom(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data = QVariant());
    QString askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices);
    bool askForDiscard(ServerPlayer *target, int discard_num, bool optional = false, bool include_equip = false);
    bool askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to);
    bool isCanceled(const CardEffectStruct &effect);
    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason);
    const Card *askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt);
    bool askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt);
    int askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable = false);
    const Card *askForCardShow(ServerPlayer *player, ServerPlayer *requestor);
    bool askForYiji(ServerPlayer *guojia, QList<int> &cards);
    const Card *askForPindian(ServerPlayer *player, const QString &ask_str);    
    ServerPlayer *askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets);

    void askForPeaches(const DyingStruct &dying, const QList<ServerPlayer *> &players);
    int askForPeach(ServerPlayer *player, ServerPlayer *dying, int peaches);
    bool askForSinglePeach(ServerPlayer *player, ServerPlayer *dying, int peaches);

    void speakCommand(ServerPlayer *player, const QString &arg);
    void trustCommand(ServerPlayer *player, const QString &arg);
    void kickCommand(ServerPlayer *player, const QString &arg);
    void surrenderCommand(ServerPlayer *player, const QString &);
    void commonCommand(ServerPlayer *player, const QString &arg);
    void signupCommand(ServerPlayer *player, const QString &arg);
    void addRobotCommand(ServerPlayer *player, const QString &arg);
    void chooseCommand(ServerPlayer *player, const QString &general_name);
    void choose2Command(ServerPlayer *player, const QString &general_name);
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const char *method, const QString &arg = ".", ServerPlayer *except = NULL);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QString mode;
    QList<ServerPlayer*> players, alive_players;
    ServerPlayer *owner;
    int player_count;
    ServerPlayer *current;
    ServerPlayer *reply_player;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;
    bool game_started;
    bool game_finished;
    int signup_count;
    int special_card;
    lua_State *L;

    RoomThread *thread;
    QSemaphore *sem;
    QString result;
    QString reply_func;

    QHash<QString, Callback> callbacks;

    QMap<int, Player::Place> place_map;
    QMap<int, ServerPlayer*> owner_map;

    QSet<Player::Phase> skip_set;

    const Card *provided;

    QVariantMap tag;
    const Scenario *scenario;

    QList<const ProhibitSkill *> prohibit_skills;

    static QString generatePlayerName();
    void prepareForStart();
    AI *cloneAI(ServerPlayer *player);
    void signup(ServerPlayer *player, const QString &screen_name, const QString &avatar, bool is_robot);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
