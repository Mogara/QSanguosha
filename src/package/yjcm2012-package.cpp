#include "yjcm2012-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class Zhiyu: public MasochismSkill{
public:
    Zhiyu():MasochismSkill("zhiyu"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if(target->askForSkillInvoke(objectName(), QVariant::fromValue(damage))){
            target->drawCards(1);

            Room *room = target->getRoom();
            room->showAllCards(target);

            QList<const Card *> cards = target->getHandcards();
            Card::Color color = cards.first()->getColor();
            bool same_color = true;
            foreach(const Card *card, cards){
                if(card->getColor() != color){
                    same_color = false;
                    break;
                }
            }

            if(same_color && damage.from){
                DamageStruct zhiyu_damage;
                zhiyu_damage.from = target;
                zhiyu_damage.to = damage.from;

                room->damage(zhiyu_damage);
            }
        }
    }
};

class Fuji: public TriggerSkill{
public:
    Fuji():TriggerSkill("fuji"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

        if(player->distanceTo(damage.to) == 1 && damage.card && damage.card->inherits("Slash")){
            player->getRoom()->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.to << damage.to;
            room->sendLog(log);
            room->loseMaxHp(damage.to);
            return true;
        }
        return false;
    }
};

YJCM2012Package::YJCM2012Package():Package("YJCM2012"){
    General *xunyou = new General(this, "xunyou", "wei", 3);
    xunyou->addSkill(new Zhiyu);

/*
    General *wangyi = new General(this, "wangyi", "wei", 3, false);


    General *caozhang = new General(this, "caozhang", "wei");
*/
    General *madai = new General(this, "madai", "shu");
    madai->addSkill(new Fuji);
/*
    General *liaohua = new General(this, "liaohua", "shu");

    General *guanxingzhangbao = new General(this, "guanxingzhangbao", "shu");

    General *chengpu = new General(this, "chengpu", "wu");

    General *bulianshi = new General(this, "bulianshi", "wu", 3, false);

    General *handang = new General(this, "handang", "qun");

    General *liubiao = new General(this, "liubiao", "qun", 3);
*/
}

ADD_PACKAGE(YJCM2012)
