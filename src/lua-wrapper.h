#ifndef LUAWRAPPER_H
#define LUAWRAPPER_H

#include "skill.h"

typedef int LuaFunction;

class LuaTriggerSkill: public TriggerSkill{
    Q_OBJECT

public:
    LuaTriggerSkill(const char *name, Frequency frequency);
    void addEvent(TriggerEvent event);
    void setViewAsSkill(ViewAsSkill *view_as_skill);

    virtual bool triggerable(ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_trigger;
    LuaFunction can_trigger;
};

class LuaViewAsSkill: public ViewAsSkill{
    Q_OBJECT

public:
    LuaViewAsSkill(const char *name);

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const;

    void pushSelf(lua_State *L) const;

    LuaFunction view_filter;
    LuaFunction view_as;

    LuaFunction enabled_at_play;
    LuaFunction enabled_at_response;

protected:
    virtual bool isEnabledAtPlay() const;
    virtual bool isEnabledAtResponse() const;
};

class LuaSkillCard: public SkillCard{
    Q_OBJECT

public:
    LuaSkillCard(const char *name);
    LuaSkillCard *clone() const;
    void setTargetFixed(bool target_fixed);
    void setWillThrow(bool will_throw);

    // member functions that do not expose to Lua interpreter
    static LuaSkillCard *Parse(const QString &str);
    void pushSelf(lua_State *L) const;

    virtual QString toString() const;    

    // these functions are defined at swig/luaskills.i
    virtual bool isAvailable() const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    // the lua callbacks
    LuaFunction available;
    LuaFunction filter;    
    LuaFunction feasible;
    LuaFunction on_use;
    LuaFunction on_effect;
};

#endif // LUAWRAPPER_H
