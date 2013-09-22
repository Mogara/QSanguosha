class LuaTriggerSkill: public TriggerSkill {
public:
    LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark);
    void addEvent(TriggerEvent event);
    void setViewAsSkill(ViewAsSkill *view_as_skill);
    void setGlobal(bool global);
    
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_trigger;
    LuaFunction can_trigger;
    int priority;
};

class GameStartSkill: public TriggerSkill {
public:
    GameStartSkill(const QString &name);

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual void onGameStart(ServerPlayer *player) const = 0;
};

class ProhibitSkill: public Skill {
public:
    ProhibitSkill(const QString &name);

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const = 0;
};

class DistanceSkill: public Skill {
public:
    DistanceSkill(const QString &name);

    virtual int getCorrect(const Player *from, const Player *to) const = 0;
};

class MaxCardsSkill: public Skill {
public:
    MaxCardsSkill(const QString &name);

    virtual int getExtra(const Player *target) const;
    virtual int getFixed(const Player *target) const;
};

class TargetModSkill: public Skill {
public:
    enum ModType {
        Residue,
        DistanceLimit,
        ExtraTarget
    };

    TargetModSkill(const QString &name);
    virtual QString getPattern() const;

    virtual int getResidueNum(const Player *from, const Card *card) const;
    virtual int getDistanceLimit(const Player *from, const Card *card) const;
    virtual int getExtraTargetNum(const Player *from, const Card *card) const;

protected:
    QString pattern;
};

class LuaProhibitSkill: public ProhibitSkill {
public:
    LuaProhibitSkill(const char *name);

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;

    LuaFunction is_prohibited;
};

class ViewAsSkill: public Skill {
public:
    ViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const = 0;
    virtual const Card *viewAs(const QList<const Card *> &cards) const = 0;

    virtual bool isEnabledAtPlay(const Player *player) const;
    virtual bool isEnabledAtResponse(const Player *player, const char *pattern) const;
};

class LuaViewAsSkill: public ViewAsSkill {
public:
    LuaViewAsSkill(const char *name, const char *response_pattern = "");

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;

    LuaFunction view_filter;
    LuaFunction view_as;

    LuaFunction enabled_at_play;
    LuaFunction enabled_at_response;
    LuaFunction enabled_at_nullification;
};

class OneCardViewAsSkill: public ViewAsSkill {
public:
    OneCardViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;

    virtual bool viewFilter(const Card *to_select) const = 0;
    virtual const Card *viewAs(const Card *originalCard) const = 0;
};

class FilterSkill: public OneCardViewAsSkill {
public:
    FilterSkill(const QString &name);
};

class LuaFilterSkill: public FilterSkill {
public:
    LuaFilterSkill(const char *name);

    virtual bool viewFilter(const Card *to_select) const;
    virtual const Card *viewAs(const Card *originalCard) const;

    LuaFunction view_filter;
    LuaFunction view_as;
};

class LuaDistanceSkill: public DistanceSkill {
public:
    LuaDistanceSkill(const char *name);
    virtual int getCorrect(const Player *from, const Player *to) const;

    LuaFunction correct_func;
};

class LuaMaxCardsSkill: public MaxCardsSkill {
public:
    LuaMaxCardsSkill(const char *name);
    virtual int getExtra(const Player *target) const;
    virtual int getFixed(const Player *target) const;

    LuaFunction extra_func;
    LuaFunction fixed_func;
};

class LuaTargetModSkill: public TargetModSkill {
public:
    LuaTargetModSkill(const char *name, const char *pattern);

    virtual int getResidueNum(const Player *from, const Card *card) const;
    virtual int getDistanceLimit(const Player *from, const Card *card) const;
    virtual int getExtraTargetNum(const Player *from, const Card *card) const;

    LuaFunction residue_func;
    LuaFunction distance_limit_func;
    LuaFunction extra_target_func;
};

class LuaSkillCard: public SkillCard {
public:
    LuaSkillCard(const char *name, const char *skillName);
    void setTargetFixed(bool target_fixed);
    void setWillThrow(bool will_throw);
    void setCanRecast(bool can_recast);
    void setHandlingMethod(Card::HandlingMethod handling_method);
    LuaSkillCard *clone() const;

    LuaFunction filter;    
    LuaFunction feasible;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
    LuaFunction on_validate;
    LuaFunction on_validate_in_response;
};

