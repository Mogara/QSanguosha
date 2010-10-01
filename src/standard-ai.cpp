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

void StandardPackage::addAIs(){
    addMetaObject<CaocaoAI>();
    addMetaObject<SimayiAI>();
}

