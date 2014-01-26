#include "mountainpackage.h"

#include "general.h"
#include "settings.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "ai.h"
#include "jsonutils.h"

#include <QCommandLinkButton>

class Zaoxian: public PhaseChangeSkill {
public:
    Zaoxian(): PhaseChangeSkill("zaoxian") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Start
               && target->getMark("zaoxian") == 0
               && target->getPile("field").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *dengai) const{
        Room *room = dengai->getRoom();
        room->notifySkillInvoked(dengai, objectName());

        LogMessage log;
        log.type = "#ZaoxianWake";
        log.from = dengai;
        log.arg = QString::number(dengai->getPile("field").length());
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$ZaoxianAnimate", 4000);

        room->addPlayerMark(dengai, "zaoxian");
        if (room->changeMaxHpForAwakenSkill(dengai))
            room->acquireSkill(dengai, "jixi");

        return false;
    }
};

class Jiang: public TriggerSkill {
public:
    Jiang(): TriggerSkill("jiang") {
        events << TargetConfirmed;
        frequency = Frequent;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *sunce, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from == sunce || use.to.contains(sunce)) {
            if (use.card->isKindOf("Duel") || (use.card->isKindOf("Slash") && use.card->isRed())) {
                if (sunce->askForSkillInvoke(objectName(), data)) {
                    int index = 1;
                    if (use.from != sunce)
                        index = 2;
                    if (!sunce->hasInnateSkill(objectName()) && sunce->hasSkill("mouduan"))
                        index += 2;
                    room->broadcastSkillInvoke(objectName(), index);
                    sunce->drawCards(1);
                }
            }
        }

        return false;
    }
};

class Hunzi: public PhaseChangeSkill {
public:
    Hunzi(): PhaseChangeSkill("hunzi") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getMark("hunzi") == 0
               && target->getPhase() == Player::Start
               && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();
        room->notifySkillInvoked(sunce, objectName());

        LogMessage log;
        log.type = "#HunziWake";
        log.from = sunce;
        log.arg = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$HunziAnimate", 5000);

        room->addPlayerMark(sunce, "hunzi");
        if (room->changeMaxHpForAwakenSkill(sunce))
            room->handleAcquireDetachSkills(sunce, "yingzi|yinghun");
        return false;
    }
};

MountainPackage::MountainPackage()
    : Package("mountain")
{

    General *sunce = new General(this, "sunce$", "wu"); // WU 010
    sunce->addSkill(new Jiang);
    sunce->addSkill(new Hunzi);


}

ADD_PACKAGE(Mountain)

