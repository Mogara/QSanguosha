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
    QList<ServerPlayer *> getOtherPlayers(ServerPlayer *except);
    QList<ServerPlayer *> getAllPlayers();
    ServerPlayer *getNextPlayer(ServerPlayer *player);
    void output(const QString &message);
    void obit(ServerPlayer *victim, ServerPlayer *killer);
    void bury(ServerPlayer *player);
    QStringList aliveRoles(ServerPlayer *except = NULL) const;
    void gameOver(const QString &winner);
    void slashResult(const SlashResultStruct &slash_result);

    bool obtainable(const Card *card, ServerPlayer *player);
    void promptUser(ServerPlayer *to, const QString &prompt_str);
    void nextPhase(ServerPlayer *player);
    void drawCards(ServerPlayer *player, int n);
    void setPlayerFlag(ServerPlayer *player, const QString &flag);
    void setPlayerCorrect(ServerPlayer *player, const QString &field, int correct);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void changePhase(ServerPlayer *target);
    void moveCard(const CardMoveStruct &move);
    void moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open = true);
    void useCard(ServerPlayer *player, const QString &card_str);
    void damage(const DamageStruct &data);
    void obtainCard(ServerPlayer *target, const Card *card);
    void obtainCard(ServerPlayer *target, int card_id);
    void damage(ServerPlayer *victim, int damage = 1);
    void recover(ServerPlayer *player, int recover = 1);
    void playCardEffect(const QString &card_name, bool is_male);
    void cardEffect(const CardEffectStruct &effect);
    void setLegatee(ServerPlayer *legatee);
    int getJudgeCard(ServerPlayer *player);

    // interactive methods
    QString activate(ServerPlayer *player);
    void useCardCommand(ServerPlayer *player, const QString &card_str);

    Card::Suit askForSuit(ServerPlayer *player);
    void chooseSuitCommand(ServerPlayer *player, const QString &suit_str);

    QString askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QString &options);
    bool askForSkillInvoke(ServerPlayer *player, const QString &skill_name);
    void invokeSkillCommand(ServerPlayer *player, const QString &arg);

    QList<int> askForDiscard(ServerPlayer *target, int discard_num);
    void discardCardCommand(ServerPlayer *player, const QString &arg);

    void askForNullification(ServerPlayer *player, const QVariant &data);
    void replyNullificationCommand(ServerPlayer *player, const QString &arg);

    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason);
    void chooseCardCommand(ServerPlayer *player, const QString &arg);

    const Card *askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt);
    void responseCardCommand(ServerPlayer *player, const QString &arg);

    void signupCommand(ServerPlayer *player, const QString &arg);
    void chooseCommand(ServerPlayer *player, const QString &general_name);

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
    QList<ServerPlayer *> nullificators;

    RoomThread *thread;
    QSemaphore *sem;
    QString result;
    QString reply_func;

    QHash<QString, Callback> callbacks;

    QMap<int, Player::Place> place_map;
    QMap<int, ServerPlayer*> owner_map;
    ServerPlayer *legatee;

    int drawCard();
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const char *method, const QString &arg = ".");
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);
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
