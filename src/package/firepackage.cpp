#include "firepackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "engine.h"

class Lianhuan: public OneCardViewAsSkill {
public:
    Lianhuan(): OneCardViewAsSkill("lianhuan") {
        filter_pattern = ".|club|.|hand";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IronChain *chain = new IronChain(originalCard->getSuit(), originalCard->getNumber());
        chain->addSubcard(originalCard);
        chain->setSkillName(objectName());
        chain->setShowSkill(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill {
public:
    Niepan(): TriggerSkill("niepan") {
        events << AskForPeaches;
        frequency = Limited;
        limit_mark = "@nirvana";
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer *ask_who /* = NULL */){
        if (TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0){
            DyingStruct dying_data = data.value<DyingStruct>();
            if (dying_data.who != target)
                return false;
            return true;
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *pangtong, QVariant &data) const{
        if (pangtong->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$NiepanAnimate");
            room->removePlayerMark(pangtong, "@nirvana");
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *pangtong, QVariant &data) const{
        pangtong->throwAllHandCardsAndEquips();
        QList<const Card *> tricks = pangtong->getJudgingArea();
        foreach (const Card *trick, tricks) {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, pangtong->objectName());
            room->throwCard(trick, reason, NULL);
        }

        RecoverStruct recover;
        recover.recover = qMin(3, pangtong->getMaxHp()) - pangtong->getHp();
        room->recover(pangtong, recover);

        pangtong->drawCards(3);

        if (pangtong->isChained())
            room->setPlayerProperty(pangtong, "chained", false);

        if (!pangtong->faceUp())
            pangtong->turnOver();

        return false;
    }
};

class Huoji: public OneCardViewAsSkill {
public:
    Huoji(): OneCardViewAsSkill("huoji") {
        filter_pattern = ".|red|.|hand";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FireAttack *fire_attack = new FireAttack(originalCard->getSuit(), originalCard->getNumber());
        fire_attack->addSubcard(originalCard->getId());
        fire_attack->setSkillName(objectName());
        fire_attack->setShowSkill(objectName());
        return fire_attack;
    }
};

class Bazhen: public TriggerSkill {
public:
    Bazhen(): TriggerSkill("bazhen") {
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (!TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who))
            return false;

        QString pattern = data.toStringList().first();
        if (pattern != "jink")
            return false;

        if (!player->tag["Qinggang"].toStringList().isEmpty() || player->getMark("Armor_Nullified") > 0
            || player->getMark("Equips_Nullified_to_Yourself") > 0)
            return false;

        if (player->getArmor() == NULL && player->isAlive())
            return true;
        
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (!player->hasShownSkill(this)){
            if (player->askForSkillInvoke("bazhen", "showgeneral")){
                if (player->ownSkill(objectName()) && !player->hasShownSkill(this))
                    player->showGeneral(player->inHeadSkills(objectName()));
            }
        }

        if (player->hasArmorEffect("bazhen")){
            return player->askForSkillInvoke("EightDiagram");
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *wolong, QVariant &data) const{
        //此处更改是因为“八阵”是“视为”装备八卦阵，真正发动的技能是八卦阵，而不是八阵。
        JudgeStruct judge;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = "EightDiagram";
        judge.who = wolong;

        room->setEmotion(wolong, "armor/eight_diagram");
        room->judge(judge);

        if (judge.isGood()) {
            Jink *jink = new Jink(Card::NoSuit, 0);
            jink->setSkillName("EightDiagram");
            room->broadcastSkillInvoke(objectName());
            room->provide(jink);
            return true;
        }


        return false;
    }
};

class Kanpo: public OneCardViewAsSkill {
public:
    Kanpo(): OneCardViewAsSkill("kanpo") {
        filter_pattern = ".|black|.|hand";
        response_pattern = "nullification";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());
        ncard->setShowSkill(objectName());
        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        foreach (const Card *card, player->getHandcards()) {
            if (card->isBlack()) return true;
        }
        return false;
    }
};

FirePackage::FirePackage()
    : Package("fire")
{

    General *pangtong = new General(this, "pangtong", "shu", 3); // SHU 010
    pangtong->addSkill(new Lianhuan);
    pangtong->addSkill(new Niepan);

    General *wolong = new General(this, "wolong", "shu", 3); // SHU 011
    wolong->addCompanion("huangyueying");
    wolong->addCompanion("pangtong");
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Kanpo);
    wolong->addSkill(new Bazhen);
}

ADD_PACKAGE(Fire)

