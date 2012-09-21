#include "assassinspackage.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class MouKui: public TriggerSkill{
public:
    MouKui(): TriggerSkill("moukui"){
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

class TianMing: public TriggerSkill{
public:
    TianMing(): TriggerSkill("tianming"){
        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

MizhaoCard::MizhaoCard(){
}

bool MizhaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return(to_select != Self);
}

void MizhaoCard::onEffect(const CardEffectStruct &effect) const{

}

class MiZhao: public ViewAsSkill{
public:
    MiZhao():ViewAsSkill("mizhao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return(!player->isKongcheng() && !player->hasUsed("MizhaoCard"));
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;
        return NULL;
    }
};

class JieYuan: public TriggerSkill{
public:
    JieYuan(): TriggerSkill("jieyuan"){
        events << DamageCaused << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(triggerEvent == DamageCaused){
            if(damage.to->getHp() >= player->getHp())
                if(player->askForSkillInvoke(objectName()) && room->askForCard(player, ".black", "@JieYuanIncrease", QVariant(), CardDiscarded)){
                    damage.damage ++;
                    data = QVariant::fromValue(damage);
                }
        }else if(triggerEvent == DamageInflicted){
            if(damage.from->getHp() >= player->getHp())
                if(player->askForSkillInvoke(objectName()) && room->askForCard(player, ".red", "@JieYuanDecrease", QVariant(), CardDiscarded)){
                    damage.damage --;
                    data = QVariant::fromValue(damage);
                }
        }

        return false;
    }
};

class FenXin: public TriggerSkill{
public:
    FenXin(): TriggerSkill("fenxin"){
        events << Death;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

AssassinsPackage::AssassinsPackage():Package("assassins"){
    General *wujiangjia = new General(this, "wujiangjia", "qun", 4);
    wujiangjia->addSkill(new MouKui);

    General *wujiangyi = new General(this, "wujiangyi", "qun", 3);
    wujiangyi->addSkill(new TianMing);
    wujiangyi->addSkill(new MiZhao);

    General *wujiangbing = new General(this, "wujiangbing", "qun", 3);
    wujiangbing->addSkill(new JieYuan);
    wujiangbing->addSkill(new FenXin);

    addMetaObject<MizhaoCard>();
}

ADD_PACKAGE(Assassins)
