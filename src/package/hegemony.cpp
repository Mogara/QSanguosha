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

class Huoshui: public TriggerSkill {
public:
    Huoshui(): TriggerSkill("huoshui") {
        events << EventPhaseStart << Death
            << EventLoseSkill << EventAcquireSkill
            << HpChanged << MaxHpChanged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 5;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (!TriggerSkill::triggerable(player) 
                || (player->getPhase() != Player::RoundStart || player->getPhase() != Player::NotActive)) return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player || !player->hasSkill(objectName())) return false;
        } else if (triggerEvent == EventLoseSkill) {
            if (data.toString() != objectName() || player->getPhase() == Player::NotActive) return false;
        } else if (triggerEvent == EventAcquireSkill) {
            if (data.toString() != objectName() || !player->hasSkill(objectName()) || player->getPhase() == Player::NotActive)
                return false;
        } else if (triggerEvent == MaxHpChanged || triggerEvent == HpChanged) {
            if (!room->getCurrent() || !room->getCurrent()->hasSkill(objectName())) return false;
        }

        if (player->getPhase() == Player::RoundStart || triggerEvent == EventAcquireSkill)
            room->broadcastSkillInvoke(objectName(), 1);
        else if (player->getPhase() == Player::NotActive || triggerEvent == EventLoseSkill)
            room->broadcastSkillInvoke(objectName(), 2);

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

        return false;
    }
};

QingchengCard::QingchengCard() {
    handling_method = Card::MethodDiscard;
}

void QingchengCard::onUse(Room *room, const CardUseStruct &use) const{
    CardUseStruct card_use = use;
    ServerPlayer *player = card_use.from, *to = card_use.to.first();

    LogMessage log;
    log.from = player;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString();
    room->sendLog(log);

    QStringList skill_list;
    foreach (const Skill *skill, to->getVisibleSkillList()) {
        if (!skill_list.contains(skill->objectName()) && !skill->isAttachedLordSkill()) {
            skill_list << skill->objectName();
        }
    }
    QString skill_qc;
    if (!skill_list.isEmpty()) {
        QVariant data_for_ai = QVariant::fromValue((PlayerStar)to);
        skill_qc = room->askForChoice(player, "qingcheng", skill_list.join("+"), data_for_ai);
    }

    if (!skill_qc.isEmpty()) {
        LogMessage log;
        log.type = "$QingchengNullify";
        log.from = player;
        log.to << to;
        log.arg = skill_qc;
        room->sendLog(log);

        QStringList Qingchenglist = to->tag["Qingcheng"].toStringList();
        Qingchenglist << skill_qc;
        to->tag["Qingcheng"] = QVariant::fromValue(Qingchenglist);
        room->addPlayerMark(to, "Qingcheng" + skill_qc);

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);

        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, player, data);
    card_use = data.value<CardUseStruct>();

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName(), QString(), card_use.card->getSkillName(), QString());
    room->moveCardTo(this, player, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, player, data);
    thread->trigger(CardFinished, room, player, data);
}

bool QingchengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

class QingchengViewAsSkill: public OneCardViewAsSkill {
public:
    QingchengViewAsSkill(): OneCardViewAsSkill("qingcheng") {
        filter_pattern = "EquipCard!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        QingchengCard *first = new QingchengCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Qingcheng: public TriggerSkill {
public:
    Qingcheng(): TriggerSkill("qingcheng") {
        events << EventPhaseStart;
        view_as_skill = new QingchengViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 6;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() == Player::RoundStart) {
            QStringList Qingchenglist = player->tag["Qingcheng"].toStringList();
            if (Qingchenglist.isEmpty()) return false;
            foreach (QString skill_name, Qingchenglist) {
                room->setPlayerMark(player, "Qingcheng" + skill_name, 0);
                if (player->hasSkill(skill_name)) {
                    LogMessage log;
                    log.type = "$QingchengReset";
                    log.from = player;
                    log.arg = skill_name;
                    room->sendLog(log);
                }
            }
            player->tag.remove("Qingcheng");
            foreach (ServerPlayer *p, room->getAllPlayers())
                room->filterCards(p, p->getCards("he"), true);

            Json::Value args;
            args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
            room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
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

    General *zoushi = new General(this, "zoushi", "qun", 3, false); // QUN 018
    zoushi->addSkill(new Huoshui);
    zoushi->addSkill(new Qingcheng);

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
    addMetaObject<QingchengCard>();
}

ADD_PACKAGE(Hegemony)

