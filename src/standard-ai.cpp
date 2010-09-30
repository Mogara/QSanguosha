#include "standard-ai.h"
#include "standard.h"

CaocaoAI::CaocaoAI(ServerPlayer *player)
    :TrustAI(player)
{

}

bool CaocaoAI::askForSkillInvoke(const QString &skill_name) const{
    return true;
}

SimayiAI::SimayiAI(ServerPlayer *player)
    :TrustAI(player)
{

}

bool SimayiAI::askForSkillInvoke(const QString &skill_name) const{
    return true;
}

void StandardPackage::addAIs(){
    addMetaObject<CaocaoAI>();
    addMetaObject<SimayiAI>();
}

