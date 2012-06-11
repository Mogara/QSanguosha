
class LuaTriggerSkill: public TriggerSkill{
public:
	LuaTriggerSkill(const char *name, Frequency frequency);
	void addEvent(TriggerEvent event);
	void setViewAsSkill(ViewAsSkill *view_as_skill);
	
	virtual bool triggerable(const ServerPlayer *target) const;
	virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const;

	LuaFunction on_trigger;
	LuaFunction can_trigger;
	int priority;
};

class GameStartSkill: public TriggerSkill{
public:
	GameStartSkill(const QString &name);

	virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const;
	virtual void onGameStart(ServerPlayer *player) const = 0;
};

class ProhibitSkill: public Skill{
public:
	ProhibitSkill(const QString &name);

	virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const = 0;
};

class SPConvertSkill: public GameStartSkill{
public:
	SPConvertSkill(const QString &name, const QString &from, const QString &to);

	virtual bool triggerable(const ServerPlayer *target) const;
	virtual void onGameStart(ServerPlayer *player) const;
};

class DistanceSkill: public Skill{
public:
	DistanceSkill(const QString &name);

	virtual int getCorrect(const Player *from, const Player *to) const = 0;
};

class LuaProhibitSkill: public ProhibitSkill{
public:
	LuaProhibitSkill(const char *name);

	virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const;

	LuaFunction is_prohibited;
};

class ViewAsSkill:public Skill{
public:
	ViewAsSkill(const QString &name);

	virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const = 0;
	virtual const Card *viewAs(const QList<CardItem *> &cards) const = 0;

	virtual bool isEnabledAtPlay(const Player *player) const;
	virtual bool isEnabledAtResponse(const Player *player, const char *pattern) const;
};

class LuaViewAsSkill: public ViewAsSkill{
public:
	LuaViewAsSkill(const char *name);

	virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
	virtual const Card *viewAs(const QList<CardItem *> &cards) const;

	LuaFunction view_filter;
	LuaFunction view_as;

	LuaFunction enabled_at_play;
	LuaFunction enabled_at_response;
	LuaFunction enabled_at_nullification;
};

class OneCardViewAsSkill: public ViewAsSkill{
public:
	OneCardViewAsSkill(const QString &name);

	virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
	virtual const Card *viewAs(const QList<CardItem *> &cards) const;

	virtual bool viewFilter(const CardItem *to_select) const = 0;
	virtual const Card *viewAs(CardItem *card_item) const = 0;
};

class FilterSkill: public OneCardViewAsSkill{
public:
	FilterSkill(const QString &name);
};

class LuaFilterSkill: public FilterSkill{
public:
	LuaFilterSkill(const char *name);

	virtual bool viewFilter(const CardItem *to_select) const;
	virtual const Card *viewAs(CardItem *card_item) const;

	LuaFunction view_filter;
	LuaFunction view_as;
};

class LuaDistanceSkill: public DistanceSkill{
public:
	LuaDistanceSkill(const char *name);
	virtual int getCorrect(const Player *from, const Player *to) const;

	LuaFunction correct_func;
};

class LuaSkillCard: public SkillCard{
public:
	LuaSkillCard(const char *name);
	void setTargetFixed(bool target_fixed);
	void setWillThrow(bool will_throw);
	LuaSkillCard *clone() const;

	LuaFunction filter;    
	LuaFunction feasible;
	LuaFunction on_use;
	LuaFunction on_effect;
};

