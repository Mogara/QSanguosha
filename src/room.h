#ifndef ROOM_H
#define ROOM_H

class TriggerSkill;

#include "serverplayer.h"
#include "roomthread.h"

#include <QTcpSocket>

class Room : public QObject{
    Q_OBJECT

public:
    friend class RoomThread;
    typedef void (Room::*Callback)(ServerPlayer *, const QString &);

    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;    
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    void throwCard(const Card *card);
    void throwCard(int card_id);
    RoomThread *getThread() const;
    void playSkillEffect(const QString &skill_name, int index = -1);
    Player::Place getCardPlace(int card_id) const;
    ServerPlayer *getCardOwner(int card_id) const;
    ServerPlayer *getCurrent() const;
    int alivePlayerCount() const;
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);
    QList<ServerPlayer *> getOtherPlayers(ServerPlayer *except);
    QList<ServerPlayer *> getAllPlayers();
    void nextPlayer();
    void output(const QString &message);
    void obit(ServerPlayer *victim, ServerPlayer *killer);
    void bury(ServerPlayer *player);
    QStringList aliveRoles(ServerPlayer *except = NULL) const;
    void gameOver(const QString &winner);
    void slashEffect(const SlashEffectStruct &effect);
    void slashResult(const SlashResultStruct &result);
    void attachSkillToPlayer(ServerPlayer *player, const QString &skill_name);
    void detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name);
    bool obtainable(const Card *card, ServerPlayer *player);
    void promptUser(ServerPlayer *to, const QString &prompt_str);
    void drawCards(ServerPlayer *player, int n);
    void setPlayerFlag(ServerPlayer *player, const QString &flag);
    void setPlayerCorrect(ServerPlayer *player, const QString &field, int correct);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void setPlayerMark(ServerPlayer *player, const QString &mark, int value);
    void moveCard(const CardMoveStruct &move);
    void moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open = true);
    void useCard(ServerPlayer *player, const QString &card_str);
    void damage(const DamageStruct &data);
    void obtainCard(ServerPlayer *target, const Card *card);
    void obtainCard(ServerPlayer *target, int card_id);
    void loseHp(ServerPlayer *victim);
    void damage(ServerPlayer *victim, int damage = 1);
    void recover(ServerPlayer *player, int recover = 1);
    void playCardEffect(const QString &card_name, bool is_male);
    void cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to);
    void cardEffect(const CardEffectStruct &effect);
    void setLegatee(ServerPlayer *legatee);
    int getJudgeCard(ServerPlayer *player);
    QList<int> getNCards(int n, bool update_pile_number = true);
    void setMenghuo(ServerPlayer *menghuo);
    ServerPlayer *getMenghuo() const;
    void skip(Player::Phase phase);
    bool isSkipped(Player::Phase phase);
    ServerPlayer *getLord() const;
    void doGuanxing(ServerPlayer *zhuge);
    int drawCard();
    const Card *askForPindian(ServerPlayer *player);
    bool pindian(ServerPlayer *source, ServerPlayer *target);

    // interactive methods
    QString activate(ServerPlayer *player);
    Card::Suit askForSuit(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const QString &skill_name);
    QString askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices);
    bool askForDiscard(ServerPlayer *target, int discard_num, bool optional = false);
    bool askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to);
    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason);
    const Card *askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt);
    const Card *askForCardWithTargets(ServerPlayer *, const QString &pattern, const QString &prompt, QList<ServerPlayer *> &targets);
    int askForAG(ServerPlayer *player);
    int askForCardShow(ServerPlayer *player, ServerPlayer *requestor);

    bool askForSave(ServerPlayer *dying, int peaches);
    int askForPeach(ServerPlayer *player, ServerPlayer *dying, int peaches);
    bool askForSinglePeach(ServerPlayer *player, ServerPlayer *dying, int peaches);

    void commonCommand(ServerPlayer *player, const QString &arg);
    void replyNullificationCommand(ServerPlayer *player, const QString &arg);
    void signupCommand(ServerPlayer *player, const QString &arg);
    void chooseCommand(ServerPlayer *player, const QString &general_name);

    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const char *method, const QString &arg = ".");

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QList<ServerPlayer*> players, alive_players;
    const int player_count;
    ServerPlayer *current;
    ServerPlayer *reply_player;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;
    bool game_started;
    bool game_finished;
    int signup_count;

    int nullificators_count;
    ServerPlayer *nullificator;
    QMap<ServerPlayer *, bool> nullificator_map;

    RoomThread *thread;
    QSemaphore *sem;
    QString result;
    QString reply_func;

    QHash<QString, Callback> callbacks;

    QMap<int, Player::Place> place_map;
    QMap<int, ServerPlayer*> owner_map;
    ServerPlayer *legatee;

    ServerPlayer *menghuo;
    QSet<Player::Phase> skip_set;


    void moveCardTo(int card_id, ServerPlayer *to, Player::Place place, bool open = true);    

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
