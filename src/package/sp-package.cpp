#include "sp-package.h"
#include "client.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "engine.h"
#include "maneuvering.h"
#include "wisdompackage.h"

class SPMoonSpearSkill: public WeaponSkill {
public:
    SPMoonSpearSkill(): WeaponSkill("SPMoonSpear") {
        events << CardUsed << CardResponded;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if (triggerEvent == CardUsed) {
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        } else if (triggerEvent == CardResponded) {
            card = data.value<CardResponseStruct>().m_card;
        }

        if (card == NULL || !card->isBlack()
            || (card->getHandlingMethod() != Card::MethodUse && card->getHandlingMethod() != Card::MethodResponse))
            return false;

        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *tmp, room->getOtherPlayers(player)) {
            if (player->inMyAttackRange(tmp))
                targets << tmp;
        }
        if (targets.isEmpty()) return false;

        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@sp_moonspear", true, true);
        if (!target) return false;
        room->setEmotion(player, "weapon/moonspear");
        if (!room->askForCard(target, "jink", "@moon-spear-jink", QVariant(), Card::MethodResponse, player))
            room->damage(DamageStruct(objectName(), player, target));
        return false;
    }
};

SPMoonSpear::SPMoonSpear(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("SPMoonSpear");
}

class Jilei: public TriggerSkill {
public:
    Jilei(): TriggerSkill("jilei") {
        events << Damaged;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *current = room->getCurrent();
        if (!current || current->getPhase() == Player::NotActive || current->isDead() || !damage.from)
            return false;

        if (room->askForSkillInvoke(yangxiu, objectName(), data)) {
            QString choice = room->askForChoice(yangxiu, objectName(), "BasicCard+EquipCard+TrickCard");
            room->broadcastSkillInvoke(objectName());

            LogMessage log;
            log.type = "#Jilei";
            log.from = damage.from;
            log.arg = choice;
            room->sendLog(log);

            QStringList jilei_list = damage.from->tag[objectName()].toStringList();
            if (jilei_list.contains(choice)) return false;
            jilei_list.append(choice);
            damage.from->tag[objectName()] = QVariant::fromValue(jilei_list);
            QString _type = choice + "|.|.|hand"; // Handcards only
            room->setPlayerCardLimitation(damage.from, "use,response,discard", _type, true);

            QString type_name = choice.replace("Card", "").toLower();
            if (damage.from->getMark("@jilei_" + type_name) == 0)
                room->addPlayerMark(damage.from, "@jilei_" + type_name);
        }

        return false;
    }
};

class JileiClear: public TriggerSkill {
public:
    JileiClear(): TriggerSkill("#jilei-clear") {
        events << EventPhaseChanging << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != target || target != room->getCurrent())
                return false;
        }
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach (ServerPlayer *player, players) {
            QStringList jilei_list = player->tag["jilei"].toStringList();
            if (!jilei_list.isEmpty()) {
                LogMessage log;
                log.type = "#JileiClear";
                log.from = player;
                room->sendLog(log);

                foreach (QString jilei_type, jilei_list) {
                    room->removePlayerCardLimitation(player, "use,response,discard", jilei_type + "|.|.|hand$1");
                    QString type_name = jilei_type.replace("Card", "").toLower();
                    room->setPlayerMark(player, "@jilei_" + type_name, 0);
                }
                player->tag.remove("jilei");
            }
        }

        return false;
    }
};

class Danlao: public TriggerSkill {
public:
    Danlao(): TriggerSkill("danlao") {
        events << TargetConfirmed << CardEffected;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.to.length() <= 1 || !use.to.contains(player)
                || !use.card->isKindOf("TrickCard")
                || !room->askForSkillInvoke(player, objectName(), data))
                return false;

            player->tag["Danlao"] = use.card->toString();
            room->broadcastSkillInvoke(objectName());

            player->drawCards(1);
        } else {
            if (!player->isAlive() || !player->hasSkill(objectName()))
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (player->tag["Danlao"].isNull() || player->tag["Danlao"].toString() != effect.card->toString())
                return false;

            player->tag["Danlao"] = QVariant(QString());

            LogMessage log;
            log.type = "#DanlaoAvoid";
            log.from = player;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();
            room->sendLog(log);

            return true;
        }

        return false;
    }
};

Yongsi::Yongsi(): TriggerSkill("yongsi") {
    events << DrawNCards << EventPhaseStart;
    frequency = Compulsory;
}

int Yongsi::getKingdoms(ServerPlayer *yuanshu) const{
    QSet<QString> kingdom_set;
    Room *room = yuanshu->getRoom();
    foreach (ServerPlayer *p, room->getAlivePlayers())
        kingdom_set << p->getKingdom();

    return kingdom_set.size();
}

bool Yongsi::trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *yuanshu, QVariant &data) const{
    if (triggerEvent == DrawNCards) {
        int x = getKingdoms(yuanshu);
        data = data.toInt() + x;

        Room *room = yuanshu->getRoom();
        LogMessage log;
        log.type = "#YongsiGood";
        log.from = yuanshu;
        log.arg = QString::number(x);
        log.arg2 = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(yuanshu, objectName());

        room->broadcastSkillInvoke("yongsi");
    } else if (triggerEvent == EventPhaseStart && yuanshu->getPhase() == Player::Discard) {
        int x = getKingdoms(yuanshu);
        LogMessage log;
        log.type = yuanshu->getCardCount(true) > x ? "#YongsiBad" : "#YongsiWorst";
        log.from = yuanshu;
        log.arg = QString::number(log.type == "#YongsiBad" ? x : yuanshu->getCardCount(true));
        log.arg2 = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(yuanshu, objectName());
        if (x > 0){
            room->broadcastSkillInvoke("yongsi");
            room->askForDiscard(yuanshu, "yongsi", x, x, false, true);
        }
    }

    return false;
}

#include "standard-skillcards.h"
WeidiCard::WeidiCard() {
    target_fixed = true;
    mute = true;
}

void WeidiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *yuanshu = card_use.from;

    QStringList choices;
    if (yuanshu->hasLordSkill("jijiang") && room->getLord()->hasLordSkill("jijiang") && Slash::IsAvailable(yuanshu))
        choices << "jijiang";

    if (yuanshu->hasLordSkill("weidai") && Analeptic::IsAvailable(yuanshu) && !yuanshu->hasFlag("drank"))
        choices << "weidai";

    if (choices.isEmpty())
        return;

    QString choice = room->askForChoice(yuanshu, "weidi", choices.join("+"));

    if (choice == "jijiang") {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer* target, room->getOtherPlayers(yuanshu)){
            if (yuanshu->canSlash(target))
                targets << target;
        }

        ServerPlayer* target = room->askForPlayerChosen(yuanshu, targets, "jijiang");
        if(target){
            JijiangCard *jijiang = new JijiangCard;
            jijiang->setSkillName("weidi");
            CardUseStruct use;
            use.card = jijiang;
            use.from = yuanshu;
            use.to << target;
            room->useCard(use);
        }
    } else {
        WeidaiCard *weidai = new WeidaiCard;
        weidai->setSkillName("weidi");
        CardUseStruct use;
        use.card = weidai;
        use.from = yuanshu;
        room->useCard(use);
    }
}

class WeidiViewAsSkill: public ZeroCardViewAsSkill {
public:
    WeidiViewAsSkill(): ZeroCardViewAsSkill("weidi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->hasLordSkill("jijiang") && Slash::IsAvailable(player))
                ||(player->hasLordSkill("weidai") && Analeptic::IsAvailable(player) && !player->hasFlag("drank"));
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->hasLordSkill("weidai"))
            return pattern == "peach+analeptic";
        else if (player->hasLordSkill("jijiang")) {
            JijiangViewAsSkill *jijiang = new JijiangViewAsSkill;
            jijiang->deleteLater();
            return jijiang->isEnabledAtResponse(player, pattern);
        }

        return false;
    }

    virtual const Card *viewAs() const{
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
            return new WeidaiCard;
        else
            return new WeidiCard;
    }
};

class Weidi: public GameStartSkill {
public:
    Weidi(): GameStartSkill("weidi") {
        frequency = Compulsory;
        view_as_skill = new WeidiViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *) const{
        return;
    }
};

class Yicong: public DistanceSkill {
public:
    Yicong(): DistanceSkill("yicong") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if (from->hasSkill(objectName()) && from->getHp() > 2)
            correct--;
        if (to->hasSkill(objectName()) && to->getHp() <= 2)
            correct++;

        return correct;
    }
};

class YicongEffect: public TriggerSkill {
public:
    YicongEffect(): TriggerSkill("#yicong-effect") {
        events << PostHpReduced << HpRecover;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        int hp = player->getHp();
        int index = 0;
        if (triggerEvent == HpRecover) {
            RecoverStruct recover = data.value<RecoverStruct>();
            if (hp > 2 && hp - recover.recover <= 2)
                index = 1;
        } else if (triggerEvent == PostHpReduced) {
            int reduce = 0;
            if (data.canConvert<DamageStruct>()) {
                DamageStruct damage = data.value<DamageStruct>();
                reduce = damage.damage;
            } else
                reduce = data.toInt();
            if (hp <= 2 && hp + reduce > 2)
                index = 2;
        }

        if (index > 0)
            room->broadcastSkillInvoke("yicong", index);
        return false;
    }
};

class Danji: public PhaseChangeSkill {
public:
    Danji(): PhaseChangeSkill("danji") { // What a silly skill!
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Start
               && target->getMark("danji") == 0
               && target->getHandcardNum() > target->getHp();
    }

    virtual bool onPhaseChange(ServerPlayer *guanyu) const{
        Room *room = guanyu->getRoom();
        ServerPlayer *the_lord = room->getLord();
        if (the_lord && (the_lord->getGeneralName() == "caocao" || the_lord->getGeneral2Name() == "caocao")) {
            room->notifySkillInvoked(guanyu, objectName());

            LogMessage log;
            log.type = "#DanjiWake";
            log.from = guanyu;
            log.arg = QString::number(guanyu->getHandcardNum());
            log.arg2 = QString::number(guanyu->getHp());
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$DanjiAnimate", 5000);

            room->addPlayerMark(guanyu, "danji");
            if (room->changeMaxHpForAwakenSkill(guanyu))
                room->acquireSkill(guanyu, "mashu");
        }

        return false;
    }
};

YuanhuCard::YuanhuCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool YuanhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void YuanhuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    int index = -1;
    if (card_use.to.first() == card_use.from)
        index = 5;
    else if (card_use.to.first()->getGeneralName().contains("caocao"))
        index = 4;
    else {
        const Card *card = Sanguosha->getCard(card_use.card->getSubcards().first());
        if (card->isKindOf("Weapon"))
            index = 1;
        else if (card->isKindOf("Armor"))
            index = 2;
        else if (card->isKindOf("Horse"))
            index = 3;
    }
    room->broadcastSkillInvoke("yuanhu", index);
    SkillCard::onUse(room, card_use);
}

void YuanhuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caohong = effect.from;
    Room *room = caohong->getRoom();
    room->moveCardTo(this, caohong, effect.to, Player::PlaceEquip,
                     CardMoveReason(CardMoveReason::S_REASON_PUT, caohong->objectName(), "yuanhu", QString()));

    const Card *card = Sanguosha->getCard(subcards.first());

    LogMessage log;
    log.type = "$ZhijianEquip";
    log.from = effect.to;
    log.card_str = QString::number(card->getEffectiveId());
    room->sendLog(log);

    if (card->isKindOf("Weapon")) {
      QList<ServerPlayer *> targets;
      foreach (ServerPlayer *p, room->getAllPlayers()) {
          if (effect.to->distanceTo(p) == 1 && caohong->canDiscard(p, "hej"))
              targets << p;
      }
      if (!targets.isEmpty()) {
          ServerPlayer *to_dismantle = room->askForPlayerChosen(caohong, targets, "yuanhu", "@yuanhu-discard:" + effect.to->objectName());
          int card_id = room->askForCardChosen(caohong, to_dismantle, "hej", "yuanhu", false, Card::MethodDiscard);
          room->throwCard(Sanguosha->getCard(card_id), to_dismantle, caohong);
      }
    } else if (card->isKindOf("Armor")) {
        effect.to->drawCards(1);
    } else if (card->isKindOf("Horse")) {
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }
}

