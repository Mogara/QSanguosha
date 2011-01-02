#ifndef LUAWRAPPER_H
#define LUAWRAPPER_H

#include "skill.h"

typedef int LuaFunction;

class LuaTriggerSkill: public TriggerSkill{
    Q_OBJECT

public:
    LuaTriggerSkill(const char *name, Frequency frequency);
    void addEvent(TriggerEvent event);

    virtual bool triggerable(ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_trigger;
    LuaFunction can_trigger;
};

class LuaSkillCard: public SkillCard{
    Q_OBJECT

public:
    LuaSkillCard(const char *name);
    void setTargetFixed(bool target_fixed);
    void setWillThrow(bool will_throw);

//    virtual QString toString() const;
//    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
//    virtual bool isAvailable() const;

    LuaFunction filter;
    LuaFunction available;
    LuaFunction feasible;
};

#endif // LUAWRAPPER_H
