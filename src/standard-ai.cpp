#include "standard-ai.h"
#include "standard.h"
#include "engine.h"
#include "yitianpackage.h"

CaocaoAI::CaocaoAI(ServerPlayer *player)
    :SmartAI(player)
{

}

bool CaocaoAI::askForSkillInvoke(const QString &skill_name, const QVariant &data) const{
    if(skill_name == "jianxiong"){
        CardStar card = data.value<CardStar>();
        return !Shit::HasShit(card);
    }else
        return SmartAI::askForSkillInvoke(skill_name, data);
}

SimayiAI::SimayiAI(ServerPlayer *player)
    :SmartAI(player)
{

}

bool SimayiAI::askForSkillInvoke(const QString &skill_name, const QVariant &data) const{
    if(skill_name == "fankui")
        return true;
    else
        return SmartAI::askForSkillInvoke(skill_name, data);
}

XiahoudunAI::XiahoudunAI(ServerPlayer *player)
    :SmartAI(player)
{

}

bool XiahoudunAI::askForSkillInvoke(const QString &skill_name, const QVariant &data) const{
    PlayerStar from = data.value<PlayerStar>();
    if(skill_name == "ganglie")
        return ! isFriend(from);
    else
        return SmartAI::askForSkillInvoke(skill_name, data);
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