class YuanhuViewAsSkill: public OneCardViewAsSkill {
public:
    YuanhuViewAsSkill(): OneCardViewAsSkill("yuanhu") {
        filter_pattern = "EquipCard";
        response_pattern = "@@yuanhu";
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        YuanhuCard *first = new YuanhuCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Yuanhu: public PhaseChangeSkill {
public:
    Yuanhu(): PhaseChangeSkill("yuanhu") {
        view_as_skill = new YuanhuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if (target->getPhase() == Player::Finish && !target->isNude())
            room->askForUseCard(target, "@@yuanhu", "@yuanhu-equip", -1, Card::MethodNone);
        return false;
    }
};

XuejiCard::XuejiCard() {
}

bool XuejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= Self->getLostHp())
        return false;

    if (to_select == Self)
        return false;

    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getEffectiveId() == getEffectiveId()) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - Self->getAttackRange(false);
    } else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getEffectiveId() == getEffectiveId())
        range_fix += 1;

    return Self->distanceTo(to_select, range_fix) <= Self->getAttackRange();
}

void XuejiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    DamageStruct damage;
    damage.from = source;
    damage.reason = "xueji";

    foreach (ServerPlayer *p, targets) {
        damage.to = p;
        room->damage(damage);
    }
    foreach (ServerPlayer *p, targets) {
        if (p->isAlive())
            p->drawCards(1);
    }
}

class Xueji: public OneCardViewAsSkill {
public:
    Xueji(): OneCardViewAsSkill("xueji") {
        filter_pattern = ".|red!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getLostHp() > 0 && player->canDiscard(player, "he") && !player->hasUsed("XuejiCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        XuejiCard *first = new XuejiCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Huxiao: public TargetModSkill {
public:
    Huxiao(): TargetModSkill("huxiao") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return from->getMark(objectName());
        else
            return 0;
    }
};

class HuxiaoCount: public TriggerSkill {
public:
    HuxiaoCount(): TriggerSkill("#huxiao-count") {
        events << SlashMissed << EventPhaseChanging;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == SlashMissed) {
            if (player->getPhase() == Player::Play)
                room->addPlayerMark(player, "huxiao");
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play)
                if (player->getMark("huxiao") > 0)
                    room->setPlayerMark(player, "huxiao", 0);
        }

        return false;
    }
};

class HuxiaoClear: public DetachEffectSkill {
public:
    HuxiaoClear(): DetachEffectSkill("huxiao") {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        room->setPlayerMark(player, "huxiao", 0);
    }
};

class WujiCount: public TriggerSkill {
public:
    WujiCount(): TriggerSkill("#wuji-count") {
        events << PreDamageDone << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == PreDamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from->isAlive() && damage.from == room->getCurrent() && damage.from->getMark("wuji") == 0)
                room->addPlayerMark(damage.from, "wuji_damage", damage.damage);
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                if (player->getMark("wuji_damage") > 0)
                    room->setPlayerMark(player, "wuji_damage", 0);
        }

        return false;
    }
};

class Wuji: public PhaseChangeSkill {
public:
    Wuji(): PhaseChangeSkill("wuji") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Finish
               && target->getMark("wuji") == 0
               && target->getMark("wuji_damage") >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->notifySkillInvoked(player, objectName());

        LogMessage log;
        log.type = "#WujiWake";
        log.from = player;
        log.arg = QString::number(player->getMark("wuji_damage"));
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$WujiAnimate", 4000);

        room->addPlayerMark(player, "wuji");

        if (room->changeMaxHpForAwakenSkill(player, 1)) {
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);

            room->detachSkillFromPlayer(player, "huxiao");
        }

        return false;
    }
};

class Baobian: public TriggerSkill {
public:
    Baobian(): TriggerSkill("baobian") {
        events << GameStart << HpChanged << MaxHpChanged << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventLoseSkill) {
            if (data.toString() == objectName()) {
                QStringList baobian_skills = player->tag["BaobianSkills"].toStringList();
                QStringList detachList;
                foreach (QString skill_name, baobian_skills)
                    detachList.append("-" + skill_name);
                room->handleAcquireDetachSkills(player, detachList);
                player->tag["BaobianSkills"] = QVariant();
            }
            return false;
        } else if (triggerEvent == EventAcquireSkill) {
            if (data.toString() != objectName()) return false;
        }

        if (!player->isAlive() || !player->hasSkill(objectName(), true)) return false;

        acquired_skills.clear();
        detached_skills.clear();
        BaobianChange(room, player, 1, "shensu");
        BaobianChange(room, player, 2, "paoxiao");
        BaobianChange(room, player, 3, "tiaoxin");
        if (!acquired_skills.isEmpty() || !detached_skills.isEmpty())
            room->handleAcquireDetachSkills(player, acquired_skills + detached_skills);
        return false;
    }


private:
    void BaobianChange(Room *room, ServerPlayer *player, int hp, const QString &skill_name) const{
        QStringList baobian_skills = player->tag["BaobianSkills"].toStringList();
        if (player->getHp() <= hp) {
            if (!baobian_skills.contains(skill_name)) {
                room->notifySkillInvoked(player, "baobian");
                acquired_skills.append(skill_name);
                baobian_skills << skill_name;
            }
        } else {
            if (baobian_skills.contains(skill_name)) {
                detached_skills.append("-" + skill_name);
                baobian_skills.removeOne(skill_name);
            }
        }
        player->tag["BaobianSkills"] = QVariant::fromValue(baobian_skills);
    }

    mutable QStringList acquired_skills, detached_skills;
};

BifaCard::BifaCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool BifaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getPile("bifa").isEmpty() && to_select != Self;
}

void BifaCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    target->tag["BifaSource" + QString::number(getEffectiveId())] = QVariant::fromValue((PlayerStar)source);
    room->broadcastSkillInvoke("bifa", 1);
    target->addToPile("bifa", this, false);
}

