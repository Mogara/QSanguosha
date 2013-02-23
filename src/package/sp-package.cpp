#include "sp-package.h"
#include "client.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "engine.h"
#include "maneuvering.h"
#include "wisdompackage.h"

class SPMoonSpearSkill: public WeaponSkill{
public:
    SPMoonSpearSkill():WeaponSkill("SPMoonSpear"){
        events << CardUsed << CardResponded;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(triggerEvent == CardUsed){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(triggerEvent == CardResponded){
            ResponsedStruct resp = data.value<ResponsedStruct>();
            card = resp.m_card;
        }

        if(card == NULL || !card->isBlack())
            return false;

        if(!room->askForSkillInvoke(player, objectName(), data))
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(player->inMyAttackRange(tmp))
                targets << tmp;
        }
        if(targets.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        if(!room->askForCard(target, "jink", "@moon-spear-jink", QVariant(), Card::MethodResponse, player)){
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

SPMoonSpear::SPMoonSpear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("SPMoonSpear");
    skill = new SPMoonSpearSkill;
}

class Jilei: public TriggerSkill {
public:
    Jilei():TriggerSkill("jilei"){
        events << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *current = room->getCurrent();
        if (!current || current->isDead())
            return false;

        if (damage.from == NULL)
           return false;

        if (room->askForSkillInvoke(yangxiu, objectName(), data)) {
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->broadcastSkillInvoke(objectName());

            room->setPlayerJilei(damage.from, choice);
            room->setPlayerFlag(damage.from, "jilei");

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
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
            if (player->hasFlag("jilei")) {
                room->setPlayerFlag(player, "-jilei");

                LogMessage log;
                log.type = "#JileiClear";
                log.from = player;
                room->sendLog(log);
                room->setPlayerJilei(player, "clear");
            }
        }

        return false;
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << TargetConfirmed << CardEffected;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.to.length() <= 1 || !use.to.contains(player) ||
               !use.card->isKindOf("TrickCard") ||
               !room->askForSkillInvoke(player, objectName(), data))
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


class Yongsi: public TriggerSkill{
public:
    Yongsi():TriggerSkill("yongsi"){
        events << DrawNCards << EventPhaseStart;
        frequency = Compulsory;
    }

    int getKingdoms(ServerPlayer *yuanshu) const{
        QSet<QString> kingdom_set;
        Room *room = yuanshu->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *yuanshu, QVariant &data) const{
        if(triggerEvent == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            log.arg2 = objectName();
            room->sendLog(log);

            room->broadcastSkillInvoke("yongsi", x);

        }else if(triggerEvent == EventPhaseStart && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            LogMessage log;
            log.type = yuanshu->getCardCount(true) > x ? "#YongsiBad" : "#YongsiWorst";
            log.from = yuanshu;
            log.arg = QString::number(log.type == "#YongsiBad" ? x : yuanshu->getCardCount(true));
            log.arg2 = objectName();
            room->sendLog(log);
            if (x > 0)
                room->askForDiscard(yuanshu, "yongsi", x, x, false, true);
        }

        return false;
    }
};

WeidiCard::WeidiCard(){
    target_fixed = true;
}

void WeidiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *yuanshu = card_use.from;

    QStringList choices;
    if(yuanshu->hasLordSkill("jijiang") && room->getLord()->hasLordSkill("jijiang") && Slash::IsAvailable(yuanshu))
        choices << "jijiang";

    if(yuanshu->hasLordSkill("weidai") && Analeptic::IsAvailable(yuanshu) && !yuanshu->hasFlag("drank"))
        choices << "weidai";

    if(choices.isEmpty())
        return;

    QString choice;
    if(choices.length() == 1)
        choice = choices.first();
    else
        choice = room->askForChoice(yuanshu, "weidi", choices.join("+"));

    if(choice == "jijiang"){
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer* target, room->getOtherPlayers(yuanshu)){
            if(yuanshu->canSlash(target))
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
    }else{
        WeidaiCard *weidai = new WeidaiCard;
        weidai->setSkillName("weidi");
        CardUseStruct use;
        use.card = weidai;
        use.from = yuanshu;
        room->useCard(use);
    }
}

class WeidiViewAsSkill:public ZeroCardViewAsSkill{
public:
    WeidiViewAsSkill():ZeroCardViewAsSkill("weidi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->hasLordSkill("jijiang") && Slash::IsAvailable(player))
                ||(player->hasLordSkill("weidai") && Analeptic::IsAvailable(player) && !player->hasFlag("drank"));
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->hasLordSkill("weidai"))
            return pattern == "peach+analeptic";
        
        return false;
    }

    virtual const Card *viewAs() const{
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
            return new WeidaiCard;
        else
            return new WeidiCard;
    }
};

class Weidi:public GameStartSkill{
public:
    Weidi():GameStartSkill("weidi"){
        frequency = Compulsory;
        view_as_skill = new WeidiViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *) const{
        // do nothing
        return;
    }
};

class Yicong: public DistanceSkill{
public:
    Yicong():DistanceSkill("yicong"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()) && from->getHp() > 2)
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() <= 2)
            correct ++;

