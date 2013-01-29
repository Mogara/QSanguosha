#include "skill.h"
#include "carditem.h"
#include "engine.h"
#include "danchuang.h"

class V5Zhenggong:public MasochismSkill{
public:
    V5Zhenggong():MasochismSkill("v5zhenggong"){
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{

    }
};

class V5Quanji: public PhaseChangeSkill{
public:
    V5Quanji():PhaseChangeSkill("v5quanji"){
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::RoundStart)
            return false;
        Room *room = player->getRoom();
        PlayerStar zh = room->findPlayerBySkillName(objectName());
        if(!zh || zh == player)
            return false;

    }
};

class V5Baijiang: public PhaseChangeSkill{
public:
    V5Baijiang():PhaseChangeSkill("v5baijiang"){
        frequency = Wake;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start)
            return false;
        Room *room = player->getRoom();
        room->playSkillEffect(objectName());
    }
};

class V5Yexin: public TriggerSkill{
public:
    V5Yexin():TriggerSkill("v5yexin"){
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        room->playSkillEffect(objectName());
        return false;
    }
};

class V5Zili: public PhaseChangeSkill{
public:
    V5Zili():PhaseChangeSkill("v5zili"){
        frequency = Wake;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start)
            return false;
        Room *room = player->getRoom();
        room->playSkillEffect(objectName());
    }
};

class V5Paiyi: public PhaseChangeSkill{
public:
    V5Paiyi():PhaseChangeSkill("v5paiyi"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Finish)
            return false;
        Room *room = player->getRoom();
        room->playSkillEffect(objectName());
    }
};

DanchuangPackage::DanchuangPackage()
    :Package("danchuang")
{
    General *v5zhonghui = new General(this, "v5zhonghui", "wei", 3);
    v5zhonghui->addSkill(new V5Zhenggong);
    v5zhonghui->addSkill(new V5Quanji);
    v5zhonghui->addSkill(new V5Baijiang);
    skills << new V5Yexin;
    v5zhonghui->addRelateSkill("v5yexin");
    skills << new V5Paiyi;
    v5zhonghui->addSkill(new V5Zili);
    v5zhonghui->addRelateSkill("v5paiyi");
}

ADD_PACKAGE(Danchuang)
