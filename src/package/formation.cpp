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

class Tuntian: public TriggerSkill {
public:
    Tuntian(): TriggerSkill("tuntian") {
        events << CardsMoveOneTime << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::NotActive) return false;
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && (move.from_places.contains(Player::PlaceHand) || move.from_places.contains(Player::PlaceEquip))
                && !(move.to == player && (move.to_place == Player::PlaceHand || move.to_place == Player::PlaceEquip)))
                return true;
        } else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason == "tuntian" && judge->isGood())
                player->addToPile("field", judge->card->getEffectiveId());
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->askForSkillInvoke("tuntian", data);
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        room->broadcastSkillInvoke("tuntian");
        JudgeStruct judge;
        judge.pattern = ".|heart";
        judge.good = false;
        judge.reason = "tuntian";
        judge.who = player;
        room->judge(judge);
       
        return false;
    }
};

class TuntianDistance: public DistanceSkill {
public:
    TuntianDistance(): DistanceSkill("#tuntian-dist") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasShownSkill(Sanguosha->getSkill("tuntian")))
            return -from->getPile("field").length();
        else
            return 0;
    }
};

JixiCard::JixiCard() {
    target_fixed = true;
}

void JixiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *dengai = card_use.from;

    QList<int> fields;
    QList<int> total = dengai->getPile("field");
    foreach (int id, total) {
        Snatch *snatch = new Snatch(Card::SuitToBeDecided, -1);
        snatch->addSubcard(id);
        if (!snatch->isAvailable(dengai))
            continue;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (!snatch->targetFilter(QList<const Player *>(), p, dengai))
                continue;
            if (dengai->isProhibited(p, snatch))
                continue;
            fields << id;
            break;
        }
        delete snatch;
        snatch = NULL;
    }

    if (fields.isEmpty())
        return;

    QList<int> disabled;
    foreach (int id, total) {
        if (!fields.contains(id))
            disabled << id;
    }

    int card_id;
    if (fields.length() == 1)
        card_id = fields.first();
    else {
        room->fillAG(total, dengai, disabled);
        card_id = room->askForAG(dengai, fields, false, "jixi");
        room->clearAG(dengai);

        if (card_id == -1)
            return;
    }

    Snatch *snatch = new Snatch(Card::SuitToBeDecided, -1);
    snatch->setSkillName("jixi");
    snatch->addSubcard(card_id);

    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (!snatch->targetFilter(QList<const Player *>(), p, dengai))
            continue;
        if (dengai->isProhibited(p, snatch))
            continue;

        targets << p;
    }
    if (targets.isEmpty())
        return;

    room->setPlayerProperty(dengai, "jixi_snatch", snatch->toString());

    CardUseStruct use;
    use.card = snatch;
    use.from = dengai;

    if (room->askForUseCard(dengai, "@@jixi!", "@jixi-target")) {
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->hasFlag("JixiSnatchTarget")) {
                room->setPlayerFlag(p, "-JixiSnatchTarget");
                use.to << p;
            }
        }
    } else {
        use.to << targets.at(qrand() % targets.length());
    }
    room->setPlayerProperty(dengai, "jixi_snatch", QString());
    room->useCard(use);
}

JixiSnatchCard::JixiSnatchCard() {
}

bool JixiSnatchCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    const Card *card = Card::Parse(Self->property("jixi_snatch").toString());
    if (card == NULL)
        return false;
    else {
        const Snatch *snatch = qobject_cast<const Snatch *>(card);
        return !Self->isProhibited(to_select, snatch, targets) && snatch->targetFilter(targets, to_select, Self);
    }
}

void JixiSnatchCard::onUse(Room *room, const CardUseStruct &card_use) const{
    foreach (ServerPlayer *to, card_use.to)
        room->setPlayerFlag(to, "JixiSnatchTarget");
}

class Jixi: public ZeroCardViewAsSkill {
public:
    Jixi(): ZeroCardViewAsSkill("jixi") {
        relate_to_place = "head";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (player->getPile("field").isEmpty())
            return false;
        foreach (int id, player->getPile("field")) {
            Snatch *snatch = new Snatch(Card::SuitToBeDecided, -1);
            snatch->setSkillName("jixi");
            snatch->addSubcard(id);
            snatch->deleteLater();
            if (!snatch->isAvailable(player))
                continue;
            foreach (const Player *p, player->getAliveSiblings()) {
                if (!snatch->targetFilter(QList<const Player *>(), p, player))
                    continue;
                if (player->isProhibited(p, snatch))
                    continue;
                return true;
            }
        }
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jixi!";
    }