%{

#include "lua-wrapper.h"
#include "clientplayer.h"
#include "carditem.h"

bool LuaTriggerSkill::triggerable(const ServerPlayer *target) const{
	if(can_trigger == 0)
		return TriggerSkill::triggerable(target);
	
	Room *room = target->getRoom();
	lua_State *L = room->getLuaState();
	
	// the callback function
	lua_rawgeti(L, LUA_REGISTRYINDEX, can_trigger);	
	SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaTriggerSkill, 0);	
	SWIG_NewPointerObj(L, target, SWIGTYPE_p_ServerPlayer, 0);

	int error = lua_pcall(L, 2, 1, 0);
	if(error){
		const char *error_msg = lua_tostring(L, -1);
		lua_pop(L, 1);
		room->output(error_msg);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

bool LuaTriggerSkill::trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
	if(on_trigger == 0)
		return false;
		
	lua_State *L = room->getLuaState();
	
	int e = static_cast<int>(event);
	
	// the callback
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
	if(error){
		const char *error_msg = lua_tostring(L, -1);
		lua_pop(L, 1);
		room->output(error_msg);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

#include <QMessageBox>

static void Error(lua_State *L){
	const char *error_string = lua_tostring(L, -1);
	lua_pop(L, 1);
	QMessageBox::warning(NULL, "Lua script error!", error_string);
}

bool LuaProhibitSkill::isProhibited(const Player *from, const Player *to, const Card *card) const{
	if(is_prohibited == 0)
		return false;

	lua_State *L = Sanguosha->getLuaState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, is_prohibited);

	SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaProhibitSkill, 0);
	SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
	SWIG_NewPointerObj(L, to, SWIGTYPE_p_Player, 0);
	SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);

	int error = lua_pcall(L, 4, 1, 0);
	if(error){
		Error(L);
		return false;
	}

	bool result = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return result;
}

int LuaDistanceSkill::getCorrect(const Player *from, const Player *to) const{
	if(correct_func == 0)
		return 0;

	lua_State *L = Sanguosha->getLuaState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, correct_func);

	SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaDistanceSkill, 0);
	SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
	SWIG_NewPointerObj(L, to, SWIGTYPE_p_Player, 0);

	int error = lua_pcall(L, 3, 1, 0);
	if(error){
		Error(L);
		return 0;
	}

	int correct = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return correct;
}

bool LuaFilterSkill::viewFilter(const CardItem *to_select) const{
	if(view_filter == 0)
		return false;

	lua_State *L = Sanguosha->getLuaState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, view_filter);

	SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaFilterSkill, 0);
	SWIG_NewPointerObj(L, to_select->getCard(), SWIGTYPE_p_Card, 0);

	int error = lua_pcall(L, 2, 1, 0);
	if(error){
		Error(L);
		return false;
	}

	bool result = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return result;
}

const Card *LuaFilterSkill::viewAs(CardItem *card_item) const{
	if(view_as == 0)
		return false;

	lua_State *L = Sanguosha->getLuaState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, view_as);

	SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaFilterSkill, 0);
	SWIG_NewPointerObj(L, card_item->getCard(), SWIGTYPE_p_Card, 0);

	int error = lua_pcall(L, 2, 1, 0);
	if(error){
		Error(L);
		return NULL;
	}
	
	void *card_ptr;
	int result = SWIG_ConvertPtr(L, -1, &card_ptr, SWIGTYPE_p_Card, 0);
	lua_pop(L, 1);
	if(SWIG_IsOK(result)){
		const Card *card = static_cast<const Card *>(card_ptr);
		return card;
	}else
		return NULL;
}


// ----------------------

void LuaViewAsSkill::pushSelf(lua_State *L) const{
	LuaViewAsSkill *self = const_cast<LuaViewAsSkill *>(this);
	SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaViewAsSkill, 0);
}

bool LuaViewAsSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
	if(view_filter == 0)
		return false;

	lua_State *L = Sanguosha->getLuaState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, view_filter);

	pushSelf(L);

	lua_createtable(L, selected.length(), 0);
	int i;
	for(i=0; i<selected.length(); i++){
		const Card *card = selected.at(i)->getFilteredCard();
		SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);
		lua_rawseti(L, -2, i+1);
	}

	const Card *card = to_select->getFilteredCard();
	SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);

	int error = lua_pcall(L, 3, 1, 0);
	if(error){
		Error(L);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

const Card *LuaViewAsSkill::viewAs(const QList<CardItem *> &cards) const{
	if(view_as == 0)
		return NULL;

	lua_State *L = Sanguosha->getLuaState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, view_as);

	pushSelf(L);

	lua_createtable(L, cards.length(), 0);
	int i;
	for(i=0; i<cards.length(); i++){
		const Card *card = cards.at(i)->getFilteredCard();
		SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);
		lua_rawseti(L, -2, i+1);
	}

	int error = lua_pcall(L, 2, 1, 0);
	if(error){
		Error(L);
		return NULL;
	}

	void *card_ptr;
	int result = SWIG_ConvertPtr(L, -1, &card_ptr, SWIGTYPE_p_Card, 0);
	lua_pop(L, 1);
	if(SWIG_IsOK(result)){
		const Card *card = static_cast<const Card *>(card_ptr);
		return card;
	}else
		return NULL;
}

