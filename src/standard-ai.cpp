#include "standard-ai.h"
#include "standard.h"

CaocaoAI::CaocaoAI(ServerPlayer *player)
    :SmartAI(player)
{

}

bool CaocaoAI::askForSkillInvoke(const QString &skill_name) const{
    return true;
}

SimayiAI::SimayiAI(ServerPlayer *player)
    :SmartAI(player)
{

}

bool SimayiAI::askForSkillInvoke(const QString &skill_name) const{
    return true;
}

XiahoudunAI::XiahoudunAI(ServerPlayer *player)
    :SmartAI(player)
{

}

bool XiahoudunAI::askForSkillInvoke(const QString &skill_name) const{
    return true;
}

LumengAI::LumengAI(ServerPlayer *player)
    :SmartAI(player, true)
{

}

void StandardPackage::addAIs(){
    addMetaObject<CaocaoAI>();
    addMetaObject<SimayiAI>();
    addMetaObject<XiahoudunAI>();
    addMetaObject<LumengAI>();
}

