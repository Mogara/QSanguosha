#ifndef GAMERULE_H
#define GAMERULE_H

#include "skill.h"

class GameRule : public PassiveSkill
{
public:
    GameRule();

    virtual bool triggerable(const ServerPlayer *target) const;
    virtual int getPriority(ServerPlayer *target) const;
    virtual void onPhaseChange(ServerPlayer *target) const;

private:
    void nextPhase(Room *room, ServerPlayer *target) const;
};

#endif // GAMERULE_H
