#ifndef ROOM_H
#define ROOM_H

class PassiveSkill;

#include "serverplayer.h"
#include "roomthread.h"

#include <QTcpSocket>

class Room : public QObject{
    Q_OBJECT

public:
    friend class RoomThread;

    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;    
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    void throwCard(ServerPlayer *player, const Card *card);

    QList<int> *getDiscardPile() const;    
    void playSkillEffect(const QString &skill_name, int index = -1);

    ServerPlayer *getCurrent() const;
    int alivePlayerCount() const;

    // non-interactive methods    
    void nextPhase(ServerPlayer *player);
    void drawCards(ServerPlayer *player, int n);
    void setPlayerFlag(ServerPlayer *player, const QString &flag);
    void changePhase(ServerPlayer *target);
    void throwCard(ServerPlayer *player, int card_id);
    void moveCard(ServerPlayer *, const CardMoveStruct &move);
    void useCard(ServerPlayer *player, const QString &card_str);

    // interactive methods
    QString activate(ServerPlayer *target);
    Q_INVOKABLE void useCardCommand(ServerPlayer *player, const QString &card_str);

    QString askForSkillInvoke(ServerPlayer *player, const QString &ask_str);
    Q_INVOKABLE void invokeSkillCommand(ServerPlayer *player, const QString &arg);

    void askForNullification(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void replyNullificationCommand(ServerPlayer *player, const QStringList &args);

    void askForCardChosen(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void chooseCardCommand(ServerPlayer *player, const QStringList &args);

    void requestForCard(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void responseCardCommand(ServerPlayer *player, const QStringList &args);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QList<ServerPlayer*> players, alive_players;
    const int player_count;
    ServerPlayer *current;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;
    bool game_started;
    int signup_count;

    int nullificators_count;
    QList<ServerPlayer *> nullificators;

    RoomThread *thread;
    QSemaphore *sem;
    QString result;
    QString reply_func;

    int drawCard();
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const char *method, const QString &arg = ".");

    Q_INVOKABLE void signupCommand(ServerPlayer *player, const QString &arg);
    Q_INVOKABLE void chooseCommand(ServerPlayer *player, const QString &general_name);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
