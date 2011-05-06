%{

#include "ai.h"
#include "joypackage.h"

%}

class AI: public QObject{
public:
    AI(ServerPlayer *player);

    enum Relation { Friend, Enemy, Neutrality };
    Relation relationTo(const ServerPlayer *other) const;
    bool isFriend(const ServerPlayer *other) const;
    bool isEnemy(const ServerPlayer *other) const;

    QList<ServerPlayer *> getEnemies() const;
    QList<ServerPlayer *> getFriends() const;

    virtual void activate(CardUseStruct &card_use) = 0;
    virtual Card::Suit askForSuit() = 0;
    virtual QString askForKingdom() = 0;
    virtual bool askForSkillInvoke(const char *skill_name, const QVariant &data) = 0;
    virtual QString askForChoice(const char *skill_name, const char *choices) = 0;
    virtual QList<int> askForDiscard(const char *reason, int discard_num, bool optional, bool include_equip) = 0;
    virtual const Card *askForNullification(const char *trick_name, ServerPlayer *from, ServerPlayer *to)  = 0;
    virtual int askForCardChosen(ServerPlayer *who, const char *flags, const char *reason)  = 0;
    virtual const Card *askForCard(const char *pattern, const char *prompt)  = 0;
    virtual QString askForUseCard(const char *pattern, const char *prompt)  = 0;
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason) = 0;
    virtual const Card *askForCardShow(ServerPlayer *requestor) = 0;
    virtual const Card *askForPindian(ServerPlayer *requestor, const char *reason) = 0;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const char *reason) = 0;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) = 0;
};

class TrustAI: public AI{
public:
    TrustAI(ServerPlayer *player);

    virtual void activate(CardUseStruct &card_use) ;
    virtual Card::Suit askForSuit() ;
    virtual QString askForKingdom() ;
    virtual bool askForSkillInvoke(const char *skill_name, const QVariant &data) ;
    virtual QString askForChoice(const char *skill_name, const char *choices);
    virtual QList<int> askForDiscard(const char *reason, int discard_num, bool optional, bool include_equip) ;
    virtual const Card *askForNullification(const char *trick_name, ServerPlayer *from, ServerPlayer *to) ;
    virtual int askForCardChosen(ServerPlayer *who, const char *flags, const char *reason) ;
    virtual const Card *askForCard(const char *pattern, const char *prompt) ;
    virtual QString askForUseCard(const char *pattern, const char *prompt) ;
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason);
    virtual const Card *askForCardShow(ServerPlayer *requestor) ;
    virtual const Card *askForPindian(ServerPlayer *requestor, const char *reason);
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const char *reason) ;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) ;

    virtual bool useCard(const Card *card);
};

class LuaAI: public TrustAI{
public:
    LuaAI(ServerPlayer *player);

    virtual const Card *askForCardShow(ServerPlayer *requestor);
    virtual bool askForSkillInvoke(const char *skill_name, const QVariant &data);
    virtual void activate(CardUseStruct &card_use);
    virtual QList<int> askForDiscard(const char *reason, int discard_num, bool optional, bool include_equip) ;
	virtual QString askForChoice(const char *skill_name, const char *choices);
    virtual int askForCardChosen(ServerPlayer *who, const char *flags, const char *reason);
	virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const char *reason) ;
	virtual const Card *askForCard(const char *pattern, const char *prompt);
	virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason);
	
    LuaFunction callback;
};

// for some AI use
class Shit:public BasicCard{
public:
    Shit(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onMove(const CardMoveStruct &move) const;

    static bool HasShit(const Card *card);
};

%{

bool LuaAI::askForSkillInvoke(const QString &skill_name, const QVariant &data) {
    if(callback == 0)
        return TrustAI::askForSkillInvoke(skill_name, data);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, skill_name.toAscii());

    SWIG_NewPointerObj(L, &data, SWIGTYPE_p_QVariant, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }else{
        bool invoke = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return invoke;
    }

    return false;
}

void LuaAI::activate(CardUseStruct &card_use) {
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    SWIG_NewPointerObj(L, &card_use, SWIGTYPE_p_CardUseStruct, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);

        TrustAI::activate(card_use);
    }
}