        return correct;
    }
};

class YicongEffect: public TriggerSkill{
public:
    YicongEffect():TriggerSkill("#yicong_effect"){
        events << PostHpReduced << HpRecover;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        int hp = player->getHp();
        int index = 0;
        if (triggerEvent == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if (hp > 2 && hp - recover.recover <= 2)
                index = 1;
        }
        else if (triggerEvent == PostHpReduced){
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

YuanhuCard::YuanhuCard(){
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
    else if (card_use.to.first()->isCaoCao())
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
    log.card_str = card->getEffectIdString();
    room->sendLog(log);

    if (card->isKindOf("Weapon")) {
      QList<ServerPlayer *> targets;
      foreach (ServerPlayer *p, room->getAllPlayers()) {
          if (effect.to->distanceTo(p) == 1 && !p->isAllNude())
              targets << p;
      }
      if (!targets.isEmpty()) {
          ServerPlayer *to_dismantle = room->askForPlayerChosen(caohong, targets, "yuanhu");
          int card_id = room->askForCardChosen(caohong, to_dismantle, "hej", "yuanhu");
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
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yuanhu";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("EquipCard");
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
        if (target->getPhase() == Player::Finish && !target->isNude()) {
            room->askForUseCard(target, "@@yuanhu", "@yuanhu-equip", -1, Card::MethodNone);
        }
        return false;
    }
};

class Xiuluo: public PhaseChangeSkill{
public:
    Xiuluo():PhaseChangeSkill("xiuluo"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && !target->isKongcheng()
                && !target->getJudgingArea().isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        bool once_success = false;
        do{
            once_success = false;

            if(!target->askForSkillInvoke(objectName()))
                return false;

            Room *room = target->getRoom();
            int card_id = room->askForCardChosen(target, target, "j", objectName());
            const Card *card = Sanguosha->getCard(card_id);

            QString suit_str = card->getSuitString();
            QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
            QString prompt = QString("@xiuluo:::%1").arg(suit_str);
            if(room->askForCard(target, pattern, prompt)){
                room->broadcastSkillInvoke(objectName());
                room->throwCard(card, NULL);
                once_success = true;
            }
        }while(!target->getCards("j").isEmpty() && once_success);

        return false;
    }
};

class Shenwei: public DrawCardsSkill{
public:
    Shenwei():DrawCardsSkill("#shenwei-draw"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        player->getRoom()->broadcastSkillInvoke("shenwei");
        return n + 2;
    }
};

class ShenweiKeep: public MaxCardsSkill{
public:
    ShenweiKeep():MaxCardsSkill("shenwei"){
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return 2;
        else
            return 0;
    }
};

class Danji: public PhaseChangeSkill{
public:
    Danji():PhaseChangeSkill("danji"){
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
        if(the_lord && the_lord->isCaoCao()){
            LogMessage log;
            log.type = "#DanjiWake";
            log.from = guanyu;
            log.arg = QString::number(guanyu->getHandcardNum());
            log.arg2 = QString::number(guanyu->getHp());
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->broadcastInvoke("animate", "lightbox:$DanjiAnimate:5000");
            room->getThread()->delay(5000);

            guanyu->setMark("danji", 1);
            guanyu->gainMark("@waked");
            room->loseMaxHp(guanyu);
            room->acquireSkill(guanyu, "mashu");
        }

        return false;
    }
};

XuejiCard::XuejiCard(){
}

bool XuejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= Self->getLostHp())
        return false;

    if (to_select == Self)
        return false;

    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getEffectiveId() == getEffectiveId()) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - 1;
    } else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getEffectiveId() == getEffectiveId())
        range_fix += 1;

    return Self->distanceTo(to_select, range_fix) <= Self->getAttackRange();
}

void XuejiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    DamageStruct damage;
    damage.from = source;

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
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getLostHp() > 0 && !player->hasUsed("XuejiCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isRed() && !Self->isJilei(to_select);
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
    
    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == SlashMissed) {
            if (player->getPhase() == Player::Play)
                room->setPlayerMark(player, "huxiao", player->getMark("huxiao") + 1);
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play)
                if (player->getMark("huxiao") > 0)
                    room->setPlayerMark(player, "huxiao", 0);
        }

        return false;
    }
};

