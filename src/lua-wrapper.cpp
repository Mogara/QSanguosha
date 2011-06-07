#include "lua-wrapper.h"

LuaTriggerSkill::LuaTriggerSkill(const char *name, Frequency frequency)
    :TriggerSkill(name), on_trigger(0), can_trigger(0)
{
    this->frequency = frequency;
}

void LuaTriggerSkill::addEvent(TriggerEvent event){
    events << event;
}

void LuaTriggerSkill::setViewAsSkill(ViewAsSkill *view_as_skill){
    this->view_as_skill = view_as_skill;
}

LuaProhibitSkill::LuaProhibitSkill(const char *name)
    :ProhibitSkill(name), is_prohibited(0)
{

}

LuaViewAsSkill::LuaViewAsSkill(const char *name)
    :ViewAsSkill(name), view_filter(0), view_as(0),
    enabled_at_play(0), enabled_at_response(0)
{

}

LuaFilterSkill::LuaFilterSkill(const char *name)
    :FilterSkill(name), view_filter(0), view_as(0)
{

}

static QHash<QString, const LuaSkillCard *> LuaSkillCards;

LuaSkillCard::LuaSkillCard(const char *name)
    :SkillCard(), available(0), filter(0), feasible(0), on_use(0), on_effect(0)
{
    if(name){
        LuaSkillCards.insert(name, this);
        setObjectName(name);
    }
}

LuaSkillCard *LuaSkillCard::clone() const{
    LuaSkillCard *new_card = new LuaSkillCard(NULL);

    new_card->setObjectName(objectName());

    new_card->target_fixed = target_fixed;
    new_card->will_throw = will_throw;

    new_card->filter = filter;
    new_card->available = available;
    new_card->feasible = feasible;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;

    return new_card;
}

void LuaSkillCard::setTargetFixed(bool target_fixed){
    this->target_fixed = target_fixed;
}

void LuaSkillCard::setWillThrow(bool will_throw){
    this->will_throw = will_throw;;
}

LuaSkillCard *LuaSkillCard::Parse(const QString &str){
    QString name = str;
    name.remove(QChar('#'));

    const LuaSkillCard *c = LuaSkillCards.value(name, NULL);

    return c ? c->clone() : NULL;
}

QString LuaSkillCard::toString() const{
    return "#" + objectName();
}