class BifaViewAsSkill: public OneCardViewAsSkill {
public:
    BifaViewAsSkill(): OneCardViewAsSkill("bifa") {
        filter_pattern = ".|.|.|hand";
        response_pattern = "@@bifa";
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        Card *card = new BifaCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class Bifa: public TriggerSkill {
public:
    Bifa(): TriggerSkill("bifa") {
        events << EventPhaseStart;
        view_as_skill = new BifaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish && !player->isKongcheng()) {
            room->askForUseCard(player, "@@bifa", "@bifa-remove", -1, Card::MethodNone);
        } else if (player->getPhase() == Player::RoundStart && player->getPile("bifa").length() > 0) {
            QList<int> bifa_list = player->getPile("bifa");

            while (!bifa_list.isEmpty()) {
                int card_id = bifa_list.last();
                ServerPlayer *chenlin = player->tag["BifaSource" + QString::number(card_id)].value<PlayerStar>();
                QList<int> ids;
                ids << card_id;

                LogMessage log;
                log.type = "$BifaView";
                log.from = player;
                log.card_str = QString::number(card_id);
                log.arg = "bifa";
                room->doNotify(player, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

                room->fillAG(ids, player);
                const Card *cd = Sanguosha->getCard(card_id);
                QString pattern;
                if (cd->isKindOf("BasicCard"))
                    pattern = "BasicCard";
                else if (cd->isKindOf("TrickCard"))
                    pattern = "TrickCard";
                else if (cd->isKindOf("EquipCard"))
                    pattern = "EquipCard";
                QVariant data_for_ai = QVariant::fromValue(pattern);
                pattern.append("|.|.|hand");
                const Card *to_give = NULL;
                if (!player->isKongcheng() && chenlin && chenlin->isAlive())
                    to_give = room->askForCard(player, pattern, "@bifa-give", data_for_ai, Card::MethodNone, chenlin);
                if (chenlin && to_give) {
                    room->broadcastSkillInvoke(objectName(), 2);
                    chenlin->obtainCard(to_give, false);
                    player->obtainCard(cd, false);
                } else {
                    room->broadcastSkillInvoke(objectName(), 2);
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                    room->throwCard(cd, reason, NULL);
                    room->loseHp(player);
                }
                bifa_list.removeOne(card_id);
                room->clearAG(player);
                player->tag.remove("BifaSource" + QString::number(card_id));
            }
        }
        return false;
    }
};

SongciCard::SongciCard() {
    mute = true;
}

bool SongciCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getMark("songci" + Self->objectName()) == 0 && to_select->getHandcardNum() != to_select->getHp();
}

void SongciCard::onEffect(const CardEffectStruct &effect) const{
    int handcard_num = effect.to->getHandcardNum();
    int hp = effect.to->getHp();
    effect.to->gainMark("@songci");
    Room *room = effect.from->getRoom();
    room->addPlayerMark(effect.to, "songci" + effect.from->objectName());
    if (handcard_num > hp) {
        room->broadcastSkillInvoke("songci", 2);
        room->askForDiscard(effect.to, "songci", 2, 2, false, true);
    } else if (handcard_num < hp) {
        room->broadcastSkillInvoke("songci", 1);
        effect.to->drawCards(2, "songci");
    }
}

class SongciViewAsSkill: public ZeroCardViewAsSkill {
public:
    SongciViewAsSkill(): ZeroCardViewAsSkill("songci") {
    }

    virtual const Card *viewAs() const{
        return new SongciCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (player->getMark("songci" + player->objectName()) == 0 && player->getHandcardNum() != player->getHp()) return true;
        foreach (const Player *sib, player->getAliveSiblings())
            if (sib->getMark("songci" + player->objectName()) == 0 && sib->getHandcardNum() != sib->getHp())
                return true;
        return false;
    }
};

class Songci: public TriggerSkill {
public:
    Songci(): TriggerSkill("songci") {
        events << Death;
        view_as_skill = new SongciViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player) return false;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->getMark("@songci") > 0)
                room->setPlayerMark(p, "@songci", 0);
            if (p->getMark("songci" + player->objectName()) > 0)
                room->setPlayerMark(p, "songci" + player->objectName(), 0);
        }
        return false;
    }
};

class Xiuluo: public PhaseChangeSkill {
public:
    Xiuluo(): PhaseChangeSkill("xiuluo") {
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Start
               && target->canDiscard(target, "h")
               && hasDelayedTrick(target);
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        while (hasDelayedTrick(target) && target->canDiscard(target, "h")) {
            QStringList suits;
            foreach (const Card *jcard, target->getJudgingArea()) {
                if (!suits.contains(jcard->getSuitString()))
                    suits << jcard->getSuitString();
            }

            const Card *card = room->askForCard(target, QString(".|%1|.|hand").arg(suits.join(",")),
                                                "@xiuluo", QVariant(), objectName());
            if (!card || !hasDelayedTrick(target)) break;
            room->broadcastSkillInvoke(objectName());

            QList<int> avail_list, other_list;
            foreach (const Card *jcard, target->getJudgingArea()) {
                if (jcard->isKindOf("SkillCard")) continue;
                if (jcard->getSuit() == card->getSuit())
                    avail_list << jcard->getEffectiveId();
                else
                    other_list << jcard->getEffectiveId();
            }
            room->fillAG(avail_list + other_list, NULL, other_list);
            int id = room->askForAG(target, avail_list, false, objectName());
            room->clearAG();
            room->throwCard(id, NULL);
        }

        return false;
    }

private:
    static bool hasDelayedTrick(const ServerPlayer *target) {
        foreach (const Card *card, target->getJudgingArea())
            if (!card->isKindOf("SkillCard")) return true;
        return false;
    }
};

class Shenwei: public DrawCardsSkill {
public:
    Shenwei(): DrawCardsSkill("#shenwei-draw") {
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        room->broadcastSkillInvoke("shenwei");
        room->notifySkillInvoked(player, "shenwei");
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = "shenwei";
        room->sendLog(log);

        return n + 2;
    }
};

class ShenweiKeep: public MaxCardsSkill {
public:
    ShenweiKeep(): MaxCardsSkill("shenwei") {
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasSkill(objectName()))
            return 2;
        else
            return 0;
    }
};

class Shenji: public TargetModSkill {
public:
    Shenji(): TargetModSkill("shenji") {
    }

    virtual int getExtraTargetNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()) && from->getWeapon() == NULL)
            return 2;
        else
            return 0;
    }
};

