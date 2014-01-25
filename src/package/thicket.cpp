#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "standard.h"



class Jiuchi: public OneCardViewAsSkill {
public:
    Jiuchi(): OneCardViewAsSkill("jiuchi") {
        filter_pattern = ".|spade|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Analeptic *analeptic = new Analeptic(originalCard->getSuit(), originalCard->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(originalCard->getId());
        return analeptic;
    }
};

class Roulin: public TriggerSkill {
public:
    Roulin(): TriggerSkill("roulin") {
        events << TargetConfirmed;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && (target->hasSkill(objectName()) || target->isFemale());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && player == use.from) {
            QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();
            int index = 0;
            bool play_effect = false;
            if (TriggerSkill::triggerable(use.from)) {
                foreach (ServerPlayer *p, use.to) {
                    if (p->isFemale()) {
                        play_effect = true;
                        if (jink_list.at(index).toInt() == 1)
                            jink_list.replace(index, QVariant(2));
                    }
                    index++;
                }
                use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
                if (play_effect) {
                    LogMessage log;
                    log.from = use.from;
                    log.arg = objectName();
                    log.type = "#TriggerSkill";
                    room->sendLog(log);
                    room->notifySkillInvoked(use.from, objectName());

                    room->broadcastSkillInvoke(objectName(), 1);
                }
            } else if (use.from->isFemale()) {
                foreach (ServerPlayer *p, use.to) {
                    if (p->hasSkill(objectName())) {
                        play_effect = true;
                        if (jink_list.at(index).toInt() == 1)
                            jink_list.replace(index, QVariant(2));
                    }
                    index++;
                }
                use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
                if (play_effect) {
                    foreach (ServerPlayer *p, use.to) {
                        if (p->hasSkill(objectName())) {
                            LogMessage log;
                            log.from = p;
                            log.arg = objectName();
                            log.type = "#TriggerSkill";
                            room->sendLog(log);
                            room->notifySkillInvoked(p, objectName());
                        }
                    }
                    room->broadcastSkillInvoke(objectName(), 2);
                }
            }
        }

        return false;
    }
};

class Benghuai: public PhaseChangeSkill {
public:
    Benghuai(): PhaseChangeSkill("benghuai") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const{
        bool trigger_this = false;
        Room *room = dongzhuo->getRoom();

        if (dongzhuo->getPhase() == Player::Finish) {
            QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
            foreach (ServerPlayer *player, players) {
                if (dongzhuo->getHp() > player->getHp()) {
                    trigger_this = true;
                    break;
                }
            }
        }

        if (trigger_this) {
            LogMessage log;
            log.from = dongzhuo;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);
            room->notifySkillInvoked(dongzhuo, objectName());

            QString result = room->askForChoice(dongzhuo, "benghuai", "hp+maxhp");
            int index = (result == "hp") ? 2 : 1;
            room->broadcastSkillInvoke(objectName(), index);
            if (result == "hp")
                room->loseHp(dongzhuo);
            else
                room->loseMaxHp(dongzhuo);
        }

        return false;
    }
};

ThicketPackage::ThicketPackage()
    : Package("thicket")
{
    General *dongzhuo = new General(this, "dongzhuo$", "qun", 8); // QUN 006
    dongzhuo->addSkill(new Jiuchi);
    dongzhuo->addSkill(new Roulin);
    dongzhuo->addSkill(new Benghuai);
}

ADD_PACKAGE(Thicket)