    virtual const Card *viewAs() const{
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "@@jixi!")
            return new JixiSnatchCard;
        else
            return new JixiCard;
    }
};

class Ziliang: public TriggerSkill {
public:
    Ziliang(): TriggerSkill("ziliang") {
        events << Damaged;
        relate_to_place = "deputy";
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (player == NULL) return false;
        ServerPlayer *dengai = room->findPlayerBySkillName(objectName());
        if (!player->isAlive()) return false;
        if (!dengai || dengai->getPile("field").isEmpty()) return false;
        ask_who = dengai;
        return true;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *dengai = room->findPlayerBySkillName(objectName());
        return room->askForSkillInvoke(dengai, objectName(), data);
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *dengai = room->findPlayerBySkillName(objectName());
        
        int id = room->askForAG(dengai, dengai->getPile("field"), false, objectName());
        if (player == dengai) {
            LogMessage log;
            log.type = "$MoveCard";
            log.from = player;
            log.to << player;
            log.card_str = QString::number(id);
            room->sendLog(log);
        }
        room->obtainCard(player, id);

        return false;
    }
}; 

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
        if (effect.to->distanceTo(p) == 1 && caohong->canDiscard(p, "he"))
            targets << p;
    }
    if (!targets.isEmpty()) {
        ServerPlayer *to_dismantle = room->askForPlayerChosen(caohong, targets, "huyuan", "@huyuan-discard:" + effect.to->objectName());
        int card_id = room->askForCardChosen(caohong, to_dismantle, "he", "huyuan", false, Card::MethodDiscard);
        room->throwCard(Sanguosha->getCard(card_id), to_dismantle, caohong);
    }
}

class HuyuanViewAsSkill: public OneCardViewAsSkill {
public:
    HuyuanViewAsSkill(): OneCardViewAsSkill("huyuan") {
        response_pattern = "@@huyuan";
        filter_pattern = "EquipCard";
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        HuyuanCard *first = new HuyuanCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        first->setShowSkill(objectName());
        return first;
    }
};

class Huyuan: public PhaseChangeSkill {
public:
    Huyuan(): PhaseChangeSkill("huyuan") {
        view_as_skill = new HuyuanViewAsSkill;
    }

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *target, QVariant &data, ServerPlayer* &ask_who) const {
        if (!PhaseChangeSkill::triggerable(target)) return false;
        return (target->getPhase() == Player::Finish && !target->isNude());
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *target, QVariant &data) const {
        return room->askForUseCard(target, "@@huyuan", "@huyuan-equip", -1, Card::MethodNone);
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        return false;
    }
}; 

class Heyi: public TriggerSkill {
public:
    Heyi(): TriggerSkill("heyi") {
        events << GeneralShown << GeneralHidden << Death;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (player == NULL) return false;
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->hasSkill(objectName())) {
                foreach (ServerPlayer *p, room->getAllPlayers())
                    if (p->hasSkill("feiying"))
                        room->detachSkillFromPlayer(p, "feiying", true);
                return false;
            }
        }
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->hasSkill("feiying"))
                room->detachSkillFromPlayer(p, "feiying", true);

        if (room->alivePlayerCount() < 4) return false;
        ServerPlayer *caohong = room->findPlayerBySkillName(objectName());
        if (!caohong) return false;
        QList<ServerPlayer *> teammates;
        bool next = true;
        ServerPlayer *next_p = caohong;
        while (next) {
            next_p = next_p->getNextAlive();
            if (next_p->isFriendWith(caohong) && !teammates.contains(next_p))
                teammates << next_p;
            else
                next = false;
        }
        next = true;
        next_p = caohong;
        while (next) {
            foreach (ServerPlayer *p, room->getAllPlayers())
                if (p->getNextAlive() == next_p) {
                    next_p = p;
                    break;
                }
            if (next_p->isFriendWith(caohong) && !teammates.contains(next_p))
                teammates << next_p;
            else
                next = false;
        }
        if (teammates.isEmpty()) return false;
        foreach(ServerPlayer *p, teammates)
            room->acquireSkill(p, "feiying", false);

        return false;
    }
};

class Feiying: public DistanceSkill {
public:
    Feiying(): DistanceSkill("feiying") {
    }