AI *Room::cloneAI(ServerPlayer *player){
    if(L == NULL)
        return new TrustAI(player);

    if(!Config.EnableAI)
        return new TrustAI(player);

    lua_getglobal(L, "CloneAI");

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 1, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        output(error_msg);
    }else{
        void *ai_ptr;
        int result = SWIG_ConvertPtr(L, -1, &ai_ptr, SWIGTYPE_p_AI, 0);
        lua_pop(L, 1);
        if(SWIG_IsOK(result)){
            AI *ai = static_cast<AI *>(ai_ptr);
            return ai;
        }
    }

    return new TrustAI(player);
}

ServerPlayer *LuaAI::askForYiji(const QList<int> &cards, int &card_id){
    if(callback == 0)
        return TrustAI::askForYiji(cards, card_id);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

	lua_createtable(L, cards.length(), 0);
	int i;
	for(i=0; i<cards.length(); i++){
		int elem = cards.at(i);
		lua_pushnumber(L, elem);
		lua_rawseti(L, -2, i+1);
	}

    int error = lua_pcall(L, 2, 2, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
        return NULL;
    }

    void *player_ptr;
    int result = SWIG_ConvertPtr(L, -2, &player_ptr, SWIGTYPE_p_ServerPlayer, 0);
    int number = lua_tonumber(L, -1);
    lua_pop(L, 2);

    if(SWIG_IsOK(result)){
        card_id = number;
        return static_cast<ServerPlayer *>(player_ptr);
    }

    return NULL;
}

void LuaAI::filterEvent(TriggerEvent event, ServerPlayer *player, const QVariant &data){
    if(callback == 0)
        return;

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushinteger(L, event);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    SWIG_NewPointerObj(L, &data, SWIGTYPE_p_QVariant, 0);

    int error = lua_pcall(L, 4, 0, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

int LuaAI::askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    SWIG_NewPointerObj(L, who, SWIGTYPE_p_ServerPlayer, 0);

    lua_pushstring(L, flags.toAscii());

    lua_pushstring(L, reason.toAscii());

    int error = lua_pcall(L, 4, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);

        return TrustAI::askForCardChosen(who, flags, reason);
    }

    if(lua_isnumber(L, -1)){
        int result = lua_tointeger(L, -1);
        lua_pop(L, 1);
        return result;
    }

    room->output(QString("The result of function %1 should be an integer!").arg(__func__));
    lua_pop(L, 1);
    return TrustAI::askForCardChosen(who, flags, reason);
}

ServerPlayer *LuaAI::askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    SWIG_NewPointerObj(L, &targets, SWIGTYPE_p_QListT_ServerPlayer_p_t, 0);

    lua_pushstring(L, reason.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);

        return TrustAI::askForPlayerChosen(targets, reason);
    }

    void *player_ptr;
    int result = SWIG_ConvertPtr(L, -1, &player_ptr, SWIGTYPE_p_ServerPlayer, 0);
    lua_pop(L, 1);
    if(SWIG_IsOK(result)){
        return static_cast<ServerPlayer *>(player_ptr);
    }else{
        return TrustAI::askForPlayerChosen(targets, reason);
    }
}

/************************************************************************/
/*  Lua's askForNullification is not not enabled yet!                   */
/************************************************************************/

/*
int LuaAI::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, trick_name.toAscii());

    SWIG_NewPointerObj(L, from, SWIGTYPE_p_ServerPlayer, 0);

    SWIG_NewPointerObj(L, to, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 4, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);

        return TrustAI::askForNullification(trick_name, from, to);
    }

    if(lua_isnumber(L, -1)){
        int card_id = lua_tointeger(L, -1);
        lua_pop(L, 1);
        return card_id;
    }

    lua_pop(L, 1);

    return TrustAI::askForNullification(trick_name, from, to);
}

*/

%}
