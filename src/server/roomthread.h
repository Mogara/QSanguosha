#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QVariant>

#include <csetjmp>

#include "structs.h"

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

class EventTriplet{
public:
    inline EventTriplet(TriggerEvent event, Room* room, ServerPlayer *target, QVariant *data)
        :_m_event(event), _m_room(room), _m_target(target), _m_data(data){}
    QString toString() const;

private:
    TriggerEvent _m_event;
    Room* _m_room;
    ServerPlayer *_m_target;
    QVariant *_m_data;
};

class RoomThread : public QThread{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    void constructTriggerTable();
    bool trigger(TriggerEvent event, Room* room, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent event, Room* room, ServerPlayer *target);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);

    void addTriggerSkill(const TriggerSkill *skill);
    void delay(unsigned long msecs = 1000);
    void run3v3();
    void action3v3(ServerPlayer *player);

    const QList<EventTriplet> *getEventStack() const;

protected:
    virtual void run();

private:
    Room *room;
    QString order;

    QList<const TriggerSkill *> skill_table[NumOfEvents];
	// I temporarily use the set of skill names instead of skill itself
	// in order to avoid duplication. Maybe we need a better solution
	QSet<QString> skillSet;

    QList<EventTriplet> event_stack;
};

#endif // ROOMTHREAD_H