    virtual int getCorrect(const Player *, const Player *to) const{
        if (to->hasSkill(objectName()))
            return 1;
        else
            return 0;
    }
};

TiaoxinCard::TiaoxinCard() {
}

bool TiaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->inMyAttackRange(Self) && to_select != Self;
}

void TiaoxinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool use_slash = false;
    if (effect.to->canSlash(effect.from, NULL, false))
        use_slash = room->askForUseSlashTo(effect.to, effect.from, "@tiaoxin-slash:" + effect.from->objectName());
    if (!use_slash && effect.from->canDiscard(effect.to, "he"))
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "tiaoxin", false, Card::MethodDiscard), effect.to, effect.from);
}

class Tiaoxin: public ZeroCardViewAsSkill {
public:
    Tiaoxin(): ZeroCardViewAsSkill("tiaoxin") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TiaoxinCard");
    }

    virtual const Card *viewAs() const{
        Card *card = new TiaoxinCard;
        card->setShowSkill(objectName());
        return card;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        int index = qrand() % 2 + 1;
        if (!player->hasInnateSkill(objectName()) && player->hasSkill("baobian"))
            index += 2;
        return index;
    }
};

class YiZhi: public TriggerSkill {
public:
    YiZhi(): TriggerSkill("yizhi") {
        relate_to_place = "deputy";
        frequency = Compulsory;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        return false;
    }
};

class Tianfu: public TriggerSkill {
public:
    Tianfu(): TriggerSkill("tianfu") {
        events << EventPhaseStart << Death;
        relate_to_place = "head";
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (player == NULL) return false;
        
        ServerPlayer *jiangwei = room->findPlayerBySkillName(objectName());
        if (!jiangwei) return false;
        if (jiangwei->hasSkill("kanpo") && !jiangwei->ownSkill("kanpo"))
            room->detachSkillFromPlayer(jiangwei, "kanpo", true);

        if (room->alivePlayerCount() < 4 || !jiangwei->hasShownSkill(this)) return false;
        QList<ServerPlayer *> teammates;
        bool next = true;
        ServerPlayer *next_p = jiangwei;
        while (next) {
            next_p = next_p->getNextAlive();
            if (next_p->isFriendWith(jiangwei) && !teammates.contains(next_p))
                teammates << next_p;
            else
                next = false;
        }
        next = true;
        next_p = jiangwei;
        while (next) {
            foreach (ServerPlayer *p, room->getAllPlayers())
                if (p->getNextAlive() == next_p) {
                    next_p = p;
                    break;
                }
            if (next_p->isFriendWith(jiangwei) && !teammates.contains(next_p))
                teammates << next_p;
            else
                next = false;
        }
        teammates << jiangwei;
        if (teammates.isEmpty()) return false;
        foreach(ServerPlayer *p, teammates)
            if (p->getPhase() != Player::NotActive) {
                room->attachSkillToPlayer(jiangwei, "kanpo");
                break;
            }

        return false;
    }
};