class LuaBasicCard: public BasicCard {
public:
    LuaBasicCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype);
    LuaBasicCard *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    void setTargetFixed(bool target_fixed);
    void setCanRecast(bool can_recast);

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    virtual QString getClassName() const;
    virtual QString getSubtype() const;
    virtual bool isKindOf(const char *cardType) const;

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction available;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
};

class LuaTrickCard: public TrickCard {
public:
    enum SubClass { TypeNormal, TypeSingleTargetTrick, TypeDelayedTrick, TypeAOE, TypeGlobalEffect };
    
    LuaTrickCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype);
    LuaTrickCard *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    void setTargetFixed(bool target_fixed);
    void setCanRecast(bool can_recast);

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onNullified(ServerPlayer *target) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    virtual QString getClassName() const;
    void setSubClass(SubClass subclass);
    SubClass getSubClass() const;
    virtual QString getSubtype() const;
    virtual bool isKindOf(const char *cardType) const;

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction available;
    LuaFunction is_cancelable;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
    LuaFunction on_nullified;
};

class LuaWeapon: public Weapon {
public:
    LuaWeapon(Card::Suit suit, int number, int range, const char *obj_name, const char *class_name);
    LuaWeapon *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual QString getClassName();
    virtual bool isKindOf(const char *cardType);

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;
};

class LuaArmor: public Armor {
public:
    LuaArmor(Card::Suit suit, int number, const char *obj_name, const char *class_name);
    LuaArmor *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual QString getClassName();
    virtual bool isKindOf(const char *cardType);

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;
};

%{

#include "lua-wrapper.h"
#include "clientplayer.h"

bool LuaTriggerSkill::triggerable(const ServerPlayer *target) const{
    if (can_trigger == 0)
        return TriggerSkill::triggerable(target);
    
    Room *room = target->getRoom();
    lua_State *L = room->getLuaState();
    
    // the callback function
    lua_rawgeti(L, LUA_REGISTRYINDEX, can_trigger);    
    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaTriggerSkill, 0);    
    SWIG_NewPointerObj(L, target, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

bool LuaTriggerSkill::trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
    if (on_trigger == 0)
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
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

#include <QMessageBox>

static void Error(lua_State *L) {
    const char *error_string = lua_tostring(L, -1);
    lua_pop(L, 1);
    QMessageBox::warning(NULL, "Lua script error!", error_string);
}

bool LuaProhibitSkill::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others) const{
    if (is_prohibited == 0)
        return false;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, is_prohibited);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaProhibitSkill, 0);
    SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, to, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);

    lua_createtable(L, others.length(), 0);
    for(int i = 0; i < others.length(); i++){
        const Player *player = others[i];
        SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    int error = lua_pcall(L, 5, 1, 0);
    if (error) {
        Error(L);
        return false;
    }

    bool result = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return result;
}

int LuaDistanceSkill::getCorrect(const Player *from, const Player *to) const{
    if (correct_func == 0)
        return 0;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, correct_func);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaDistanceSkill, 0);
    SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, to, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return 0;
    }

    int correct = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return correct;
}

int LuaMaxCardsSkill::getExtra(const Player *target) const{
    if (extra_func == 0)
        return 0;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, extra_func);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaMaxCardsSkill, 0);
    SWIG_NewPointerObj(L, target, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return 0;
    }

    int extra = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return extra;
}

int LuaMaxCardsSkill::getFixed(const Player *target) const{
    if (fixed_func == 0)
        return 0;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, fixed_func);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaMaxCardsSkill, 0);
    SWIG_NewPointerObj(L, target, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return 0;
    }

    int extra = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return extra;
}

int LuaTargetModSkill::getResidueNum(const Player *from, const Card *card) const{
    if (residue_func == 0)
        return 0;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, residue_func);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaTargetModSkill, 0);
    SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return 0;
    }

    int residue = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return residue;
}

int LuaTargetModSkill::getDistanceLimit(const Player *from, const Card *card) const{
    if (distance_limit_func == 0)
        return 0;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, distance_limit_func);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaTargetModSkill, 0);
    SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return 0;
    }

    int distance_limit = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return distance_limit;
}