class Xingwu: public TriggerSkill {
public:
    Xingwu(): TriggerSkill("xingwu") {
        events << PreCardUsed << CardResponded << EventPhaseStart << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == PreCardUsed || triggerEvent == CardResponded) {
            CardStar card = NULL;
            if (triggerEvent == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct response = data.value<CardResponseStruct>();
                if (response.m_isUse)
                   card = response.m_card;
            }
            if (card && card->getTypeId() != Card::TypeSkill && card->getHandlingMethod() == Card::MethodUse) {
                int n = player->getMark(objectName());
                if (card->isBlack())
                    n |= 1;
                else if (card->isRed())
                    n |= 2;
                player->setMark(objectName(), n);
            }
        } else if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() == Player::Discard) {
                int n = player->getMark(objectName());
                bool red_avail = ((n & 2) == 0), black_avail = ((n & 1) == 0);
                if (player->isKongcheng() || (!red_avail && !black_avail))
                    return false;
                QString pattern = ".|.|.|hand";
                if (red_avail != black_avail)
                    pattern = QString(".|%1|.|hand").arg(red_avail ? "red" : "black");
                const Card *card = room->askForCard(player, pattern, "@xingwu", QVariant(), Card::MethodNone);
                if (card) {
                    room->notifySkillInvoked(player, objectName());
                    room->broadcastSkillInvoke(objectName(), 1);

                    LogMessage log;
                    log.type = "#InvokeSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);

                    player->addToPile(objectName(), card);
                }
            } else if (player->getPhase() == Player::RoundStart) {
                player->setMark(objectName(), 0);
            }
        } else if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to == player && move.to_place == Player::PlaceSpecial && player->getPile(objectName()).length() >= 3) {
                player->clearOnePrivatePile(objectName());
                QList<ServerPlayer *> males;
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (p->isMale())
                        males << p;
                }
                if (males.isEmpty()) return false;

                ServerPlayer *target = room->askForPlayerChosen(player, males, objectName(), "@xingwu-choose");
                room->broadcastSkillInvoke(objectName(), 2);
                room->damage(DamageStruct(objectName(), player, target, 2));

                if (!player->isAlive()) return false;
                QList<const Card *> equips = target->getEquips();
                if (!equips.isEmpty()) {
                    DummyCard *dummy = new DummyCard;
                    foreach (const Card *equip, equips) {
                        if (player->canDiscard(target, equip->getEffectiveId()))
                            dummy->addSubcard(equip);
                    }
                    if (dummy->subcardsLength() > 0)
                        room->throwCard(dummy, target, player);
                    delete dummy;
                }
            }
        }
        return false;
    }
};

class Luoyan: public TriggerSkill {
public:
    Luoyan(): TriggerSkill("luoyan") {
        events << CardsMoveOneTime << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventLoseSkill && data.toString() == objectName()) {
            room->handleAcquireDetachSkills(player, "-tianxiang|-liuli", true);
        } else if (triggerEvent == EventAcquireSkill && data.toString() == objectName()) {
            if (!player->getPile("xingwu").isEmpty()) {
                room->notifySkillInvoked(player, objectName());
                room->handleAcquireDetachSkills(player, "tianxiang|liuli");
            }
        } else if (triggerEvent == CardsMoveOneTime && player->isAlive() && player->hasSkill(objectName(), true)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to == player && move.to_place == Player::PlaceSpecial && move.to_pile_name == "xingwu") {
                if (player->getPile("xingwu").length() == 1) {
                    room->notifySkillInvoked(player, objectName());
                    room->handleAcquireDetachSkills(player, "tianxiang|liuli");
                }
            } else if (move.from == player && move.from_places.contains(Player::PlaceSpecial)
                       && move.from_pile_names.contains("xingwu")) {
                if (player->getPile("xingwu").isEmpty())
                    room->handleAcquireDetachSkills(player, "-tianxiang|-liuli", true);
            }
        }
        return false;
    }
};

class Yanyu: public TriggerSkill {
public:
    Yanyu(): TriggerSkill("yanyu") {
        events << EventPhaseStart << BeforeCardsMove << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            ServerPlayer *xiahou = room->findPlayerBySkillName(objectName());
            if (xiahou && player->getPhase() == Player::Play) {
                if (!xiahou->canDiscard(xiahou, "he")) return false;
                const Card *card = room->askForCard(xiahou, "..", "@yanyu-discard", QVariant(), objectName());
                if (card)
                    xiahou->addMark("YanyuDiscard" + QString::number(card->getTypeId()), 3);
            }
        } else if (triggerEvent == BeforeCardsMove && TriggerSkill::triggerable(player)) {
            ServerPlayer *current = room->getCurrent();
            if (!current || current->getPhase() != Player::Play) return false;
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to_place == Player::DiscardPile) {
                QList<int> ids, disabled;
                QList<int> all_ids = move.card_ids;
                foreach (int id, move.card_ids) {
                    const Card *card = Sanguosha->getCard(id);
                    if (player->getMark("YanyuDiscard" + QString::number(card->getTypeId())) > 0)
                        ids << id;
                    else
                        disabled << id;
                }
                if (ids.isEmpty()) return false;
                while (!ids.isEmpty()) {
                    room->fillAG(all_ids, player, disabled);
                    bool only = (all_ids.length() == 1);
                    int card_id = -1;
                    if (only)
                        card_id = ids.first();
                    else
                        card_id = room->askForAG(player, ids, true, objectName());
                    if (card_id == -1) break;
                    if (only)
                        player->setMark("YanyuOnlyId", card_id + 1); // For AI
                    const Card *card = Sanguosha->getCard(card_id);
                    ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(),
                                                                    QString("@yanyu-give:::%1:%2\\%3").arg(card->objectName())
                                                                                                      .arg(card->getSuitString() + "_char")
                                                                                                      .arg(card->getNumberString()),
                                                                    only, true);
                    room->clearAG(player);
                    player->setMark("YanyuOnlyId", 0);
                    if (target) {
                        player->removeMark("YanyuDiscard" + QString::number(card->getTypeId()));
                        int index = move.card_ids.indexOf(card_id);
                        Player::Place place = move.from_places.at(index);
                        move.from_places.removeAt(index);
                        move.card_ids.removeOne(card_id);
                        data = QVariant::fromValue(move);
                        ids.removeOne(card_id);
                        disabled << card_id;
                        foreach (int id, ids) {
                            const Card *card = Sanguosha->getCard(id);
                            if (player->getMark("YanyuDiscard" + QString::number(card->getTypeId())) == 0) {
                                ids.removeOne(id);
                                disabled << id;
                            }
                        }
                        if (move.from && move.from->objectName() == target->objectName() && place != Player::PlaceTable) {
                            // just indicate which card she chose...
                            LogMessage log;
                            log.type = "$MoveCard";
                            log.from = target;
                            log.to << target;
                            log.card_str = QString::number(card_id);
                            room->sendLog(log);
                        }
                        target->obtainCard(card);
                    } else
                        break;
                }
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    p->setMark("YanyuDiscard1", 0);
                    p->setMark("YanyuDiscard2", 0);
                    p->setMark("YanyuDiscard3", 0);
                }
            }
        }
        return false;
    }
};

