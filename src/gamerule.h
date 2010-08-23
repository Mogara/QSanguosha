#ifndef GAMERULE_H
#define GAMERULE_H

#include "skill.h"

class GameRule : public PassiveSkill{
    Q_OBJECT

public:
    GameRule();

    virtual bool triggerable(const ServerPlayer *target) const;
    virtual int getPriority(ServerPlayer *target) const;

    virtual void getTriggerEvents(QList<Room::TriggerEvent> &events) const;
    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const;

private:
    void nextPhase(Room *room, ServerPlayer *target) const;
    void onPhaseChange(ServerPlayer *player) const;
};

#endif // GAMERULE_H