bool LuaViewAsSkill::isEnabledAtPlay(const Player *player) const{
	if(enabled_at_play == 0)
		return ViewAsSkill::isEnabledAtPlay(player);

	lua_State *L = Sanguosha->getLuaState();

	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, enabled_at_play);

	pushSelf(L);

	SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);

	int error = lua_pcall(L, 2, 1, 0);
	if(error){
		Error(L);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

bool LuaViewAsSkill::isEnabledAtResponse(const Player *player, const QString &pattern) const{
	if(enabled_at_response == 0)
		return ViewAsSkill::isEnabledAtResponse(player, pattern);

	lua_State *L = Sanguosha->getLuaState();

	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, enabled_at_response);

	pushSelf(L);

	SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);
	
	lua_pushstring(L, pattern.toAscii());

	int error = lua_pcall(L, 3, 1, 0);
	if(error){
		Error(L);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

bool LuaViewAsSkill::isEnabledAtNullification(const Player *player) const{
	if(enabled_at_nullification == 0)
		return false;

	lua_State *L = Sanguosha->getLuaState();

	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, enabled_at_nullification);

	pushSelf(L);

	SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);

	int error = lua_pcall(L, 2, 1, 0);
	if(error){
		Error(L);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}
// ---------------------

void LuaSkillCard::pushSelf(lua_State *L) const{
	LuaSkillCard *self = const_cast<LuaSkillCard *>(this);
	SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaSkillCard, 0);
}

bool LuaSkillCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *self) const{
	if(filter == 0)
		return SkillCard::targetFilter(targets, to_select, self);

	lua_State *L = Sanguosha->getLuaState();
	
	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, filter);	

	pushSelf(L);

	lua_createtable(L, targets.length(), 0);
	int i;
	for(i=0; i<targets.length(); i++){
		SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
		lua_rawseti(L, -2, i+1);
	}

	SWIG_NewPointerObj(L, to_select, SWIGTYPE_p_Player, 0);
	SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

	int error = lua_pcall(L, 4, 1, 0);
	if(error){
		Error(L);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

bool LuaSkillCard::targetsFeasible(const QList<const Player *> &targets, const Player *self) const{
	if(feasible == 0)
		return SkillCard::targetsFeasible(targets, self);

	lua_State *L = Sanguosha->getLuaState();
	
	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, feasible);	

	pushSelf(L);

	lua_createtable(L, targets.length(), 0);
	int i;
	for(i=0; i<targets.length(); i++){
		SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
		lua_rawseti(L, -2, i+1);
	}

	SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

	int error = lua_pcall(L, 3, 1, 0);
	if(error){
		Error(L);
		return false;
	}else{
		bool result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	}
}

void LuaSkillCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
	if(on_use == 0)
		return SkillCard::use(room, source, targets);

	lua_State *L = Sanguosha->getLuaState();
	
	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, on_use);

	pushSelf(L);

	SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);

	SWIG_NewPointerObj(L, source, SWIGTYPE_p_ServerPlayer, 0);

	lua_createtable(L, targets.length(), 0);
	int i;
	for(i=0; i<targets.length(); i++){
		SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_ServerPlayer, 0);
		lua_rawseti(L, -2, i+1);
	}

	int error = lua_pcall(L, 4, 0, 0);
	if(error){
		const char *error_msg = lua_tostring(L, -1);
		lua_pop(L, 1);
		room->output(error_msg);
	}
}

void LuaSkillCard::onEffect(const CardEffectStruct &effect) const{
	if(on_effect == 0)
		return SkillCard::onEffect(effect);

	lua_State *L = Sanguosha->getLuaState();
	
	// the callback
	lua_rawgeti(L, LUA_REGISTRYINDEX, on_effect);

	pushSelf(L);

	SWIG_NewPointerObj(L, &effect, SWIGTYPE_p_CardEffectStruct, 0);

	int error = lua_pcall(L, 2, 0, 0);
	if(error){
		const char *error_msg = lua_tostring(L, -1);
		lua_pop(L, 1);
		Room *room = effect.to->getRoom();
		room->output(error_msg);
	}
}



%}