class Xiaode: public TriggerSkill {
public:
    Xiaode(): TriggerSkill("xiaode") {
        events << BuryVictim;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *xiahoushi = room->findPlayerBySkillName(objectName());
        if (!xiahoushi || !xiahoushi->tag["XiaodeSkill"].toString().isEmpty()) return false;
        QStringList skill_list;
        skill_list.append(addSkillList(player->getGeneral()));
        skill_list.append(addSkillList(player->getGeneral2()));
        if (skill_list.isEmpty()) return false;
        if (!room->askForSkillInvoke(xiahoushi, objectName(), QVariant::fromValue(skill_list))) return false;
        QString skill_name = room->askForChoice(xiahoushi, objectName(), skill_list.join("+"));
        xiahoushi->tag["XiaodeSkill"] = skill_name;
        room->acquireSkill(xiahoushi, skill_name);
        return false;
    }

private:
    QStringList addSkillList(const General *general) const{
        if (!general) return QStringList();
        QStringList skill_list;
        foreach (const Skill *skill, general->getSkillList()) {
            if (skill->isVisible() && !skill->isLordSkill() && skill->getFrequency() != Skill::Wake)
                skill_list.append(skill->objectName());
        }
        return skill_list;
    }
};

class XiaodeEx: public TriggerSkill {
public:
    XiaodeEx(): TriggerSkill("#xiaode") {
        events << EventPhaseChanging << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                QString skill_name = player->tag["XiaodeSkill"].toString();
                if (!skill_name.isEmpty()) {
                    room->detachSkillFromPlayer(player, skill_name, false, true);
                    player->tag.remove("XiaodeSkill");
                }
            }
        } else if (triggerEvent == EventLoseSkill && data.toString() == objectName()) {
            QString skill_name = player->tag["XiaodeSkill"].toString();
            if (!skill_name.isEmpty()) {
                room->detachSkillFromPlayer(player, skill_name, false, true);
                player->tag.remove("XiaodeSkill");
            }
        }
        return false;
    }
};

ZhoufuCard::ZhoufuCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhoufuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && to_select->getPile("incantation").isEmpty();
}

void ZhoufuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    target->tag["ZhoufuSource" + QString::number(getEffectiveId())] = QVariant::fromValue((PlayerStar)source);
    room->broadcastSkillInvoke("zhoufu");
    target->addToPile("incantation", this);
}

class ZhoufuViewAsSkill: public OneCardViewAsSkill {
public:
    ZhoufuViewAsSkill(): OneCardViewAsSkill("zhoufu") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhoufuCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        Card *card = new ZhoufuCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class Zhoufu: public TriggerSkill {
public:
    Zhoufu(): TriggerSkill("zhoufu") {
        events << StartJudge << EventPhaseChanging;
        view_as_skill = new ZhoufuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getPile("incantation").length() > 0;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == StartJudge) {
            int card_id = player->getPile("incantation").first();

            JudgeStar judge = data.value<JudgeStar>();
            judge->card = Sanguosha->getCard(card_id);

            LogMessage log;
            log.type = "$ZhoufuJudge";
            log.from = player;
            log.arg = objectName();
            log.card_str = QString::number(judge->card->getEffectiveId());
            room->sendLog(log);

            room->moveCardTo(judge->card, NULL, judge->who, Player::PlaceJudge,
                             CardMoveReason(CardMoveReason::S_REASON_JUDGE,
                             judge->who->objectName(),
                             QString(), QString(), judge->reason), true);
            judge->updateResult();
            room->setTag("SkipGameRule", true);
        } else {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                int id = player->getPile("incantation").first();
                PlayerStar zhangbao = player->tag["ZhoufuSource" + QString::number(id)].value<PlayerStar>();
                if (zhangbao && zhangbao->isAlive())
                    zhangbao->obtainCard(Sanguosha->getCard(id));
            }
        }
        return false;
    }
};

class Yingbing: public TriggerSkill {
public:
    Yingbing(): TriggerSkill("yingbing") {
        events << StartJudge;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        int id = judge->card->getEffectiveId();
        PlayerStar zhangbao = player->tag["ZhoufuSource" + QString::number(id)].value<PlayerStar>();
        if (zhangbao && TriggerSkill::triggerable(zhangbao)
            && zhangbao->askForSkillInvoke(objectName(), data))
            zhangbao->drawCards(2);
        return false;
    }
};

#include "jsonutils.h"
class AocaiViewAsSkill: public ZeroCardViewAsSkill {
public:
    AocaiViewAsSkill(): ZeroCardViewAsSkill("aocai") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->getPhase() != Player::NotActive || player->hasFlag("Global_AocaiFailed")) return false;
        if (pattern == "slash")
            return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
        else if (pattern == "peach")
            return !player->hasFlag("Global_PreventPeach");
        else if (pattern.contains("analeptic"))
            return true;
        return false;
    }

    virtual const Card *viewAs() const{
        AocaiCard *aocai_card = new AocaiCard;
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "peach+analeptic" && Self->hasFlag("Global_PreventPeach"))
            pattern = "analeptic";
        aocai_card->setUserString(pattern);
        return aocai_card;
    }
};

class Aocai: public TriggerSkill {
public:
    Aocai(): TriggerSkill("aocai") {
        events << CardAsked;
        view_as_skill = new AocaiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QString pattern = data.toStringList().first();
        if (player->getPhase() == Player::NotActive
            && (pattern == "slash" || pattern == "jink")
            && room->askForSkillInvoke(player, objectName(), data)) {
            QList<int> ids = room->getNCards(2, false);
            QList<int> enabled, disabled;
            foreach (int id, ids) {
                if (Sanguosha->getCard(id)->objectName().contains(pattern))
                    enabled << id;
                else
                    disabled << id;
            }
            int id = Aocai::view(room, player, ids, enabled, disabled);
            if (id != -1) {
                const Card *card = Sanguosha->getCard(id);
                room->provide(card);
                return true;
            }
        }
        return false;
    }