int LuaTargetModSkill::getExtraTargetNum(const Player *from, const Card *card) const{
    if (extra_target_func == 0)
        return 0;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, extra_target_func);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaTargetModSkill, 0);
    SWIG_NewPointerObj(L, from, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return 0;
    }

    int extra_target_func = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return extra_target_func;
}

bool LuaFilterSkill::viewFilter(const Card *to_select) const{
    if (view_filter == 0)
        return false;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, view_filter);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaFilterSkill, 0);
    SWIG_NewPointerObj(L, to_select, SWIGTYPE_p_Card, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return false;
    }

    bool result = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return result;
}

const Card *LuaFilterSkill::viewAs(const Card *originalCard) const{
    if (view_as == 0)
        return false;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, view_as);

    SWIG_NewPointerObj(L, this, SWIGTYPE_p_LuaFilterSkill, 0);
    SWIG_NewPointerObj(L, originalCard, SWIGTYPE_p_Card, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return NULL;
    }
    
    void *card_ptr;
    int result = SWIG_ConvertPtr(L, -1, &card_ptr, SWIGTYPE_p_Card, 0);
    lua_pop(L, 1);
    if (SWIG_IsOK(result)) {
        const Card *card = static_cast<const Card *>(card_ptr);
        return card;
    } else
        return NULL;
}

// ----------------------

void LuaViewAsSkill::pushSelf(lua_State *L) const{
    LuaViewAsSkill *self = const_cast<LuaViewAsSkill *>(this);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaViewAsSkill, 0);
}

bool LuaViewAsSkill::viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
    if (view_filter == 0)
        return false;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, view_filter);

    pushSelf(L);

    lua_createtable(L, selected.length(), 0);
    for(int i = 0; i < selected.length(); i++){
        const Card *card = selected[i];
        SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);
        lua_rawseti(L, -2, i + 1);
    }

    const Card *card = to_select;
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

const Card *LuaViewAsSkill::viewAs(const QList<const Card *> &cards) const{
    if (view_as == 0)
        return NULL;

    lua_State *L = Sanguosha->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, view_as);

    pushSelf(L);

    lua_createtable(L, cards.length(), 0);
    for (int i = 0; i < cards.length(); i++) {
        const Card *card = cards[i];
        SWIG_NewPointerObj(L, card, SWIGTYPE_p_Card, 0);
        lua_rawseti(L, -2, i + 1);
    }

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return NULL;
    }

    void *card_ptr;
    int result = SWIG_ConvertPtr(L, -1, &card_ptr, SWIGTYPE_p_Card, 0);
    lua_pop(L, 1);
    if (SWIG_IsOK(result)) {
        const Card *card = static_cast<const Card *>(card_ptr);
        return card;
    } else
        return NULL;
}

bool LuaViewAsSkill::isEnabledAtPlay(const Player *player) const{
    if (enabled_at_play == 0)
        return ViewAsSkill::isEnabledAtPlay(player);

    lua_State *L = Sanguosha->getLuaState();

    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, enabled_at_play);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

bool LuaViewAsSkill::isEnabledAtResponse(const Player *player, const QString &pattern) const{
    if (enabled_at_response == 0)
        return ViewAsSkill::isEnabledAtResponse(player, pattern);

    lua_State *L = Sanguosha->getLuaState();

    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, enabled_at_response);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);
    
    lua_pushstring(L, pattern.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

bool LuaViewAsSkill::isEnabledAtNullification(const ServerPlayer *player) const{
    if (enabled_at_nullification == 0)
        return ViewAsSkill::isEnabledAtNullification(player);

    lua_State *L = Sanguosha->getLuaState();

    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, enabled_at_nullification);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
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
    if (filter == 0)
        return SkillCard::targetFilter(targets, to_select, self);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, filter);    

    pushSelf(L);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    SWIG_NewPointerObj(L, to_select, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 4, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

bool LuaSkillCard::targetsFeasible(const QList<const Player *> &targets, const Player *self) const{
    if (feasible == 0)
        return SkillCard::targetsFeasible(targets, self);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, feasible);    

    pushSelf(L);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

void LuaSkillCard::onUse(Room *room, const CardUseStruct &card_use) const{
    if (about_to_use == 0)
        return SkillCard::onUse(room, card_use);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, about_to_use);

    pushSelf(L);

    SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);
    SWIG_NewPointerObj(L, &card_use, SWIGTYPE_p_CardUseStruct, 0);

    int error = lua_pcall(L, 3, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

void LuaSkillCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if (on_use == 0)
        return SkillCard::use(room, source, targets);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_use);

    pushSelf(L);

    SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);
    SWIG_NewPointerObj(L, source, SWIGTYPE_p_ServerPlayer, 0);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_ServerPlayer, 0);
        lua_rawseti(L, -2, i + 1);
    }

    int error = lua_pcall(L, 4, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

void LuaSkillCard::onEffect(const CardEffectStruct &effect) const{
    if (on_effect == 0)
        return SkillCard::onEffect(effect);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_effect);

    pushSelf(L);

    SWIG_NewPointerObj(L, &effect, SWIGTYPE_p_CardEffectStruct, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = effect.to->getRoom();
        room->output(error_msg);
    }
}

