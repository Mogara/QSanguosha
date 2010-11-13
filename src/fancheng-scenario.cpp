#include "fancheng-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"

class Guagu: public TriggerSkill{
public:
    Guagu():TriggerSkill("guagu"){
        events << Damage;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isLord()){
            int x = damage.damage;
            Room *room = player->getRoom();
            room->recover(damage.to, x*2);
            player->drawCards(x);
        }

        return false;
    }
};

DujiangCard::DujiangCard(){
    target_fixed = true;
}

void DujiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = source;
    log.arg = "dujiang";
    room->sendLog(log);

    // transfiguration
    room->setPlayerProperty(source, "general", "shenlumeng");
    room->getThread()->removeTriggerSkill("keji");
    room->acquireSkill(source, "shelie", false);

    source->setMaxHP(3);
    room->broadcastProperty(source, "maxhp");
    room->broadcastProperty(source, "hp");
}

class DujiangViewAsSkill: public ViewAsSkill{
public:
    DujiangViewAsSkill():ViewAsSkill("dujiang"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@dujiang-card";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2 && to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return false;

        DujiangCard *card = new DujiangCard;
        card->addSubcards(cards);

        return card;
    }
};

class Dujiang: public PhaseChangeSkill{
public:
    Dujiang():PhaseChangeSkill("dujiang"){
        view_as_skill = new DujiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!PhaseChangeSkill::triggerable(target))
            return false;

        return target->getGeneralName() != "shenlumeng";
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            int equip_num = 0;
            if(target->getWeapon())
                equip_num ++;

            if(target->getArmor())
                equip_num ++;

            if(target->getDefensiveHorse())
                equip_num ++;

            if(target->getOffensiveHorse())
                equip_num ++;

            if(equip_num < 2)
                return false;

            Room *room = target->getRoom();
            room->askForUseCard(target, "@dujiang-card", "@@dujiang");
        }

        return false;
    }
};

class FanchengRule: public ScenarioRule{
public:
    FanchengRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    room->installEquip(player, "chitu");
                    room->installEquip(player, "blade");

                    ServerPlayer *pangde = room->findPlayer("pangde");
                    room->installEquip(pangde, "eight_diagram");

                    ServerPlayer *huatuo = room->findPlayer("huatuo");
                    room->acquireSkill(huatuo, "guagu");

                    ServerPlayer *lumeng = room->findPlayer("lumeng");
                    room->acquireSkill(lumeng, "dujiang");
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

FanchengScenario::FanchengScenario()
    :Scenario("fancheng")
{
    lord = "guanyu";
    loyalists << "huatuo";
    rebels << "caoren" << "pangde" << "xuhuang";
    renegades << "lumeng";

    rule = new FanchengRule(this);

    skills << new Guagu << new Dujiang;

    addMetaObject<DujiangCard>();

    t["fancheng"] = tr("fancheng");
    t["guagu"] = tr("guagu");
    t["dujiang"] = tr("dujiang");

    t["@@dujiang"] = tr("@@dujiang");
}

void FanchengScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(Fancheng);