    static int view(Room *room, ServerPlayer *player, QList<int> &ids, QList<int> &enabled, QList<int> &disabled) {
        int result = -1;
        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(ids).join("+");
        room->doNotify(player, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
        room->broadcastSkillInvoke("aocai");
        room->notifySkillInvoked(player, "aocai");
        if (enabled.isEmpty()) {
            Json::Value arg(Json::arrayValue);
            arg[0] = QSanProtocol::Utils::toJsonString(".");
            arg[1] = false;
            arg[2] = QSanProtocol::Utils::toJsonArray(ids);
            room->doNotify(player, QSanProtocol::S_COMMAND_SHOW_ALL_CARDS, arg);
        } else {
            room->fillAG(ids, player, disabled);
            int id = room->askForAG(player, enabled, true, "aocai");
            if (id != -1) {
                ids.removeOne(id);
                result = id;
            }
            room->clearAG(player);
        }

        QList<int> &drawPile = room->getDrawPile();
        for (int i = ids.length() - 1; i >= 0; i--)
            drawPile.prepend(ids.at(i));
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, Json::Value(drawPile.length()));
        if (result == -1)
            room->setPlayerFlag(player, "Global_AocaiFailed");
        return result;
    }
};

AocaiCard::AocaiCard() {
}

bool AocaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    QString name;
    const Card *card = NULL;
    if (!user_string.isEmpty()) {
        name = user_string.split("+").first();
        card = Sanguosha->cloneCard(name);
    }
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool AocaiCard::targetFixed() const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
        return true;

    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFixed();
}

bool AocaiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    QString name;
    const Card *card = NULL;
    if (!user_string.isEmpty()) {
        name = user_string.split("+").first();
        card = Sanguosha->cloneCard(name);
    }
    return card && card->targetsFeasible(targets, Self);
}

const Card *AocaiCard::validateInResponse(ServerPlayer *user) const{
    Room *room = user->getRoom();
    QList<int> ids = room->getNCards(2, false);
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    QList<int> enabled, disabled;
    foreach (int id, ids) {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "aocai";
    room->sendLog(log);

    int id = Aocai::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

const Card *AocaiCard::validate(CardUseStruct &cardUse) const{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *user = cardUse.from;
    Room *room = user->getRoom();
    QList<int> ids = room->getNCards(2, false);
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    QList<int> enabled, disabled;
    foreach (int id, ids) {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "aocai";
    room->sendLog(log);

    int id = Aocai::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

DuwuCard::DuwuCard() {
}

bool DuwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || qMax(0, to_select->getHp()) != subcardsLength())
        return false;
    if (!Self->inMyAttackRange(to_select) || Self == to_select)
        return false;

    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        int distance_fix = weapon->getRange() - Self->getAttackRange(false);
        if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
            distance_fix += 1;
        return Self->distanceTo(to_select, distance_fix) <= Self->getAttackRange();
    } else if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId())) {
        return Self->distanceTo(to_select, 1) <= Self->getAttackRange();
    } else
        return true;
}

void DuwuCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->damage(DamageStruct("duwu", effect.from, effect.to));
}

class DuwuViewAsSkill: public ViewAsSkill {
public:
    DuwuViewAsSkill(): ViewAsSkill("duwu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasFlag("DuwuEnterDying");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        DuwuCard *duwu = new DuwuCard;
        if (!cards.isEmpty())
            duwu->addSubcards(cards);
        return duwu;
    }
};

class Duwu: public TriggerSkill {
public:
    Duwu(): TriggerSkill("duwu") {
        events << QuitDying;
        view_as_skill = new DuwuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage && dying.damage->getReason() == "duwu") {
            ServerPlayer *from = dying.damage->from;
            if (from && from->isAlive()) {
                room->setPlayerFlag(from, "DuwuEnterDying");
                room->loseHp(from, 1);
            }
        }
        return false;
    }
};

class Shuijian: public DrawCardsSkill {
public:
    Shuijian(): DrawCardsSkill("shuijian") {
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        if (player->askForSkillInvoke(objectName())) {
            player->getRoom()->broadcastSkillInvoke(objectName());
            return n + player->getEquips().length() / 2 + 1;
        } else
            return n;
    }
};

JisuCard::JisuCard() {
    mute = true;
}

bool JisuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("jisu");
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

void JisuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *target, targets) {
        if (!source->canSlash(target, NULL, false))
            targets.removeOne(target);
    }

    if (targets.length() > 0) {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_jisu");
        room->useCard(CardUseStruct(slash, source, targets));
    }
}

class JisuViewAsSkill: public ZeroCardViewAsSkill {
public:
    JisuViewAsSkill(): ZeroCardViewAsSkill("jisu") {
        response_pattern = "@@jisu";
    }

    virtual const Card *viewAs() const{
        return new JisuCard;
    }
};

class Jisu: public TriggerSkill {
public:
    Jisu(): TriggerSkill("jisu") {
        events << EventPhaseChanging;
        view_as_skill = new JisuViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Judge && !player->isSkipped(Player::Judge)
            && !player->isSkipped(Player::Draw)) {
            if (Slash::IsAvailable(player) && room->askForUseCard(player, "@@jisu", "@jisu-slash")) {
                player->skip(Player::Judge);
                player->skip(Player::Draw);
            }
        }
        return false;
    }
};

