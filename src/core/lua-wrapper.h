#ifndef _LUA_WRAPPER_H
#define _LUA_WRAPPER_H

#include "skill.h"
#include "standard.h"

typedef int LuaFunction;

class LuaTriggerSkill: public TriggerSkill {
    Q_OBJECT

public:
    LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark);
    inline void addEvent(TriggerEvent triggerEvent) { events << triggerEvent; }
    inline void setViewAsSkill(ViewAsSkill *view_as_skill) { this->view_as_skill = view_as_skill; }
    inline void setGlobal(bool global) { this->global = global; }

    virtual int getPriority() const;
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_trigger;
    LuaFunction can_trigger;
    int priority;
};

class LuaProhibitSkill: public ProhibitSkill {
    Q_OBJECT

public:
    LuaProhibitSkill(const char *name);

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;

    LuaFunction is_prohibited;
};

class LuaViewAsSkill: public ViewAsSkill {
    Q_OBJECT

public:
    LuaViewAsSkill(const char *name, const char *response_pattern = "");

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;

    void pushSelf(lua_State *L) const;

    LuaFunction view_filter;
    LuaFunction view_as;

    LuaFunction enabled_at_play;
    LuaFunction enabled_at_response;
    LuaFunction enabled_at_nullification;

    virtual bool isEnabledAtPlay(const Player *player) const;
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    virtual bool isEnabledAtNullification(const ServerPlayer *player) const;
};

class LuaFilterSkill: public FilterSkill {
    Q_OBJECT

public:
    LuaFilterSkill(const char *name);

    virtual bool viewFilter(const Card *to_select) const;
    virtual const Card *viewAs(const Card *originalCard) const;

    LuaFunction view_filter;
    LuaFunction view_as;
};

class LuaDistanceSkill: public DistanceSkill {
    Q_OBJECT

public:
    LuaDistanceSkill(const char *name);

    virtual int getCorrect(const Player *from, const Player *to) const;

    LuaFunction correct_func;
};

class LuaMaxCardsSkill: public MaxCardsSkill {
    Q_OBJECT

public:
    LuaMaxCardsSkill(const char *name);

    virtual int getExtra(const Player *target) const;
    virtual int getFixed(const Player *target) const;

    LuaFunction extra_func;
    LuaFunction fixed_func;
};

class LuaTargetModSkill: public TargetModSkill {
    Q_OBJECT

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
    Q_OBJECT

public:
    LuaSkillCard(const char *name, const char *skillName);
    LuaSkillCard *clone() const;
    inline void setTargetFixed(bool target_fixed) { this->target_fixed = target_fixed; }
    inline void setWillThrow(bool will_throw) { this->will_throw = will_throw; }
    inline void setCanRecast(bool can_recast) { this->can_recast = can_recast; }
    inline void setHandlingMethod(Card::HandlingMethod handling_method) { this->handling_method = handling_method; }

    // member functions that do not expose to Lua interpreter
    static LuaSkillCard *Parse(const QString &str);
    void pushSelf(lua_State *L) const;

    virtual QString toString(bool hidden = false) const;

    // these functions are defined at swig/luaskills.i
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
    LuaFunction on_validate;
    LuaFunction on_validate_in_response;
};

class LuaBasicCard: public BasicCard {
    Q_OBJECT

public:
    Q_INVOKABLE LuaBasicCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype);
    LuaBasicCard *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    inline void setTargetFixed(bool target_fixed) { this->target_fixed = target_fixed; }
    inline void setCanRecast(bool can_recast) { this->can_recast = can_recast; }

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    inline virtual QString getClassName() const{ return QString(class_name); }
    inline virtual QString getSubtype() const{ return QString(subtype); }
    inline virtual bool isKindOf(const char *cardType) const{
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction available;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;

private:
    QString class_name, subtype;
};

class LuaTrickCard: public TrickCard {
    Q_OBJECT

public:
    enum SubClass { TypeNormal, TypeSingleTargetTrick, TypeDelayedTrick, TypeAOE, TypeGlobalEffect };

    Q_INVOKABLE LuaTrickCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype);
    LuaTrickCard *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    inline void setTargetFixed(bool target_fixed) { this->target_fixed = target_fixed; }
    inline void setCanRecast(bool can_recast) { this->can_recast = can_recast; }

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onNullified(ServerPlayer *target) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    inline virtual QString getClassName() const{ return class_name; }
    inline void setSubtype(const char *subtype) { this->subtype = subtype; }
    inline virtual QString getSubtype() const{ return subtype; }
    inline void setSubClass(SubClass subclass) { this->subclass = subclass; }
    inline SubClass getSubClass() const{ return subclass; }
    inline virtual bool isKindOf(const char *cardType) const{
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else {
            if (Card::isKindOf(cardType)) return true;
            switch (subclass) {
            case TypeSingleTargetTrick: return strcmp(cardType, "SingleTargetTrick") == 0; break;
            case TypeDelayedTrick: return strcmp(cardType, "DelayedTrick") == 0; break;
            case TypeAOE: return strcmp(cardType, "AOE") == 0; break;
            case TypeGlobalEffect: return strcmp(cardType, "GlobalEffect") == 0; break;
            case TypeNormal:
            default:
                    return false;
                    break;
            }
        }
    }

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction available;
    LuaFunction is_cancelable;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
    LuaFunction on_nullified;

private:
    SubClass subclass;
    QString class_name, subtype;
};

class LuaWeapon: public Weapon {
    Q_OBJECT

public:
    Q_INVOKABLE LuaWeapon(Card::Suit suit, int number, int range, const char *obj_name, const char *class_name);
    LuaWeapon *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    inline virtual QString getClassName() const{ return class_name; }
    inline virtual bool isKindOf(const char *cardType) const{
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;

private:
    QString class_name;
};

class LuaArmor: public Armor {
    Q_OBJECT

public:
    Q_INVOKABLE LuaArmor(Card::Suit suit, int number, const char *obj_name, const char *class_name);
    LuaArmor *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    inline virtual QString getClassName() const{ return class_name; }
    inline virtual bool isKindOf(const char *cardType) const{
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;

private:
    QString class_name;
};


#endif
