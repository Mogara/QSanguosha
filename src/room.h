#ifndef ROOM_H
#define ROOM_H

#include "serverplayer.h"
#include "skill.h"
#include "cardpattern.h"

#include <QTcpSocket>
#include <QStack>

class Room;

struct ActiveRecord{
    ServerPlayer *source;
    ServerPlayer *target;
    const CardPattern *pattern;
    const Card *card;
};

class ActiveRecordSorter{
public:
    ActiveRecordSorter(Room *room);
    void setSource(const ServerPlayer *source);
    void setTarget(const ServerPlayer *target);

    void sort(QList<ActiveRecord*> &records) const;
    bool operator()(const ActiveRecord *a, const ActiveRecord *b) const;

private:
    Room *room;
    const ServerPlayer *source, *target;
};

class Room : public QObject
{
    Q_OBJECT
public:
    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;
    void drawCards(ServerPlayer *player, int n);
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    void throwCard(ServerPlayer *player, const Card *card);
    void throwCard(ServerPlayer *player, int card_id);
    QList<int> *getDiscardPile() const;
    void moveCard(ServerPlayer *src, Player::Place src_place, ServerPlayer *dest, Player::Place dest_place, int card_id);
    void activate(ServerPlayer *player, Skill::TriggerReason reason = Skill::Nop, const QString &data = "");
    void requestForCard(ServerPlayer *source, ServerPlayer *target, const CardPattern *pattern);
    void startRequest();

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QList<ServerPlayer*> players, alive_players;
    const int player_count;
    Player *focus;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;
    bool game_started;
    int signup_count;
    QStack<ActiveRecord*> active_records;
    QList<const PassiveSkill *> skills;
    ActiveRecordSorter sorter;

    int drawCard();
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());

    Q_INVOKABLE void setCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void signupCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void chooseCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void useCardCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void endPhaseCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void drawCardsCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void judgeCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void yesCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void noCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void nullifyCommand(ServerPlayer *player, const QStringList &args);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};




#endif // ROOM_H
