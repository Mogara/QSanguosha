#ifndef ROOM_H
#define ROOM_H

class PassiveSkill;

#include "serverplayer.h"
#include "cardpattern.h"

#include <QTcpSocket>
#include <QStack>

struct PassiveSkillSorter{
    ServerPlayer *target;

    bool operator()(const PassiveSkill *a, const PassiveSkill *b);
    void sort(QList<const PassiveSkill *> &skills);
};

struct ActiveRecord{
    const char *method;
    QList<QGenericArgument> args;
};

class Room : public QObject
{
    Q_OBJECT

public:
    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;    
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    void throwCard(ServerPlayer *player, const Card *card);
    void throwCard(ServerPlayer *player, int card_id);
    QList<int> *getDiscardPile() const;
    void moveCard(ServerPlayer *src, Player::Place src_place, ServerPlayer *dest, Player::Place dest_place, int card_id);

    QList<const PassiveSkill *> getInvokableSkills(ServerPlayer *target) const;

    void pushActiveRecord(ActiveRecord *record);
    ServerPlayer *getCurrent() const;
    int alivePlayerCount() const;

    Q_INVOKABLE void activate(const ServerPlayer *target);
    Q_INVOKABLE void nextPhase(ServerPlayer *player);
    Q_INVOKABLE void drawCards(ServerPlayer *player, int n);
    Q_INVOKABLE void askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QString &options);

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
    QMap<QString, const PassiveSkill *> passive_skills;
    QStack<ActiveRecord *> stack;

    int drawCard();
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const char *method, const QString &arg = ".");
    void invokeStackTop();

    // method that may invoke skills
    void changePhase(ServerPlayer *target);

    Q_INVOKABLE void setCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void signupCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void chooseCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void useCardCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void judgeCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void invokeSkillCommand(ServerPlayer *player, const QStringList &args);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
