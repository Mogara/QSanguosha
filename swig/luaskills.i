
class LuaTriggerSkill: public TriggerSkill{
public:
    LuaTriggerSkill(const char *name, Frequency frequency);
    void addEvent(TriggerEvent event);
	
	virtual bool triggerable(ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_trigger;
    LuaFunction can_trigger;
};


class LuaSkillCard: public SkillCard{
public:
    LuaSkillCard(const char *name);
    void setTargetFixed(bool target_fixed);
    void setWillThrow(bool will_throw);

    LuaFunction filter;
    LuaFunction available;
    LuaFunction feasible;
};

%{

#include "lua-wrapper.h"

bool LuaTriggerSkill::triggerable(ServerPlayer *target) const{
	if(can_trigger == 0)
		return TriggerSkill::triggerable(target);
	
	Room *room = target->getRoom();
	lua_State *L = room->getLuaState();
	
	// the callback function
	lua_rawgeti(L, LUA_REGISTRYINDEX, can_trigger);	
	
	LuaTriggerSkill *self = const_cast<LuaTriggerSkill *>(this);
	SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaTriggerSkill, 0);
	
	SWIG_NewPointerObj(L, target, SWIGTYPE_p_ServerPlayer, 0);

	int error = lua_pcall(L, 2, 1, 0);
	if(error)	
		room->output(lua_tostring(L, -1));
	else	
		return lua_toboolean(L, -1);
		
	return false;
}

bool LuaTriggerSkill::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
	if(on_trigger == 0)
		return false;
		
	Room *room = player->getRoom();
	lua_State *L = room->getLuaState();
	
	int e = static_cast<int>(event);
	
	// the callback function
	lua_rawgeti(L, LUA_REGISTRYINDEX, on_trigger);	
	
	LuaTriggerSkill *self = const_cast<LuaTriggerSkill *>(this);
	SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaTriggerSkill, 0);

	// the first argument: event
	lua_pushinteger(L, e);	
	
	// the second argument: player
	SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

	// the last event: data
	SWIG_NewPointerObj(L, &data, SWIGTYPE_p_QVariant, 0);
	
	int error = lua_pcall(L, 4, 1, 0);
	if(error)	
		room->output(lua_tostring(L, -1));
	else	
		return lua_toboolean(L, -1);
	
	return false;
}

%}