class HuxiaoRemove: public TriggerSkill {
public:
    HuxiaoRemove(): TriggerSkill("#huxiao") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        if(data.toString() == "huxiao")
            room->setPlayerMark(player, "huxiao", 0);
        return false;
    }
};

class WujiCount: public TriggerSkill {
public:
    WujiCount(): TriggerSkill("#wuji-count") {
        events << DamageDone << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from->isAlive() && damage.from == room->getCurrent() && damage.from->getMark("wuji") == 0)
                room->setPlayerMark(damage.from, "wuji_damage", damage.from->getMark("wuji_damage") + damage.damage);
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
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Finish
                && target->getMark("wuji") == 0
                && target->getMark("wuji_damage") >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#WujiWake";
        log.from = player;
        log.arg = QString::number(player->getMark("wuji_damage"));
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->broadcastInvoke("animate", "lightbox:$WujiAnimate:4000");
        room->getThread()->delay(4000);

        player->addMark("wuji");
        player->gainMark("@waked");

        room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 1);

        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);

        room->detachSkillFromPlayer(player, "huxiao");

        return false;
    }
};

class Baobian: public TriggerSkill {
public:
    Baobian(): TriggerSkill("baobian") {
        events << GameStart << HpChanged << MaxHpChanged << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }

    static void BaobianChange(Room *room, ServerPlayer *player, int hp, const QString &skill_name) {
        QStringList baobian_skills = player->tag["BaobianSkills"].toStringList();
        if (player->getHp() <= hp) {
            if (!baobian_skills.contains(skill_name)) {
                room->acquireSkill(player, skill_name);
                baobian_skills << skill_name;
            }
        } else {
            room->detachSkillFromPlayer(player, skill_name);
            baobian_skills.removeOne(skill_name);
        }
        player->tag["BaobianSkills"] = QVariant::fromValue(baobian_skills);
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventLoseSkill) {
            if (data.toString() == objectName()) {
                QStringList baobian_skills = player->tag["BaobianSkills"].toStringList();
                foreach (QString skill_name, baobian_skills)
                    room->detachSkillFromPlayer(player, skill_name);
                player->tag["BaobianSkills"] = QVariant();
            }
            return false;
        }

        if (!TriggerSkill::triggerable(player)) return false;

        BaobianChange(room, player, 1, "shensu");
        BaobianChange(room, player, 2, "paoxiao");
        BaobianChange(room, player, 3, "tiaoxin");
        return false;
    }
};

class Jieyuan: public TriggerSkill {
public:
    Jieyuan(): TriggerSkill("jieyuan"){
        events << DamageCaused << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(triggerEvent == DamageCaused){
            if(damage.to && damage.to->isAlive()
               && damage.to->getHp() >= player->getHp() && damage.to != player && !player->isKongcheng())
                if(room->askForCard(player, ".black", "@JieyuanIncrease", data, objectName())){
                    room->broadcastSkillInvoke(objectName(), 1);

                    LogMessage log;
                    log.type = "#JieyuanIncrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(++damage.damage);
                    room->sendLog(log);

                    data = QVariant::fromValue(damage);
                }
        }else if(triggerEvent == DamageInflicted){
            if(damage.from && damage.from->isAlive()
               && damage.from->getHp() >= player->getHp() && damage.from != player && !player->isKongcheng())
                if(room->askForCard(player, ".red", "@JieyuanDecrease", data, objectName())){
                    room->broadcastSkillInvoke(objectName(), 2);

                    LogMessage log;
                    log.type = "#JieyuanDecrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(--damage.damage);
                    room->sendLog(log);

                    if (damage.damage < 1){
                        LogMessage log;
                        log.type = "#ZeroDamage";
                        log.from = damage.from;
                        log.to << player;
                        room->sendLog(log);
                        return true;
                    }
                    data = QVariant::fromValue(damage);
                }
        }

        return false;
    }
};

