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

    virtual int getPriority() const;
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_trigger;
    LuaFunction can_trigger;
    int priority;
};

class LuaProhibitSkill: public ProhibitSkill{
    Q_OBJECT

public:
    LuaProhibitSkill(const char *name);

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const;

    LuaFunction is_prohibited;
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
    LuaFunction enabled_at_nullification;

    virtual bool isEnabledAtPlay(const Player *player) const;
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    virtual bool isEnabledAtNullification(const Player *player) const;
};

class LuaFilterSkill: public FilterSkill{
    Q_OBJECT

public:
    LuaFilterSkill(const char *name);

    virtual bool viewFilter(const CardItem *to_select) const;
    virtual const Card *viewAs(CardItem *card_item) const;

    LuaFunction view_filter;
    LuaFunction view_as;
};

class LuaDistanceSkill: public DistanceSkill{
    Q_OBJECT

public:
    LuaDistanceSkill(const char *name);

    virtual int getCorrect(const Player *from, const Player *to) const;

    LuaFunction correct_func;
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
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction on_use;
    LuaFunction on_effect;
};

#endif // LUAWRAPPER_H
