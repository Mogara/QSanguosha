#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "test.h"

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
    :Package("test")
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

#include "standard-generals.cpp"
class SuperZhiheng: public Zhiheng{
public:
    SuperZhiheng(){
        setObjectName("super_zhiheng");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("ZhihengCard") < (player->getLostHp() + 1);
    }
};

class SuperGuanxing: public Guanxing{
public:
    SuperGuanxing():Guanxing(){
        setObjectName("super_guanxing");
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect("guanxing");

            room->doGuanxing(zhuge, room->getNCards(5, false), false);
        }

        return false;
    }
};

class SuperJushou: public PhaseChangeSkill{
public:
    SuperJushou():PhaseChangeSkill("super_jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(5);
                target->turnOver();

                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    General *zhiba_sunquan = new General(this, "zhibasunquan$", "wu", 4, true, true);
    zhiba_sunquan->addSkill(new SuperZhiheng);
    zhiba_sunquan->addSkill("jiuyuan");

    General *wuxing_zhuge = new General(this, "wuxingzhuge", "shu", 3, true, true);
    wuxing_zhuge->addSkill(new SuperGuanxing);
    wuxing_zhuge->addSkill("kongcheng");
    wuxing_zhuge->addSkill("#kongcheng-effect");

    General *dunkeng_caoren = new General(this, "dunkengcaoren", "wei", 4, true, true);
    dunkeng_caoren->addSkill(new SuperJushou);

    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);

    new General(this, "anjiang", "god", 4,true, true, true);

    addMetaObject<CheatCard>();
    addMetaObject<ChangeCard>();
}

ADD_PACKAGE(Danchuang)
ADD_PACKAGE(Test)