class Fenxin: public TriggerSkill{
public:
    Fenxin(): TriggerSkill("fenxin") {
        events << BeforeGameOverJudge;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        if (!room->getMode().endsWith("p") && !room->getMode().endsWith("pd") && !room->getMode().endsWith("pz"))
            return false;
        DamageStar damage = data.value<DamageStar>();
        if (damage == NULL)
            return false;
        ServerPlayer *killer = damage->from;
        if (killer == NULL || killer->isLord() || player->isLord() || player->getHp() > 0)
            return false;
        if (!TriggerSkill::triggerable(killer) || killer->getMark("@burnheart") == 0)
            return false;
        room->setPlayerFlag(player, "FenxinTarget");
        if (room->askForSkillInvoke(killer, objectName(), QVariant::fromValue(player))) {
            room->broadcastSkillInvoke(objectName());
            room->broadcastInvoke("animate", "lightbox:$FenxinAnimate");
            room->getThread()->delay(1500);
            killer->loseMark("@burnheart");
            QString role1 = killer->getRole();
            killer->setRole(player->getRole());
            room->setPlayerProperty(killer, "role", player->getRole());
            player->setRole(role1);
            room->setPlayerProperty(player, "role", role1);
        }
        room->setPlayerFlag(player, "-FenxinTarget");
        return false;
    }
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
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@bifa";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        Card *card = new BifaCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class Bifa: public TriggerSkill {
public:
    Bifa() : TriggerSkill("bifa") {
        events << EventPhaseStart;
        view_as_skill = new BifaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish && !player->isKongcheng()) {
            room->askForUseCard(player, "@@bifa", "@bifa-remove", -1, Card::MethodNone);
        } else if (player->getPhase() == Player::RoundStart && player->getPile("bifa").length() > 0) {
            QList<int> bifa_list = player->getPile("bifa");

            while (!bifa_list.isEmpty()) {
                int card_id = bifa_list.last();
                ServerPlayer *chenlin = player->tag["BifaSource" + QString::number(card_id)].value<PlayerStar>();
                QList<int> ids;
                ids << card_id;
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
                    room->broadcastSkillInvoke(objectName(), 3);
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                    room->throwCard(cd, reason, NULL);
                    room->loseHp(player);
                }
                bifa_list.removeOne(card_id);
                player->invoke("clearAG");
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
    return targets.isEmpty() && to_select->getMark("@songci") == 0 && to_select->getHandcardNum() != to_select->getHp();
}

void SongciCard::onEffect(const CardEffectStruct &effect) const{
    int handcard_num = effect.to->getHandcardNum();
    int hp = effect.to->getHp();
    effect.to->gainMark("@songci");
    if (handcard_num > hp) {
        effect.to->getRoom()->broadcastSkillInvoke("songci", 2);
        effect.to->getRoom()->askForDiscard(effect.to, "songci", 2, 2, false, true);
    } else if (handcard_num < hp) {
        effect.to->getRoom()->broadcastSkillInvoke("songci", 1);
        effect.to->drawCards(2, "songci");
    }
}

class SongciViewAsSkill: public ZeroCardViewAsSkill{
public:
    SongciViewAsSkill():ZeroCardViewAsSkill("songci"){
    }

