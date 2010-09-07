#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
}

class Guidao:public ViewAsSkill{
public:
    Guidao():ViewAsSkill("guidao"){

    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guidao";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->isBlack();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        Card *card = new GuidaoCard;
        card->addSubcard(cards.first()->getCard()->getId());

        return card;
    }
};

class Shensu: public PhaseChangeSkill{
public:
    Shensu():PhaseChangeSkill("shensu"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() != Player::Start)
            return false;

        return false;
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(!target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(3);
                target->turnOver();

                room->broadcastProperty(target, "face_up", target->faceUp() ? "true" : "false");
            }
        }

        return false;
    }
};

class Liegong: public SlashBuffSkill{
public:
    Liegong():SlashBuffSkill("liegong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huangzhong = effect.from;

        int num = effect.to->getHandcardNum();
        if(num >= huangzhong->getHp() || num <= huangzhong->getAttackRange()){
            Room *room = huangzhong->getRoom();
            if(room->askForSkillInvoke(huangzhong, "liegong")){
                SlashResultStruct result;
                result.from = huangzhong;
                result.to = effect.to;
                result.slash = effect.slash;
                result.success = true;
                result.nature = effect.nature;

                room->slashResult(result);

                return true;
            }
        }

        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
    }

    virtual void getTriggerEvents(QList<TriggerEvent> &events) const{
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, const QVariant &data) const{
        if(data.canConvert<DamageStruct>()){
            DamageStruct damage = data.value<DamageStruct>();

            if(player->distanceTo(damage.to) <= 1){
                Room *room = player->getRoom();
                room->recover(player, damage.damage);
            }
        }

        return false;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    // xiaoqiao, zhoutai and yuji is omitted
    General *xiahouyuan, *caoren, *huangzhong, *weiyan, *zhangjiao;

    xiahouyuan = new General(this, "xiahouyuan", "wei");
    xiahouyuan->addSkill(new Shensu);

    caoren = new General(this, "caoren", "wei");
    caoren->addSkill(new Jushou);

    huangzhong = new General(this, "huangzhong", "shu");
    huangzhong->addSkill(new Liegong);

    weiyan = new General(this, "weiyan", "shu");
    weiyan->addSkill(new Kuanggu);

    zhangjiao = new General(this, "zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Skill("leiji"));
    zhangjiao->addSkill(new Skill("huangtian$"));

    t["wind"] = tr("wind");

    t["xiahouyuan"] = tr("xiahouyuan");
    t["caoren"] = tr("caoren");
    t["huangzhong"] = tr("huangzhong");
    t["weiyan"] = tr("weiyan");
    t["zhangjiao"] = tr("zhangjiao");   

    // skills
    t["shensu"] = tr("shensu");
    t["jushou"] = tr("jushou");
    t["liegong"] = tr("liegong");
    t["kuanggu"] = tr("kuanggu");
    t["guidao"] = tr("guidao");
    t["leiji"] = tr("leiji");
    t["huangtian"] = tr("huangtian");
}

extern "C" {
    Q_DECL_EXPORT Package *NewWind(){
        return new WindPackage;
    }
}



