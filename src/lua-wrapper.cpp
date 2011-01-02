#include "lua-wrapper.h"

LuaTriggerSkill::LuaTriggerSkill(const char *name, Frequency frequency)
    :TriggerSkill(name), on_trigger(0), can_trigger(0)
{
    this->frequency = frequency;
}

void LuaTriggerSkill::addEvent(TriggerEvent event){
    events << event;
}

LuaSkillCard::LuaSkillCard(const char *name)
    :SkillCard(), filter(0), available(0), feasible(0)
{

}

void LuaSkillCard::setTargetFixed(bool target_fixed){
    this->target_fixed = target_fixed;
}

void LuaSkillCard::setWillThrow(bool will_throw){
    this->will_throw = will_throw;
}

//QString LuaSkillCard::toString() const{
//    return QString();
//}

//bool LuaSkillCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{

//}

//#include "engine.h"

//bool LuaSkillCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
//    if(filter == 0)
//        return SkillCard::targetFilter(targets, to_select);
//}

//bool LuaSkillCard::isAvailable(){
//    if(available == 0)
//        return SkillCard::isAvailable();

//    lua_State *L = Sanguosha->getLuaState();


//}