class Yicheng: public TriggerSkill {
public:
    Yicheng(): TriggerSkill("yicheng") {
        events << TargetConfirmed;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash")) return false;
        foreach (ServerPlayer *p, use.to) {
            if (room->askForSkillInvoke(player, objectName(), QVariant::fromValue(p))) {
                p->drawCards(1);
                if (p->isAlive() && p->canDiscard(p, "he"))
                    room->askForDiscard(p, objectName(), 1, 1, false, true);
            }
            if (!player->isAlive())
                break;
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

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damaged && player->isAlive()) {
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
            if (!use.card || use.card->getTypeId() == Card::TypeEquip || use.card->getTypeId() == Card::TypeSkill)
                return false;
            if (use.to.length() != 1) return false;
            ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
            if (!yuji || yuji->getPile("sorcery").isEmpty()) return false;
            if (room->askForSkillInvoke(yuji, objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                room->notifySkillInvoked(yuji, objectName());
                if (yuji == player || room->askForChoice(player, objectName(), "accept+reject", data) == "accept") {
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


class Zhendu: public TriggerSkill {
public:
    Zhendu(): TriggerSkill("zhendu") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() != Player::Play)
            return false;
        QList<ServerPlayer *> hetaihous = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *hetaihou, hetaihous) {
            if (!hetaihou->isAlive() || !hetaihou->canDiscard(hetaihou, "h") || hetaihou->getPhase() == Player::Play)
                continue;
            if (room->askForCard(hetaihou, ".", "@zhendu-discard", QVariant(), objectName())) {
                Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
                analeptic->setSkillName("_zhendu");
                room->useCard(CardUseStruct(analeptic, player, QList<ServerPlayer *>(), true));
                if (player->isAlive())
                    room->damage(DamageStruct(objectName(), hetaihou, player));
            }
        }
        return false;
    }
};

class Qiluan: public TriggerSkill {
public:
    Qiluan(): TriggerSkill("qiluan") {
        events << Death << EventPhaseStart;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;

            ServerPlayer *current = room->getCurrent();
            if (current && (current->isAlive() || death.who == current) && current->getPhase() != Player::NotActive){
                foreach(ServerPlayer *p, room->getAllPlayers())
                    if (TriggerSkill::triggerable(p))
                        room->setPlayerMark(p, objectName(), 1);
            }

        } else {
            if (player->getPhase() == Player::NotActive) {
                QList<ServerPlayer *> hetaihous;
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->getMark(objectName()) > 0 && TriggerSkill::triggerable(p))
                        hetaihous << p;
                    room->setPlayerMark(p, objectName(), 0);
                }

                foreach (ServerPlayer *p, hetaihous) {
                    if (p->isAlive() && room->askForSkillInvoke(p, objectName())){
                        QStringList kingdoms;
                        ServerPlayer *lord = room->getLord();
                        foreach (ServerPlayer *other, room->getAlivePlayers()){
                            if (!kingdoms.contains(other->getKingdom()) && other->getKingdom() != lord->getKingdom())
                                kingdoms << other->getKingdom();
                        }
                        p->drawCards(kingdoms.length());
                    }
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (!TriggerSkill::triggerable(player)) return false;
        if (triggerEvent == EventPhaseEnd) {
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Play) {
                if (!player->hasFlag("ShengxiDamageInPlayPhase"))
                    return true;
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

    virtual bool cost(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        player->drawCards(2);
        
        return false;
    }
};

class Shoucheng: public TriggerSkill {
public:
    Shoucheng(): TriggerSkill("shoucheng") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }
    
    virtual bool triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (!TriggerSkill::triggerable(player)) return false;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from && move.from->isAlive() && move.from->getPhase() == Player::NotActive && move.from->isFriendWith(player)
                && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard)
            return true;

        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        room->broadcastSkillInvoke(objectName());
        ServerPlayer *from = (ServerPlayer *)move.from;
        from->drawCards(1);
        return false;
    }
};


FormationPackage::FormationPackage()
    : Package("formation")
{
    General *dengai = new General(this, "dengai", "wei"); // WEI 015
    dengai->addSkill(new Tuntian);
    dengai->addSkill(new TuntianDistance);
    dengai->addSkill(new Jixi);
    dengai->setHeadMaxHpAdjustedValue(-1);
    dengai->addSkill(new Ziliang); 
    related_skills.insertMulti("tuntian", "#tuntian-dist");

    General *caohong = new General(this, "caohong", "wei"); // WEI 018
    caohong->addSkill(new Huyuan);
    caohong->addSkill(new Heyi);

    General *jiangwei = new General(this, "jiangwei", "shu"); // SHU 012 G
    jiangwei->addSkill(new Tiaoxin);
    jiangwei->addSkill(new YiZhi);
    jiangwei->setDeputyMaxHpAdjustedValue(-1);
    jiangwei->addSkill(new Tianfu);

    General *jiangwanfeiyi = new General(this, "jiangwanfeiyi", "shu", 3); // SHU 018 
    jiangwanfeiyi->addSkill(new Shengxi);
    jiangwanfeiyi->addSkill(new Shoucheng);

    //ToDo: Jiang Qin(I don't understand the skill)

    General *xusheng = new General(this, "xusheng", "wu"); // WU 020
    xusheng->addSkill(new Yicheng);

    General *yuji = new General(this, "yuji", "qun", 3); // QUN 011 G
    yuji->addSkill(new Qianhuan);

    General *hetaihou = new General(this, "hetaihou", "qun", 3, false); // QUN 020
    hetaihou->addSkill(new Zhendu);
    hetaihou->addSkill(new Qiluan); 
    
    addMetaObject<JixiCard>();
    addMetaObject<JixiSnatchCard>();
    addMetaObject<HuyuanCard>();
    addMetaObject<TiaoxinCard>();

    skills << new Feiying;
}

ADD_PACKAGE(Formation)