const Card *LuaSkillCard::validate(CardUseStruct &cardUse) const{
    if (on_validate == 0)
        return SkillCard::validate(cardUse);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_validate);

    pushSelf(L);

    SWIG_NewPointerObj(L, &cardUse, SWIGTYPE_p_CardUseStruct, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return SkillCard::validate(cardUse);
    }

    void *card_ptr;
    int result = SWIG_ConvertPtr(L, -1, &card_ptr, SWIGTYPE_p_Card, 0);
    lua_pop(L, 1);
    if (SWIG_IsOK(result)) {
        const Card *card = static_cast<const Card *>(card_ptr);
        return card;
    } else
        return SkillCard::validate(cardUse);
}

const Card *LuaSkillCard::validateInResponse(ServerPlayer *user) const{
    if (on_validate_in_response == 0)
        return SkillCard::validateInResponse(user);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_validate_in_response);

    pushSelf(L);

    SWIG_NewPointerObj(L, user, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return SkillCard::validateInResponse(user);
    }

    void *card_ptr;
    int result = SWIG_ConvertPtr(L, -1, &card_ptr, SWIGTYPE_p_Card, 0);
    lua_pop(L, 1);
    if (SWIG_IsOK(result)) {
        const Card *card = static_cast<const Card *>(card_ptr);
        return card;
    } else
        return SkillCard::validateInResponse(user);
}

// ---------------------

void LuaBasicCard::pushSelf(lua_State *L) const{
    LuaBasicCard *self = const_cast<LuaBasicCard *>(this);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaBasicCard, 0);
}

bool LuaBasicCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *self) const{
    if (filter == 0)
        return BasicCard::targetFilter(targets, to_select, self);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, filter);    

    pushSelf(L);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    SWIG_NewPointerObj(L, to_select, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 4, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

bool LuaBasicCard::targetsFeasible(const QList<const Player *> &targets, const Player *self) const{
    if (feasible == 0)
        return BasicCard::targetsFeasible(targets, self);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, feasible);    

    pushSelf(L);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

void LuaBasicCard::onUse(Room *room, const CardUseStruct &card_use) const{
    if (about_to_use == 0)
        return BasicCard::onUse(room, card_use);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, about_to_use);

    pushSelf(L);

    SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);
    SWIG_NewPointerObj(L, &card_use, SWIGTYPE_p_CardUseStruct, 0);

    int error = lua_pcall(L, 3, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

void LuaBasicCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if (on_use == 0)
        return BasicCard::use(room, source, targets);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_use);

    pushSelf(L);

    SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);
    SWIG_NewPointerObj(L, source, SWIGTYPE_p_ServerPlayer, 0);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_ServerPlayer, 0);
        lua_rawseti(L, -2, i + 1);
    }

    int error = lua_pcall(L, 4, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

void LuaBasicCard::onEffect(const CardEffectStruct &effect) const{
    if (on_effect == 0)
        return BasicCard::onEffect(effect);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_effect);

    pushSelf(L);

    SWIG_NewPointerObj(L, &effect, SWIGTYPE_p_CardEffectStruct, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = effect.to->getRoom();
        room->output(error_msg);
    }
}

bool LuaBasicCard::isAvailable(const Player *player) const{
    if (available == 0)
        return BasicCard::isAvailable(player);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, available);    

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

// ---------------------

void LuaTrickCard::pushSelf(lua_State *L) const{
    LuaTrickCard *self = const_cast<LuaTrickCard *>(this);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaTrickCard, 0);
}

