#include "formation.h"
#include "general.h"
#include "standard.h"
#include "standard-equips.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"


HuyuanCard::HuyuanCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool HuyuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void HuyuanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caohong = effect.from;
    Room *room = caohong->getRoom();
    room->moveCardTo(this, caohong, effect.to, Player::PlaceEquip,
                     CardMoveReason(CardMoveReason::S_REASON_PUT, caohong->objectName(), "huyuan", QString()));

    const Card *card = Sanguosha->getCard(subcards.first());

    LogMessage log;
    log.type = "$ZhijianEquip";
    log.from = effect.to;
    log.card_str = QString::number(card->getEffectiveId());
    room->sendLog(log);

    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getAllPlayers()) {
        if (effect.to->distanceTo(p) == 1 && caohong->canDiscard(p, "hej"))
            targets << p;
    }
    if (!targets.isEmpty()) {
        ServerPlayer *to_dismantle = room->askForPlayerChosen(caohong, targets, "huyuan", "@huyuan-discard:" + effect.to->objectName());
        int card_id = room->askForCardChosen(caohong, to_dismantle, "hej", "huyuan", false, Card::MethodDiscard);
        room->throwCard(Sanguosha->getCard(card_id), to_dismantle, caohong);
    }
}

class HuyuanViewAsSkill: public OneCardViewAsSkill {
public:
    HuyuanViewAsSkill(): OneCardViewAsSkill("huyuan") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@huyuan";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("EquipCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        HuyuanCard *first = new HuyuanCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Huyuan: public PhaseChangeSkill {
public:
    Huyuan(): PhaseChangeSkill("huyuan") {
        view_as_skill = new HuyuanViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if (target->getPhase() == Player::Finish && !target->isNude())
            room->askForUseCard(target, "@@huyuan", "@huyuan-equip", -1, Card::MethodNone);
        return false;
    }
}; 

class Ziliang: public TriggerSkill {
public:
    Ziliang(): TriggerSkill("ziliang") {
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> dengais = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *dengai, dengais) {
            if (!player->isAlive()) break;
            if (dengai->getPile("field").isEmpty()) continue;
            if (!room->askForSkillInvoke(dengai, objectName(), data)) continue;
            room->obtainCard(player, room->askForAG(dengai, dengai->getPile("field"), false, objectName()));
        }
        return false;
    }
}; 

class Qianhuan: public TriggerSkill {
public:
    Qianhuan(): TriggerSkill("qianhuan") {
        events << Damaged << TargetConfirming;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damaged) {
            ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
            if (yuji && room->askForSkillInvoke(player, objectName(), "choice:" + yuji->objectName())) {
                room->broadcastSkillInvoke(objectName());
                if (yuji != player) {
                    room->notifySkillInvoked(yuji, objectName());
                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = player;
                    log.to << yuji;
                    log.arg = objectName();
                    room->sendLog(log);
                }

                int id = room->drawCard();
                Card::Suit suit = Sanguosha->getCard(id)->getSuit();
                bool duplicate = false;
                foreach (int card_id, yuji->getPile("sorcery")) {
                    if (Sanguosha->getCard(card_id)->getSuit() == suit) {
                        duplicate = true;
                        break;
                    }
                }
                yuji->addToPile("sorcery", id);
                if (duplicate) {
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                    room->throwCard(Sanguosha->getCard(id), reason, NULL);
                }
            }
        } else if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.to.length() != 1) return false;
            ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
            if (!yuji || yuji->getPile("sorcery").isEmpty()) return false;
            if (room->askForSkillInvoke(yuji, objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                room->notifySkillInvoked(yuji, objectName());
                if (yuji == player || room->askForChoice(player, objectName(), "accept+reject") == "accept") {
                    QList<int> ids = yuji->getPile("sorcery");
                    int id = -1;
                    if (ids.length() > 1) {
                        room->fillAG(ids, yuji);
                        id = room->askForAG(yuji, ids, false, objectName());
                        room->clearAG(yuji);
                    } else {
                        id = ids.first();
                    }
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                    room->throwCard(Sanguosha->getCard(id), reason, NULL);

                    LogMessage log;
                    if (use.from) {
                        log.type = "$CancelTarget";
                        log.from = use.from;
                    } else {
                        log.type = "$CancelTargetNoUser";
                    }
                    log.to = use.to;
                    log.arg = use.card->objectName();
                    room->sendLog(log);


                    use.to.clear();
                    data = QVariant::fromValue(use);
                } else {
                    LogMessage log;
                    log.type = "#ZhibaReject";
                    log.from = player;
                    log.to << yuji;
                    log.arg = objectName();
                    room->sendLog(log);
                }
            }
        }
        return false;
    }
};


class Shengxi: public TriggerSkill {
public:
    Shengxi(): TriggerSkill("shengxi") {
        events << DamageDone << EventPhaseEnd;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseEnd) {
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Play) {
                if (!player->hasFlag("ShengxiDamageInPlayPhase") && player->askForSkillInvoke(objectName()))
                    player->drawCards(2);
            }
            if (player->hasFlag("ShengxiDamageInPlayPhase"))
                player->setFlags("-ShengxiDamageInPlayPhase");
        } else if (triggerEvent == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from->getPhase() == Player::Play && !damage.from->hasFlag("ShengxiDamageInPlayPhase"))
                damage.from->setFlags("ShengxiDamageInPlayPhase");
        }
        return false;
    }
};

class Shoucheng: public TriggerSkill {
public:
    Shoucheng(): TriggerSkill("shoucheng") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from && move.from->getPhase() == Player::NotActive
            && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard) {
            if (room->askForSkillInvoke(player, objectName(), data)) {
                if (move.from == player || room->askForChoice(player, objectName(), "accept+reject") == "accept") {
                    room->broadcastSkillInvoke(objectName());
                    ServerPlayer *from = (ServerPlayer *)move.from;
                    from->drawCards(1);
                } else {
                    LogMessage log;
                    log.type = "#ZhibaReject";
                    log.from = (ServerPlayer *)move.from;
                    log.to << player;
                    log.arg = objectName();
                    room->sendLog(log);
                }
            }
        }
        return false;
    }
};
 

FormationPackage::FormationPackage()
    : Package("formation")
{
    General *heg_dengai = new General(this, "heg_dengai", "wei"); // WEI 015 G
    heg_dengai->addSkill("tuntian");
    heg_dengai->addSkill(new Ziliang); 

    General *jiangwanfeiyi = new General(this, "jiangwanfeiyi", "shu", 3); // SHU 018 
    //ToDo: Add skin for jiangwanfeiyi @@Yan Guam
    jiangwanfeiyi->addSkill(new Shengxi);
    jiangwanfeiyi->addSkill(new Shoucheng);

    General *heg_yuji = new General(this, "heg_yuji", "qun", 3); // QUN 011 G
    heg_yuji->addSkill(new Qianhuan);
}

ADD_PACKAGE(Formation)