    virtual const Card *viewAs() const{
        return new SongciCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (player->getMark("@songci") == 0 && player->getHandcardNum() != player->getHp()) return true;
        foreach (const Player *sib, player->getSiblings())
            if (sib->getMark("@songci") == 0 && sib->getHandcardNum() != sib->getHp())
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

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player) return false;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->getMark("@songci") > 0)
                room->setPlayerMark(p, "@songci", 0);
        return false;
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

SPCardPackage::SPCardPackage()
    :Package("sp_cards")
{
    (new SPMoonSpear)->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(SPCard)

SPPackage::SPPackage()
    :Package("sp")
{
    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);
    related_skills.insertMulti("jilei", "#jilei-clear");

    General *sp_diaochan = new General(this, "sp_diaochan", "qun", 3, false, true);
    sp_diaochan->addSkill("lijian");
    sp_diaochan->addSkill("biyue");

    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Yicong);
    gongsunzan->addSkill(new YicongEffect);
    related_skills.insertMulti("yicong", "#yicong_effect");

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);
    yuanshu->addSkill(new SPConvertSkill("yuanshu", "tw_yuanshu"));

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");

    General *sp_pangde = new General(this, "sp_pangde", "wei", 4, true, true);
    sp_pangde->addSkill("mengjin");
    sp_pangde->addSkill("mashu");

    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4);
    sp_guanyu->addSkill("wusheng");
    sp_guanyu->addSkill(new Danji);

    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("mashu");
    shenlvbu1->addSkill("wushuang");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("mashu");
    shenlvbu2->addSkill("wushuang");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new ShenweiKeep);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Shenji);
    related_skills.insertMulti("shenwei", "#shenwei-draw");

    General *sp_caiwenji = new General(this, "sp_caiwenji", "wei", 3, false, true);
    sp_caiwenji->addSkill("beige");
    sp_caiwenji->addSkill("duanchang");

    General *sp_machao = new General(this, "sp_machao", "qun", 4, true, true);
    sp_machao->addSkill("mashu");
    sp_machao->addSkill("tieji");

    General *sp_jiaxu = new General(this, "sp_jiaxu", "wei", 3, true, true);
    sp_jiaxu->addSkill("wansha");
    sp_jiaxu->addSkill("luanwu");
    sp_jiaxu->addSkill("weimu");
    sp_jiaxu->addSkill("#@chaos-1");

    General *caohong = new General(this, "caohong", "wei");
    caohong->addSkill(new Yuanhu);

    General *guanyinping = new General(this, "guanyinping", "shu", 3, false);
    guanyinping->addSkill(new Xueji);
    guanyinping->addSkill(new Huxiao);
    guanyinping->addSkill(new HuxiaoRemove);
    guanyinping->addSkill(new HuxiaoCount);
    guanyinping->addSkill(new Wuji);
    guanyinping->addSkill(new WujiCount);
    related_skills.insertMulti("wuji", "#wuji-count");
    related_skills.insertMulti("huxiao", "#huxiao");
    related_skills.insertMulti("huxiao", "#huxiao-count");

    General *sp_zhenji = new General(this, "sp_zhenji", "wei", 3, false, true);
    sp_zhenji->addSkill("qingguo");
    sp_zhenji->addSkill("luoshen");

    General *lingju = new General(this, "lingju", "qun", 3, false);
    lingju->addSkill(new Jieyuan);
    lingju->addSkill(new Fenxin);
    lingju->addSkill(new MarkAssignSkill("@burnheart", 1));
    related_skills.insertMulti("fenxin", "#@burnheart-1");
    lingju->addSkill(new SPConvertSkill("lingju", "ass_lingju"));

    General *xiahouba = new General(this, "xiahouba", "shu");
    xiahouba->addSkill(new Baobian);

    General *chenlin = new General(this, "chenlin", "wei", 3);
    chenlin->addSkill(new Bifa);
    chenlin->addSkill(new Songci);

    General *tw_diaochan = new General(this, "tw_diaochan", "qun", 3, false, true);
    tw_diaochan->addSkill("lijian");
    tw_diaochan->addSkill("biyue");

    General *tw_yuanshu = new General(this, "tw_yuanshu", "qun", 4, true, true);
    tw_yuanshu->addSkill("yongsi");
    tw_yuanshu->addSkill("weidi");

    General *tw_daqiao = new General(this, "tw_daqiao", "wu", 3, false, true);
    tw_daqiao->addSkill("guose");
    tw_daqiao->addSkill("liuli");

    General *tw_zhaoyun = new General(this, "tw_zhaoyun", "shu", 4, true, true);
    tw_zhaoyun->addSkill("longdan");

    General *tw_zhenji = new General(this, "tw_zhenji", "wei", 3, false, true);
    tw_zhenji->addSkill("qingguo");
    tw_zhenji->addSkill("luoshen");

    General *tw_lvbu = new General(this, "tw_lvbu", "qun", 4, true, true);
    tw_lvbu->addSkill("wushuang");

    General *tw_ganning = new General(this, "tw_ganning", "wu", 4, true, true);
    tw_ganning->addSkill("qixi");

    General *tw_machao = new General(this, "tw_machao", "shu", 4, true, true);
    tw_machao->addSkill("mashu");
    tw_machao->addSkill("tieji");

    General *wz_daqiao = new General(this, "wz_daqiao", "wu", 3, false, true);
    wz_daqiao->addSkill("guose");
    wz_daqiao->addSkill("liuli");

    General *wz_xiaoqiao = new General(this, "wz_xiaoqiao", "wu", 3, false, true);
    wz_xiaoqiao->addSkill("tianxiang");
    wz_xiaoqiao->addSkill("#tianxiang");
    wz_xiaoqiao->addSkill("hongyan");
    
    addMetaObject<WeidiCard>();
    addMetaObject<YuanhuCard>();
    addMetaObject<XuejiCard>();
    addMetaObject<BifaCard>();
    addMetaObject<SongciCard>();
}

ADD_PACKAGE(SP)

HegemonySPPackage::HegemonySPPackage()
    : Package("hegemony_sp")
{
    General *sp_heg_zhouyu = new General(this, "sp_heg_zhouyu", "wu", 3, true, true);
    sp_heg_zhouyu->addSkill("yingzi");
    sp_heg_zhouyu->addSkill("fanjian");
}

ADD_PACKAGE(HegemonySP)