bool LuaTrickCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *self) const{
    if (filter == 0)
        return TrickCard::targetFilter(targets, to_select, self);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, filter);    

    pushSelf(L);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    SWIG_NewPointerObj(L, to_select, SWIGTYPE_p_Player, 0);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 4, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

bool LuaTrickCard::targetsFeasible(const QList<const Player *> &targets, const Player *self) const{
    if (feasible == 0)
        return TrickCard::targetsFeasible(targets, self);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, feasible);    

    pushSelf(L);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_Player, 0);
        lua_rawseti(L, -2, i + 1);
    }

    SWIG_NewPointerObj(L, self, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 3, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

void LuaTrickCard::onNullified(ServerPlayer *target) const{
    if (on_nullified == 0)
        return TrickCard::onNullified(target);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_nullified);    

    pushSelf(L);

    SWIG_NewPointerObj(L, target, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        target->getRoom()->output(error_msg);
    }
}

bool LuaTrickCard::isCancelable(const CardEffectStruct &effect) const{
    if (is_cancelable == 0)
        return TrickCard::isCancelable(effect);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, is_cancelable);

    pushSelf(L);

    SWIG_NewPointerObj(L, &effect, SWIGTYPE_p_CardEffectStruct, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

void LuaTrickCard::onUse(Room *room, const CardUseStruct &card_use) const{
    if (about_to_use == 0)
        return TrickCard::onUse(room, card_use);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, about_to_use);

    pushSelf(L);

    SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);
    SWIG_NewPointerObj(L, &card_use, SWIGTYPE_p_CardUseStruct, 0);

    int error = lua_pcall(L, 3, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

void LuaTrickCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if (on_use == 0)
        return TrickCard::use(room, source, targets);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_use);

    pushSelf(L);

    SWIG_NewPointerObj(L, room, SWIGTYPE_p_Room, 0);
    SWIG_NewPointerObj(L, source, SWIGTYPE_p_ServerPlayer, 0);

    lua_createtable(L, targets.length(), 0);
    for (int i = 0; i < targets.length(); i++) {
        SWIG_NewPointerObj(L, targets.at(i), SWIGTYPE_p_ServerPlayer, 0);
        lua_rawseti(L, -2, i + 1);
    }

    int error = lua_pcall(L, 4, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}

void LuaTrickCard::onEffect(const CardEffectStruct &effect) const{
    if (on_effect == 0)
        return TrickCard::onEffect(effect);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_effect);

    pushSelf(L);

    SWIG_NewPointerObj(L, &effect, SWIGTYPE_p_CardEffectStruct, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = effect.to->getRoom();
        room->output(error_msg);
    }
}

bool LuaTrickCard::isAvailable(const Player *player) const{
    if (available == 0)
        return TrickCard::isAvailable(player);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, available);    

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_Player, 0);

    int error = lua_pcall(L, 2, 1, 0);
    if (error) {
        Error(L);
        return false;
    } else {
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }
}

void LuaWeapon::pushSelf(lua_State *L) const{
    LuaWeapon *self = const_cast<LuaWeapon *>(this);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaWeapon, 0);
}

void LuaWeapon::onInstall(ServerPlayer *player) const{
    if (on_install == 0)
        return Weapon::onInstall(player);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_install);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = player->getRoom();
        room->output(error_msg);
    }
}

void LuaWeapon::onUninstall(ServerPlayer *player) const{
    if (on_uninstall == 0)
        return Weapon::onUninstall(player);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_uninstall);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = player->getRoom();
        room->output(error_msg);
    }
}

void LuaArmor::pushSelf(lua_State *L) const{
    LuaArmor *self = const_cast<LuaArmor *>(this);
    SWIG_NewPointerObj(L, self, SWIGTYPE_p_LuaArmor, 0);
}

void LuaArmor::onInstall(ServerPlayer *player) const{
    if (on_install == 0)
        return Armor::onInstall(player);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_install);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = player->getRoom();
        room->output(error_msg);
    }
}

void LuaArmor::onUninstall(ServerPlayer *player) const{
    if (on_uninstall == 0)
        return Armor::onUninstall(player);

    lua_State *L = Sanguosha->getLuaState();
    
    // the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, on_uninstall);

    pushSelf(L);

    SWIG_NewPointerObj(L, player, SWIGTYPE_p_ServerPlayer, 0);

    int error = lua_pcall(L, 2, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        Room *room = player->getRoom();
        room->output(error_msg);
    }
}

%}
