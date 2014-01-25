#include "hegemony.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"
#include "standard.h"

class Xiaoguo: public TriggerSkill {
public:
    Xiaoguo(): TriggerSkill("xiaoguo") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() != Player::Finish)
            return false;
        ServerPlayer *yuejin = room->findPlayerBySkillName(objectName());
        if (!yuejin || yuejin == player)
            return false;
        if (yuejin->canDiscard(yuejin, "h") && room->askForCard(yuejin, ".Basic", "@xiaoguo", QVariant(), objectName())) {
            room->broadcastSkillInvoke(objectName(), 1);
            if (!room->askForCard(player, ".Equip", "@xiaoguo-discard", QVariant())) {
                room->broadcastSkillInvoke(objectName(), 2);
                room->damage(DamageStruct("xiaoguo", yuejin, player));
            } else {
                room->broadcastSkillInvoke(objectName(), 3);
                if (yuejin->isAlive())
                    yuejin->drawCards(1);
            }
        }
        return false;
    }
};

class Shushen: public TriggerSkill {
public:
    Shushen(): TriggerSkill("shushen") {
        events << HpRecover;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover_struct = data.value<RecoverStruct>();
        int recover = recover_struct.recover;
        for (int i = 0; i < recover; i++) {
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "shushen-invoke", true, true);
            if (target) {
                room->broadcastSkillInvoke(objectName(), target->getGeneralName().contains("liubei") ? 2 : 1);
                target->drawCards(1);
            } else {
                break;
            }
        }
        return false;
    }
};

class Shenzhi: public PhaseChangeSkill {
public:
    Shenzhi(): PhaseChangeSkill("shenzhi") {
    }

    virtual bool onPhaseChange(ServerPlayer *ganfuren) const{
        Room *room = ganfuren->getRoom();
        if (ganfuren->getPhase() != Player::Start || ganfuren->isKongcheng())
            return false;
        if (room->askForSkillInvoke(ganfuren, objectName())) {
            // As the cost, if one of her handcards cannot be throwed, the skill is unable to invoke
            foreach (const Card *card, ganfuren->getHandcards()) {
                if (ganfuren->isJilei(card))
                    return false;
            }
            //==================================
            int handcard_num = ganfuren->getHandcardNum();
            room->broadcastSkillInvoke(objectName());
            ganfuren->throwAllHandCards();
            if (handcard_num >= ganfuren->getHp()) {
                RecoverStruct recover;
                recover.who = ganfuren;
                room->recover(ganfuren, recover);
            }
        }
        return false;
    }
};

DuoshiCard::DuoshiCard() {
    mute = true;
}

bool DuoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

bool DuoshiCard::targetsFeasible(const QList<const Player *> &, const Player *) const{
    return true;
}

void DuoshiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (!use.to.contains(use.from))
        use.to << use.from;
    use.from->getRoom()->broadcastSkillInvoke("duoshi", qMin(2, use.to.length()));
    SkillCard::onUse(room, use);
}

void DuoshiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(2);
    room->askForDiscard(effect.to, "duoshi", 2, 2, false, true);
}

class Duoshi: public OneCardViewAsSkill {
public:
    Duoshi(): OneCardViewAsSkill("duoshi") {
        filter_pattern = ".|red|.|hand!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("DuoshiCard") < 4;
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        DuoshiCard *await = new DuoshiCard;
        await->addSubcard(originalcard->getId());
        return await;
    }
};

FenxunCard::FenxunCard() {
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FenxunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->tag["FenxunTarget"] = QVariant::fromValue(effect.to);
    room->setFixedDistance(effect.from, effect.to, 1);
}

class FenxunViewAsSkill: public OneCardViewAsSkill {
public:
    FenxunViewAsSkill(): OneCardViewAsSkill("fenxun") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("FenxunCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        FenxunCard *first = new FenxunCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Fenxun: public TriggerSkill {
public:
    Fenxun(): TriggerSkill("fenxun") {
        events << EventPhaseChanging << Death;
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->tag["FenxunTarget"].value<PlayerStar>() != NULL;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *dingfeng, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != dingfeng)
                return false;
        }
        ServerPlayer *target = dingfeng->tag["FenxunTarget"].value<PlayerStar>();

        if (target) {
            room->setFixedDistance(dingfeng, target, -1);
            dingfeng->tag.remove("FenxunTarget");
        }
        return false;
    }
};


HegemonyPackage::HegemonyPackage()
    : Package("hegemony")
{
    General *yuejin = new General(this, "heg_yuejin", "wei", 4, true, true); // WEI 016
    yuejin->addSkill(new Xiaoguo);

    General *ganfuren = new General(this, "ganfuren", "shu", 3, false); // SHU 016
    ganfuren->addSkill(new Shushen);
    ganfuren->addSkill(new Shenzhi);

    General *heg_luxun = new General(this, "heg_luxun", "wu", 3); // WU 007 G
    heg_luxun->addSkill("qianxun");
    heg_luxun->addSkill(new Duoshi);

    General *dingfeng = new General(this, "dingfeng", "wu"); // WU 016
    dingfeng->addSkill(new Skill("duanbing", Skill::Compulsory));
    dingfeng->addSkill(new Fenxun);

    General *heg_caopi = new General(this, "heg_caopi$", "wei", 3, true, true); // WEI 014 G
    heg_caopi->addSkill("fangzhu");
    heg_caopi->addSkill("xingshang");

    General *heg_zhenji = new General(this, "heg_zhenji", "wei", 3, false, true); // WEI 007 G
    heg_zhenji->addSkill("qingguo");
    heg_zhenji->addSkill("luoshen");

    General *heg_zhugeliang = new General(this, "heg_zhugeliang", "shu", 3, true, true); // SHU 004 G
    heg_zhugeliang->addSkill("guanxing");
    heg_zhugeliang->addSkill("kongcheng");

    General *heg_huangyueying = new General(this, "heg_huangyueying", "shu", 3, false, true); // SHU 007 G
    heg_huangyueying->addSkill("nosjizhi");
    heg_huangyueying->addSkill("nosqicai");

    General *heg_zhouyu = new General(this, "heg_zhouyu", "wu", 3, true, true); // WU 005 G
    heg_zhouyu->addSkill("yingzi");
    heg_zhouyu->addSkill("fanjian");

    General *heg_xiaoqiao = new General(this, "heg_xiaoqiao", "wu", 3, false, true); // WU 011 G
    heg_xiaoqiao->addSkill("tianxiang");
    heg_xiaoqiao->addSkill("hongyan");

    General *heg_lvbu = new General(this, "heg_lvbu", "qun", 4, true, true); // QUN 002 G
    heg_lvbu->addSkill("wushuang");

    General *heg_diaochan = new General(this, "heg_diaochan", "qun", 3, false, true); // QUN 003 G
    heg_diaochan->addSkill("lijian");
    heg_diaochan->addSkill("biyue");

    addMetaObject<DuoshiCard>();
    addMetaObject<FenxunCard>();
}

ADD_PACKAGE(Hegemony)

