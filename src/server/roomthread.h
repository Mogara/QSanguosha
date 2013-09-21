#ifndef _ROOM_THREAD_H
#define _ROOM_THREAD_H

#include <QThread>
#include <QSemaphore>
#include <QVariant>

#include "structs.h"

class GameRule;

struct LogMessage {
    LogMessage();
    QString toString() const;
    Json::Value toJsonValue() const;

    QString type;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    QString card_str;
    QString arg;
    QString arg2;
};

class EventTriplet {
public:
    inline EventTriplet(TriggerEvent triggerEvent, Room *room, ServerPlayer *target)
               : _m_event(triggerEvent), _m_room(room), _m_target(target) {}
    QString toString() const;

private:
    TriggerEvent _m_event;
    Room *_m_room;
    ServerPlayer *_m_target;
};

class RoomThread: public QThread {
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    void constructTriggerTable();
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);

    void addTriggerSkill(const TriggerSkill *skill);
    void delay(long msecs = -1);
    ServerPlayer *find3v3Next(QList<ServerPlayer *> &first, QList<ServerPlayer *> &second);
    void run3v3(QList<ServerPlayer *> &first, QList<ServerPlayer *> &second, GameRule *game_rule, ServerPlayer *current);
    void actionHulaoPass(ServerPlayer *shenlvbu, QList<ServerPlayer *> league, GameRule *game_rule, int stage);
    ServerPlayer *findHulaoPassNext(ServerPlayer *shenlvbu, QList<ServerPlayer *> league, int stage);
    void actionNormal(GameRule *game_rule);

    const QList<EventTriplet> *getEventStack() const;

protected:
    virtual void run();

private:
    void _handleTurnBroken3v3(QList<ServerPlayer *> &first, QList<ServerPlayer *> &second, GameRule *game_rule);
    void _handleTurnBrokenHulaoPass(ServerPlayer *shenlvbu, QList<ServerPlayer *> league, GameRule *game_rule, int stage);
    void _handleTurnBrokenNormal(GameRule *game_rule);

    Room *room;
    QString order;

    QList<const TriggerSkill *> skill_table[NumOfEvents];
    QSet<QString> skillSet;

    QList<EventTriplet> event_stack;
};

#endif

