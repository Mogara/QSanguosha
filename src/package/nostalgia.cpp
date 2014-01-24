#include "standard.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "nostalgia.h"
#include "settings.h"


// old stantard generals

class NosJizhi: public TriggerSkill {
public:
    NosJizhi(): TriggerSkill("nosjizhi") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *yueying, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isNDTrick() && !use.card->isVirtualCard() && room->askForSkillInvoke(yueying, objectName())) {
            room->broadcastSkillInvoke("jizhi");
            yueying->drawCards(1);
        }

        return false;
    }
};

class NosQicai: public TargetModSkill {
public:
    NosQicai(): TargetModSkill("nosqicai") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

// old wind generals

NostalStandardPackage::NostalStandardPackage()
    : Package("nostal_standard")
{
    General *huangyueying = new General(this, "nos_huangyueying", "shu", 3, false, true, true);
    huangyueying->addSkill(new NosJizhi);
    huangyueying->addSkill(new NosQicai);
}

ADD_PACKAGE(NostalStandard)