class Shuiyong: public TriggerSkill {
public:
    Shuiyong(): TriggerSkill("shuiyong") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.nature == DamageStruct::Fire) {
            room->notifySkillInvoked(player, objectName());
            room->broadcastSkillInvoke(objectName());

            LogMessage log;
            log.type = "#ShuiyongProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = "fire_nature";
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

Fentian::Fentian(): PhaseChangeSkill("fentian"){
    frequency = Compulsory;
}

bool Fentian::onPhaseChange(ServerPlayer *hanba) const{
    if (hanba->getPhase() != Player::Finish)
        return false;

    if (hanba->getHandcardNum() >= hanba->getHp())
        return false;

    QList<ServerPlayer*> targets;
    Room* room = hanba->getRoom();
    foreach(ServerPlayer* p, room->getOtherPlayers(hanba)){
        if (hanba->inMyAttackRange(p) && !p->isNude())
            targets << p;
    };

    if (targets.isEmpty())
        return false;

    room->broadcastSkillInvoke(objectName());
    ServerPlayer *target = room->askForPlayerChosen(hanba, targets, objectName());
    int id = room->askForCardChosen(hanba, target, "he", objectName());
    hanba->addToPile("burn", id);
    return false;
}

Zhiri::Zhiri(): PhaseChangeSkill("zhiri") {
    frequency = Wake;
}

bool Zhiri::onPhaseChange(ServerPlayer *hanba) const {
    if (hanba->getMark(objectName()) > 0 || hanba->getPhase() != Player::Start)
        return false;

    if (hanba->getPile("burn").length() < 3)
        return false;

    Room *room = hanba->getRoom();
    room->broadcastSkillInvoke(objectName());
    room->doLightbox("$ZhiriAnimate", 4000);

    if (room->changeMaxHpForAwakenSkill(hanba)) {
        room->acquireSkill(hanba, "xintan");
        room->addPlayerMark(hanba, objectName());
    }

    return false;
};

XintanCard::XintanCard() {
    will_throw = true;
    handling_method = Card::MethodNone;
}

bool XintanCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const {
    return targets.isEmpty();
}

void XintanCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *hanba = effect.from;
    QList<int> burn = hanba->getPile("burn");
    if (burn.length() < 2)
        return;

    Room *room = hanba->getRoom();
    QList<int> subs;

    if (burn.length() == 2)
        subs = burn;
    else {
        while (subs.length() < 2) {
            room->fillAG(burn, hanba);
            int id = room->askForAG(hanba, burn, false, objectName());
            subs << id;
            burn.removeOne(id);
            room->clearAG(hanba);
        };
    };
    CardsMoveStruct move;
    move.from = hanba;
    move.to_place = Player::DiscardPile;
    move.reason = CardMoveReason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, hanba->objectName(), objectName(), QString());
    move.card_ids = subs;
    room->moveCardsAtomic(move, true);

    room->loseHp(effect.to);
}

class Xintan: public ZeroCardViewAsSkill {
public:
    Xintan(): ZeroCardViewAsSkill("xintan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("burn").length() >= 2 && !player->hasUsed("XintanCard");
    }

    virtual const Card *viewAs() const{
        return new XintanCard;
    }
};

SPCardPackage::SPCardPackage()
    : Package("sp_cards")
{
    (new SPMoonSpear)->setParent(this);
    skills << new SPMoonSpearSkill;

    type = CardPack;
}

ADD_PACKAGE(SPCard)


ChaosPackage::ChaosPackage()
    : Package("Chaos")
{
    General *sunyang = new General(this, "sunyang", "wu", 4);
    sunyang->addSkill(new Shuijian);

    General *yeshiwen = new General(this, "yeshiwen", "wu", 3, false);
    yeshiwen->addSkill(new Jisu);
    yeshiwen->addSkill(new SlashNoDistanceLimitSkill("jisu"));
    yeshiwen->addSkill(new Shuiyong);
    related_skills.insertMulti("jisu", "#jisu-slash-ndl");

    General *hanba = new General(this, "hanba", "qun", 4, false);
    hanba->addSkill(new Fentian);
    hanba->addSkill(new Zhiri);
    hanba->addRelateSkill("xintan");

    addMetaObject<JisuCard>();
    addMetaObject<XintanCard>();

    skills << new Xintan;
}

ADD_PACKAGE(Chaos)


SPPackage::SPPackage()
    : Package("sp")
{
    General *yangxiu = new General(this, "yangxiu", "wei", 3); // SP 001
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);
    related_skills.insertMulti("jilei", "#jilei-clear");

    General *gongsunzan = new General(this, "gongsunzan", "qun"); // SP 003
    gongsunzan->addSkill(new Yicong);
    gongsunzan->addSkill(new YicongEffect);
    related_skills.insertMulti("yicong", "#yicong-effect");

    General *yuanshu = new General(this, "yuanshu", "qun"); // SP 004
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);


    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4); // SP 007

    sp_guanyu->addSkill(new Danji);


    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true); // SP 008 (2-2)

    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new ShenweiKeep);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Shenji);
    related_skills.insertMulti("shenwei", "#shenwei-draw");

    General *caohong = new General(this, "caohong", "wei"); // SP 013
    caohong->addSkill(new Yuanhu);

    General *guanyinping = new General(this, "guanyinping", "shu", 3, false); // SP 014
    guanyinping->addSkill(new Xueji);
    guanyinping->addSkill(new Huxiao);
    guanyinping->addSkill(new HuxiaoCount);
    guanyinping->addSkill(new HuxiaoClear);
    guanyinping->addSkill(new Wuji);
    guanyinping->addSkill(new WujiCount);
    related_skills.insertMulti("wuji", "#wuji-count");
    related_skills.insertMulti("huxiao", "#huxiao-count");
    related_skills.insertMulti("huxiao", "#huxiao-clear");

    General *xiahouba = new General(this, "xiahouba", "shu"); // SP 019
    xiahouba->addSkill(new Baobian);

    General *chenlin = new General(this, "chenlin", "wei", 3); // SP 020
    chenlin->addSkill(new Bifa);
    chenlin->addSkill(new Songci);

    General *erqiao = new General(this, "erqiao", "wu", 3, false); // SP 021
    erqiao->addSkill(new Xingwu);
    erqiao->addSkill(new Luoyan);


    General *xiahoushi = new General(this, "xiahoushi", "shu", 3, false); // SP 023
    xiahoushi->addSkill(new Yanyu);
    xiahoushi->addSkill(new Xiaode);
    xiahoushi->addSkill(new XiaodeEx);
    related_skills.insertMulti("xiaode", "#xiaode");


    General *zhangbao = new General(this, "zhangbao", "qun", 3); // SP 025
    zhangbao->addSkill(new Zhoufu);
    zhangbao->addSkill(new Yingbing);

    addMetaObject<WeidiCard>();
    addMetaObject<YuanhuCard>();
    addMetaObject<XuejiCard>();
    addMetaObject<BifaCard>();
    addMetaObject<SongciCard>();
    addMetaObject<ZhoufuCard>();
}

ADD_PACKAGE(SP)

OLPackage::OLPackage()
    : Package("OL")
{

    General *zhugeke = new General(this, "zhugeke", "wu", 3); // OL 002
    zhugeke->addSkill(new Aocai);
    zhugeke->addSkill(new Duwu);

    addMetaObject<AocaiCard>();
    addMetaObject<DuwuCard>();
}

ADD_PACKAGE(OL)
