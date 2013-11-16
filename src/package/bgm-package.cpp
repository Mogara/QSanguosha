#include "bgm-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "engine.h"
#include "settings.h"

class Chongzhen: public TriggerSkill {
public:
    Chongzhen(): TriggerSkill("chongzhen") {
        events << CardResponded << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardResponded) {
            CardResponseStruct resp = data.value<CardResponseStruct>();
            if (resp.m_card->getSkillName() == "longdan"
                && resp.m_who != NULL && !resp.m_who->isKongcheng()) {
                QVariant data = QVariant::fromValue((PlayerStar)resp.m_who);
                if (player->askForSkillInvoke(objectName(), data)) {
                    room->broadcastSkillInvoke("chongzhen", 1);
                    int card_id = room->askForCardChosen(player, resp.m_who, "h", objectName());
                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                    room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                }
            }
        } else {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from == player && use.card->getSkillName() == "longdan") {
                foreach (ServerPlayer *p, use.to) {
                    if (p->isKongcheng()) continue;
                    QVariant data = QVariant::fromValue((PlayerStar)p);
                    p->setFlags("ChongzhenTarget");
                    bool invoke = player->askForSkillInvoke(objectName(), data);
                    p->setFlags("-ChongzhenTarget");
                    if (invoke) {
                        room->broadcastSkillInvoke("chongzhen", 2);
                        int card_id = room->askForCardChosen(player, p, "h", objectName());
                        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                        room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                    }
                }
            }
        }
        return false;
    }
};

LihunCard::LihunCard() {
    mute = true;
}

bool LihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isMale() && to_select != Self;
}

void LihunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->turnOver();
    room->broadcastSkillInvoke("lihun", 1);

    DummyCard *dummy_card = new DummyCard(effect.to->handCards());
    if (!effect.to->isKongcheng()) {
        CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, effect.from->objectName(),
                              effect.to->objectName(), "lihun", QString());
        room->moveCardTo(dummy_card, effect.to, effect.from, Player::PlaceHand, reason, false);
    }
    effect.to->setFlags("LihunTarget");
    delete dummy_card;
}

class LihunSelect: public OneCardViewAsSkill {
public:
    LihunSelect(): OneCardViewAsSkill("lihun") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("LihunCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new LihunCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Lihun: public TriggerSkill {
public:
    Lihun(): TriggerSkill("lihun") {
        events << EventPhaseStart << EventPhaseEnd;
        view_as_skill = new LihunSelect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasUsed("LihunCard");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *diaochan, QVariant &) const{
        if (triggerEvent == EventPhaseEnd && diaochan->getPhase() == Player::Play) {
            ServerPlayer *target = NULL;
            foreach (ServerPlayer *other, room->getOtherPlayers(diaochan)) {
                if (other->hasFlag("LihunTarget")) {
                    other->setFlags("-LihunTarget");
                    target = other;
                    break;
                }
            }

            if (!target || target->getHp() < 1 || diaochan->isNude())
                return false;

            if (target->getGeneralName().contains("lvbu"))
                room->broadcastSkillInvoke(objectName(), 3);
            else if (target->getGeneralName().contains("dongzhuo"))
                room->broadcastSkillInvoke(objectName(), 4);
            else
                room->broadcastSkillInvoke(objectName(), 2);
            DummyCard *to_goback;
            if (diaochan->getCardCount() <= target->getHp()) {
                to_goback = diaochan->isKongcheng() ? new DummyCard : diaochan->wholeHandCards();
                for (int i = 0; i < 4; i++)
                    if (diaochan->getEquip(i))
                        to_goback->addSubcard(diaochan->getEquip(i)->getEffectiveId());
            } else
                to_goback = (DummyCard *)room->askForExchange(diaochan, objectName(), target->getHp(), true, "LihunGoBack");

            CardMoveReason reason(CardMoveReason::S_REASON_GIVE, diaochan->objectName(),
                                  target->objectName(), objectName(), QString());
            room->moveCardTo(to_goback, diaochan, target, Player::PlaceHand, reason);
            delete to_goback;
        } else if (triggerEvent == EventPhaseStart && diaochan->getPhase() == Player::NotActive) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("LihunTarget"))
                    p->setFlags("-LihunTarget");
            }
        }

        return false;
    }
};

class Kuiwei: public TriggerSkill {
public:
    Kuiwei(): TriggerSkill("kuiwei") {
        events << EventPhaseStart;
    }

    static int getWeaponCount(ServerPlayer *caoren) {
        int n = 0;
        foreach (ServerPlayer *p, caoren->getRoom()->getAlivePlayers()) {
            if (p->getWeapon()) n++;
        }
        return n;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->isAlive()
               && (target->hasSkill(objectName()) || target->getMark("@kuiwei") > 0);
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *caoren, QVariant &) const{
        if (caoren->getPhase() == Player::Finish) {
            if (!caoren->hasSkill(objectName()))
                return false;
            if (!caoren->askForSkillInvoke(objectName()))
                return false;

            room->broadcastSkillInvoke(objectName());
            int n = getWeaponCount(caoren);
            caoren->drawCards(n + 2);
            caoren->turnOver();

            if (caoren->getMark("@kuiwei") == 0)
                room->addPlayerMark(caoren, "@kuiwei");
        } else if (caoren->getPhase() == Player::Draw) {
            if (caoren->getMark("@kuiwei") == 0)
                return false;
            room->removePlayerMark(caoren, "@kuiwei");
            int n = getWeaponCount(caoren);
            if (n > 0) {
                LogMessage log;
                log.type = "#KuiweiDiscard";
                log.from = caoren;
                log.arg = QString::number(n);
                log.arg2 = objectName();
                room->sendLog(log);

                room->askForDiscard(caoren, objectName(), n, n, false, true);
            }
        }
        return false;
    }
};

class Yanzheng: public OneCardViewAsSkill {
public:
    Yanzheng(): OneCardViewAsSkill("yanzheng") {
        filter_pattern = ".|.|.|equipped";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" && player->getHandcardNum() > player->getHp();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());
        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return player->getHandcardNum() > player->getHp() && !player->getEquips().isEmpty();
    }
};

class Manjuan: public TriggerSkill {
public:
    Manjuan(): TriggerSkill("manjuan") {
        events << BeforeCardsMove;
        frequency = Frequent;
    }

    void doManjuan(ServerPlayer *sp_pangtong, int card_id) const{
        Room *room = sp_pangtong->getRoom();
        sp_pangtong->setFlags("ManjuanInvoke");
        QList<int> DiscardPile = room->getDiscardPile(), toGainList;
        const Card *card = Sanguosha->getCard(card_id);
        foreach (int id, DiscardPile) {
            const Card *cd = Sanguosha->getCard(id);
            if (cd->getNumber() == card->getNumber())
                toGainList << id;
        }
        if (toGainList.isEmpty()) return;

        room->fillAG(toGainList, sp_pangtong);
        int id = room->askForAG(sp_pangtong, toGainList, false, objectName());
        if (id != -1)
            room->moveCardTo(Sanguosha->getCard(id), sp_pangtong, Player::PlaceHand, true);

        room->clearAG(sp_pangtong);
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *sp_pangtong, QVariant &data) const{
        if (sp_pangtong->hasFlag("ManjuanInvoke")) {
            sp_pangtong->setFlags("-ManjuanInvoke");
            return false;
        }

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        QList<int> ids;
        CardMoveReason reason(CardMoveReason::S_REASON_PUT, sp_pangtong->objectName(), "manjuan", QString());
        if (room->getTag("FirstRound").toBool())
            return false;
        if (move.to != sp_pangtong || move.to_place != Player::PlaceHand)
            return false;
        room->broadcastSkillInvoke(objectName());
        foreach (int card_id, move.card_ids) {
            const Card *card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, NULL, NULL, Player::DiscardPile, reason);
        }
        ids = move.card_ids;
        move.card_ids.clear();
        data = QVariant::fromValue(move);

        LogMessage log;
        log.type = "$ManjuanGot";
        log.from = sp_pangtong;
        log.card_str = IntList2StringList(ids).join("+");
        room->sendLog(log);

        if (sp_pangtong->getPhase() == Player::NotActive || !sp_pangtong->askForSkillInvoke(objectName(), data))
            return false;

        foreach (int _card_id, ids) {
            doManjuan(sp_pangtong, _card_id);
            if (!sp_pangtong->isAlive()) break;
        }

        return false;
    }
};

class Zuixiang: public TriggerSkill {
public:
    Zuixiang(): TriggerSkill("zuixiang") {
        events << EventPhaseStart << SlashEffected << CardEffected;
        frequency = Limited;
        limit_mark = "@sleep";

        type[Card::TypeBasic] = "BasicCard";
        type[Card::TypeTrick] = "TrickCard";
        type[Card::TypeEquip] = "EquipCard";
    }

    void doZuixiang(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->broadcastSkillInvoke("zuixiang");
        if (player->getPile("dream").isEmpty())
            room->doLightbox("$ZuixiangAnimate", 3000);

        QList<Card::CardType> type_list;
        foreach (int card_id, player->getPile("dream")) {
            const Card *c = Sanguosha->getCard(card_id);
            type_list << c->getTypeId();
        }

        QList<int> ids = room->getNCards(3, false);
        CardsMoveStruct move(ids, player, Player::PlaceTable,
                             CardMoveReason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), "zuixiang", QString()));
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        player->addToPile("dream", ids, true);
        foreach (int id, ids) {
            const Card *cd = Sanguosha->getCard(id);
            if (type[cd->getTypeId()] == "EquipCard") {
                if (player->getMark("Equips_Nullified_to_Yourself") == 0)
                    room->setPlayerMark(player, "Equips_Nullified_to_Yourself", 1);
                if (player->getMark("Equips_of_Others_Nullified_to_You") == 0)
                    room->setPlayerMark(player, "Equips_of_Others_Nullified_to_You", 1);
            }
            if (!type_list.contains(cd->getTypeId())) {
                type_list << cd->getTypeId();
                room->setPlayerCardLimitation(player, "use,response", type[cd->getTypeId()], false);
            }
        }

        QList<int> zuixiang = player->getPile("dream");
        QSet<int> numbers;
        bool zuixiangDone = false;
        foreach (int id, zuixiang) {
            const Card *card = Sanguosha->getCard(id);
            if (numbers.contains(card->getNumber())) {
                zuixiangDone = true;
                break;
            }            
            numbers.insert(card->getNumber());
        }
        if (zuixiangDone) {
            player->addMark("zuixiangHasTrigger");
            room->setPlayerMark(player, "Equips_Nullified_to_Yourself", 0);
            room->setPlayerMark(player, "Equips_of_Others_Nullified_to_You", 0);
            room->removePlayerCardLimitation(player, "use,response", "BasicCard$0");
            room->removePlayerCardLimitation(player, "use,response", "TrickCard$0");
            room->removePlayerCardLimitation(player, "use,response", "EquipCard$0");

            LogMessage log;
            log.type = "$ZuixiangGot";
            log.from = player;

            log.card_str = IntList2StringList(zuixiang).join("+");
            room->sendLog(log);

            player->setFlags("ManjuanInvoke");
            CardsMoveStruct move(zuixiang, player, Player::PlaceHand,
                                 CardMoveReason(CardMoveReason::S_REASON_PUT, player->objectName(), QString(), "zuixiang", QString()));
            room->moveCardsAtomic(move, true);
        }
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *sp_pangtong, QVariant &data) const{
        QList<int> zuixiang = sp_pangtong->getPile("dream");

        if (triggerEvent == EventPhaseStart && sp_pangtong->getMark("zuixiangHasTrigger") == 0) {
            if (sp_pangtong->getPhase() == Player::Start) {
                if (TriggerSkill::triggerable(sp_pangtong) && sp_pangtong->getMark("@sleep") > 0) {
                    if (!sp_pangtong->askForSkillInvoke(objectName()))
                        return false;
                    room->removePlayerMark(sp_pangtong, "@sleep");
                    doZuixiang(sp_pangtong);
                } else if (!sp_pangtong->getPile("dream").isEmpty())
                    doZuixiang(sp_pangtong);
            }
        } else if (triggerEvent == CardEffected && TriggerSkill::triggerable(sp_pangtong)) {
            if (zuixiang.isEmpty())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("Slash")) return false; // we'll judge it later
            bool eff = true;
            foreach (int card_id, zuixiang) {
                const Card *c = Sanguosha->getCard(card_id);
                if (c->getTypeId() == effect.card->getTypeId()) {
                    eff = false;
                    break;
                }
            }

            if (!eff) {
                LogMessage log;
                log.type = effect.from ? "#ZuiXiang1" : "#ZuiXiang2";
                log.from = effect.to;
                if (effect.from)
                    log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();

                room->sendLog(log);
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        } else if (triggerEvent == SlashEffected && TriggerSkill::triggerable(sp_pangtong)) {
            if (zuixiang.isEmpty())
                return false;

            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            bool eff = true;
            foreach (int card_id, zuixiang) {
                const Card *c = Sanguosha->getCard(card_id);
                if (c->getTypeId() == Card::TypeBasic) {
                    eff = false;
                    break;
                }
            }

            if (!eff) {
                LogMessage log;
                log.type = effect.from ? "#ZuiXiang1" : "#ZuiXiang2";
                log.from = effect.to;
                if (effect.from)
                    log.to << effect.from;
                log.arg = effect.slash->objectName();
                log.arg2 = objectName();

                room->sendLog(log);
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        return false;
    }

private:
    QMap<Card::CardType, QString> type;
};

class ZuixiangClear: public DetachEffectSkill {
public:
    ZuixiangClear(): DetachEffectSkill("zuixiang") {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        room->setPlayerMark(player, "Equips_Nullified_to_Yourself", 0);
        room->setPlayerMark(player, "Equips_of_Others_Nullified_to_You", 0);
        room->removePlayerCardLimitation(player, "use,response", "BasicCard$0");
        room->removePlayerCardLimitation(player, "use,response", "TrickCard$0");
        room->removePlayerCardLimitation(player, "use,response", "EquipCard$0");
    }
};

class Jie: public TriggerSkill {
public:
    Jie(): TriggerSkill("jie") {
        events << DamageCaused;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer || !damage.by_user
            || !damage.card || !damage.card->isKindOf("Slash") || !damage.card->isRed())
            return false;

        int index = 1;
        if (damage.to->getGeneralName().contains("lvbu"))
            index = 2;
        room->broadcastSkillInvoke(objectName(), index);
        room->notifySkillInvoked(player, objectName());

        LogMessage log;
        log.type = "#Jie";
        log.from = player;
        log.to << damage.to;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);
        data = QVariant::fromValue(damage);

        return false;
    }
};

DaheCard::DaheCard() {
}

bool DaheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void DaheCard::use(Room *room, ServerPlayer *zhangfei, QList<ServerPlayer *> &targets) const{
    zhangfei->pindian(targets.first(), "dahe", NULL);
}

class DaheViewAsSkill: public ZeroCardViewAsSkill {
public:
    DaheViewAsSkill(): ZeroCardViewAsSkill("dahe") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaheCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new DaheCard;
    }
};

class Dahe: public TriggerSkill {
public:
    Dahe() :TriggerSkill("dahe") {
        events << JinkEffect << EventPhaseChanging << Death;
        view_as_skill = new DaheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == JinkEffect) {
            CardStar jink = data.value<CardStar>();
            ServerPlayer *bgm_zhangfei = room->findPlayerBySkillName(objectName());
            if (bgm_zhangfei && bgm_zhangfei->isAlive() && player->hasFlag(objectName()) && jink->getSuit() != Card::Heart) {
                LogMessage log;
                log.type = "#DaheEffect";
                log.from = bgm_zhangfei;
                log.to << player;
                log.arg = jink->getSuitString();
                log.arg2 = "dahe";
                room->sendLog(log);

                return true;
            }
            return false;
        }
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        }
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
        }
        foreach (ServerPlayer *other, room->getOtherPlayers(player))
            if (other->hasFlag(objectName()))
                room->setPlayerFlag(other, "-" + objectName());
        return false;
    }
};

class DahePindian: public TriggerSkill {
public:
    DahePindian(): TriggerSkill("#dahe") {
        events << Pindian;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->reason != "dahe" || !pindian->from->hasSkill(objectName())
            || room->getCardPlace(pindian->to_card->getEffectiveId()) != Player::PlaceTable)
            return false;

        if (pindian->isSuccess()) {
            room->setPlayerFlag(pindian->to, "dahe");
            QList<ServerPlayer *> to_givelist;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->getHp() <= pindian->from->getHp())
                    to_givelist << p;
            }
            if (!to_givelist.isEmpty()) {
                ServerPlayer *to_give = room->askForPlayerChosen(pindian->from, to_givelist, "dahe", "@dahe-give", true);
                if (!to_give) return false;
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, pindian->from->objectName());
                reason.m_playerId = to_give->objectName();
                to_give->obtainCard(pindian->to_card);
            }
        } else {
            if (!pindian->from->isKongcheng()) {
                room->showAllCards(pindian->from);
                room->askForDiscard(pindian->from, "dahe", 1, 1, false, false);
            }
        }
        return false;
    }
};

TanhuCard::TanhuCard() {
}

bool TanhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TanhuCard::use(Room *room, ServerPlayer *lvmeng, QList<ServerPlayer *> &targets) const{
    bool success = lvmeng->pindian(targets.first(), "tanhu", NULL);
    if (success) {
        room->broadcastSkillInvoke("tanhu", 2);
        lvmeng->tag["TanhuInvoke"] = QVariant::fromValue(targets.first());
        targets.first()->setFlags("TanhuTarget");
        room->setFixedDistance(lvmeng, targets.first(), 1);
    } else {
        room->broadcastSkillInvoke("tanhu", 3);
    }
}

class TanhuViewAsSkill: public ZeroCardViewAsSkill {
public:
    TanhuViewAsSkill(): ZeroCardViewAsSkill("tanhu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TanhuCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new TanhuCard;
    }
};

class Tanhu: public TriggerSkill {
public:
    Tanhu(): TriggerSkill("tanhu") {
        events << EventPhaseChanging << Death << TrickCardCanceling;
        view_as_skill = new TanhuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TrickCardCanceling) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.from && effect.from->tag["TanhuInvoke"].value<PlayerStar>() != NULL
                && effect.to && effect.to->hasFlag("TanhuTarget"))
                return true;
        } else if (player->tag["TanhuInvoke"].value<PlayerStar>() != NULL) {
            if (triggerEvent == EventPhaseChanging) {
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.to != Player::NotActive)
                    return false;
            } else if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player)
                    return false;
            }

            ServerPlayer *target = player->tag["TanhuInvoke"].value<PlayerStar>();

            target->setFlags("-TanhuTarget");
            room->setFixedDistance(player, target, -1);
            player->tag.remove("TanhuInvoke");
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class MouduanStart: public TriggerSkill {
public:
    MouduanStart(): TriggerSkill("#mouduan-start") {
        events << GameStart << EventAcquireSkill;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data) const{
        if (triggerEvent == GameStart) {
            lvmeng->gainMark("@wu");
            room->handleAcquireDetachSkills(lvmeng, "jiang|qianxun");
            room->getThread()->addTriggerSkill(Sanguosha->getTriggerSkill("keji"));
        } else if (data.toString() == "mouduan") {
            if (lvmeng->getMark("@wu") > 0)
                room->handleAcquireDetachSkills(lvmeng, "jiang|qianxun");
            else if (lvmeng->getMark("@wen") > 0)
                room->handleAcquireDetachSkills(lvmeng, "yingzi|keji");
        }
        return false;
    }
};

class Mouduan: public TriggerSkill {
public:
    Mouduan(): TriggerSkill("mouduan") {
        events << EventPhaseStart << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *lvmeng = room->findPlayerBySkillName(objectName());

        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && player->isAlive() && player->hasSkill(objectName(), true)
                && player->getMark("@wu") > 0 && player->getHandcardNum() <= 2) {
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = "mouduan";
                room->sendLog(log);
                room->notifySkillInvoked(player, objectName());

                player->loseMark("@wu");
                player->gainMark("@wen");
                room->handleAcquireDetachSkills(player, "-jiang|-qianxun|yingzi|keji", true);
            }
        } else if (player->getPhase() == Player::RoundStart && lvmeng && lvmeng->getMark("@wen") > 0
                   && lvmeng->canDiscard(lvmeng, "he") && room->askForCard(lvmeng, "..", "@mouduan", QVariant(), objectName())) {
            if (lvmeng->getHandcardNum() > 2) {
                room->broadcastSkillInvoke(objectName());
                lvmeng->loseMark("@wen");
                lvmeng->gainMark("@wu");
                room->handleAcquireDetachSkills(lvmeng, "-yingzi|-keji|jiang|qianxun", true);
            }
        }
        return false;
    }
};

class MouduanClear: public DetachEffectSkill {
public:
    MouduanClear(): DetachEffectSkill("mouduan") {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        if (player->getMark("@wu") > 0) {
            player->loseMark("@wu");
            room->handleAcquireDetachSkills(player, "-jiang|-qianxun", true);
        } else if (player->getMark("@wen") > 0) {
            player->loseMark("@wen");
            room->handleAcquireDetachSkills(player, "-yingzi|-keji", true);
        }
    }
};

class Zhaolie: public DrawCardsSkill {
public:
    Zhaolie(): DrawCardsSkill("zhaolie") {
    }

    virtual int getDrawNum(ServerPlayer *liubei, int n) const{
        Room *room = liubei->getRoom();
        QList<ServerPlayer *> targets = room->getOtherPlayers(liubei);
        QList<ServerPlayer *> victims;
        foreach (ServerPlayer *p, targets) {
            if (liubei->inMyAttackRange(p))
                victims << p;
        }
        if (victims.empty())
            return n;
        ServerPlayer *victim = room->askForPlayerChosen(liubei, victims, "zhaolie", "zhaolie-invoke", true, true);
        if (victim) {
            victim->setFlags("ZhaolieTarget");
            liubei->setFlags("zhaolie");
            return n - 1;
        }

        return n;
    }
};

class ZhaolieAct: public TriggerSkill {
public:
    ZhaolieAct(): TriggerSkill("#zhaolie") {
        events << AfterDrawNCards;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *liubei, QVariant &) const{
        if (!liubei->hasFlag("zhaolie")) return false;
        liubei->setFlags("-zhaolie");

        ServerPlayer *victim = NULL;
        foreach (ServerPlayer *p, room->getOtherPlayers(liubei)) {
            if (p->hasFlag("ZhaolieTarget")) {
                p->setFlags("-ZhaolieTarget");
                victim = p;
                break;
            }
        }
        if (!victim) return false;

        QList<const Card *> cards;
        int no_basic = 0;

        QList<int> cardIds;
        for (int i = 0; i < 3; i++) {
            int id = room->drawCard();
            cardIds << id;
            CardsMoveStruct move(id, NULL, Player::PlaceTable,
                                 CardMoveReason(CardMoveReason::S_REASON_TURNOVER, liubei->objectName(), QString(), "zhaolie", QString()));
            room->moveCardsAtomic(move, true);
            room->getThread()->delay();
        }
        DummyCard *dummy = new DummyCard;
        dummy->deleteLater();
        for (int i = 0; i < 3; i++) {
            int card_id = cardIds[i];
            const Card *card = Sanguosha->getCard(card_id);
            if (!card->isKindOf("BasicCard") || card->isKindOf("Peach")) {
                if (!card->isKindOf("BasicCard"))
                    no_basic++;
                dummy->addSubcard(card_id);
            } else {
                cards << card;
            }
        }
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "zhaolie", QString());
        if (dummy->subcardsLength() > 0)
            room->throwCard(dummy, reason, NULL);
        dummy->clearSubcards();

        if (no_basic == 0 && cards.isEmpty())
            return false;
        dummy->addSubcards(cards);

        if (no_basic == 0) {
            if (room->askForSkillInvoke(victim, "zhaolie_obtain", "obtain:" + liubei->objectName())) {
                room->broadcastSkillInvoke("zhaolie", 2);
                liubei->obtainCard(dummy);
            } else {
                room->broadcastSkillInvoke("zhaolie", 1);
                victim->obtainCard(dummy);
            }
        } else {
            if (victim->getCardCount() >= no_basic
                && room->askForDiscard(victim, "zhaolie", no_basic, no_basic, true, true, "@zhaolie-discard:" + liubei->objectName())) {
                room->broadcastSkillInvoke("zhaolie", 2);
                if (dummy->subcardsLength() > 0) {
                    if (liubei->isAlive())
                        liubei->obtainCard(dummy);
                    else
                        room->throwCard(dummy, reason, NULL);
                }
            } else {
                room->broadcastSkillInvoke("zhaolie", 1);
                if (no_basic > 0)
                    room->damage(DamageStruct("zhaolie", liubei, victim, no_basic));
                if (dummy->subcardsLength() > 0) {
                    if (victim->isAlive())
                        victim->obtainCard(dummy);
                    else
                        room->throwCard(dummy, reason, NULL);
                }
            }
        }
        return false;
    }
};

ShichouCard::ShichouCard() {
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
}

bool ShichouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getKingdom() == "shu" && to_select != Self;
}

void ShichouCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    ServerPlayer *player = effect.from, *victim = effect.to;
    room->broadcastSkillInvoke("shichou");
    room->doLightbox("$ShichouAnimate", 4500);

    room->removePlayerMark(player, "@hate");
    room->setPlayerMark(player, "xhate", 1);
    victim->gainMark("@hate_to");
    room->setPlayerMark(victim, "hate_" + player->objectName(), 1);

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, player->objectName());
    reason.m_playerId = victim->objectName();
    room->obtainCard(victim, this, reason, false);
}

class ShichouViewAsSkill: public ViewAsSkill {
public:
    ShichouViewAsSkill(): ViewAsSkill("shichou") {
        response_pattern = "@@shichou";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *) const{
        return selected.length() < 2;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        ShichouCard *card = new ShichouCard;
        card->addSubcards(cards);
        return card;
    }
};

class Shichou: public TriggerSkill {
public:
    Shichou(): TriggerSkill("shichou$") {
        events << EventPhaseStart << DamageInflicted << Dying;
        frequency = Limited;
        limit_mark = "@hate";
        view_as_skill = new ShichouViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && player->getMark("xhate") == 0 && player->hasLordSkill("shichou")
                   && player->getPhase() == Player::Start && player->getCards("he").length() > 1) {
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->getKingdom() == "shu") {
                    room->askForUseCard(player, "@@shichou", "@shichou-give", -1, Card::MethodNone);
                    break;
                }
            }
        } else if (triggerEvent == DamageInflicted && player->hasLordSkill(objectName()) && player->getMark("ShichouTarget") == 0) {
            ServerPlayer *target = NULL;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->getMark("hate_" + player->objectName()) > 0 && p->getMark("@hate_to") > 0) {
                    target = p;
                    break;
                }
            }
            if (target == NULL || target->isDead())
                return false;
            LogMessage log;
            log.type = "#ShichouProtect";
            log.arg = objectName();
            log.from = player;
            log.to << target;
            room->sendLog(log);
            DamageStruct damage = data.value<DamageStruct>();

            if (damage.card && damage.card->isKindOf("Slash"))
                player->removeQinggangTag(damage.card);

            DamageStruct newdamage = damage;
            newdamage.to = target;
            newdamage.transfer = true;

            target->addMark("ShichouTarget");
            try {
                room->damage(newdamage);
            }
            catch (TriggerEvent triggerEvent) {
                if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                    target->removeMark("ShichouTarget");
                throw triggerEvent;
            }
            return true;
        } else if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.who != player)
                return false;
            if (player->getMark("@hate_to") > 0)
                player->loseAllMarks("@hate_to");
        }
        return false;
    }
};

class ShichouDraw: public TriggerSkill {
public:
    ShichouDraw(): TriggerSkill("#shichou") {
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (player->isAlive() && player->getMark("ShichouTarget") > 0 && damage.transfer) {
            player->drawCards(damage.damage);
            player->removeMark("ShichouTarget");
        }

        return false;
    }
};

YanxiaoCard::YanxiaoCard(Suit suit, int number)
    : DelayedTrick(suit, number)
{
    mute = true;
    handling_method = Card::MethodNone;
    setObjectName("YanxiaoCard");
}

bool YanxiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->containsTrick(objectName()))
        return false;

    return true;
}

void YanxiaoCard::onUse(Room *room, const CardUseStruct &card_use) const{
    bool has_sunce = false;
    foreach (ServerPlayer *to, card_use.to)
        if (to->getGeneralName().contains("sunce")) {
            has_sunce = true;
            break;
        }
    room->broadcastSkillInvoke("yanxiao", has_sunce ? 2 : 1);
    DelayedTrick::onUse(room, card_use);
}

void YanxiaoCard::takeEffect(ServerPlayer *) const{
}

class YanxiaoViewAsSkill: public OneCardViewAsSkill {
public:
    YanxiaoViewAsSkill(): OneCardViewAsSkill("yanxiao") {
        filter_pattern = ".|diamond";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        YanxiaoCard *yanxiao = new YanxiaoCard(originalCard->getSuit(), originalCard->getNumber());
        yanxiao->addSubcard(originalCard->getId());
        yanxiao->setSkillName(objectName());
        return yanxiao;
    }
};

class Yanxiao: public PhaseChangeSkill {
public:
    Yanxiao(): PhaseChangeSkill("yanxiao") {
        view_as_skill = new YanxiaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getPhase() == Player::Judge && target->containsTrick("YanxiaoCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        CardsMoveStruct move;
        LogMessage log;
        log.type = "$YanxiaoGot";
        log.from = target;

        foreach (const Card *delayed_trick, target->getJudgingArea())
            move.card_ids << delayed_trick->getEffectiveId();
        log.card_str = IntList2StringList(move.card_ids).join("+");
        target->getRoom()->sendLog(log);

        move.to = target;
        move.to_place = Player::PlaceHand;
        target->getRoom()->moveCardsAtomic(move, true);

        return false;
    }
};

class Anxian: public TriggerSkill {
public:
    Anxian(): TriggerSkill("anxian") {
        events << DamageCaused << TargetConfirming << SlashEffected;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data) const{
        if (triggerEvent == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash")
                && damage.by_user && !damage.chain && !damage.transfer
                && daqiao->askForSkillInvoke(objectName(), data)) {
                room->broadcastSkillInvoke(objectName(), 1);
                LogMessage log;
                log.type = "#Anxian";
                log.from = daqiao;
                log.arg = objectName();
                room->sendLog(log);
                if (damage.to->canDiscard(damage.to, "h"))
                    room->askForDiscard(damage.to, "anxian", 1, 1);
                daqiao->drawCards(1);
                return true;
            }
        } else if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.to.contains(daqiao) || !daqiao->canDiscard(daqiao, "h"))
                return false;
            if (use.card->isKindOf("Slash")) {
                if (room->askForCard(daqiao, ".", "@anxian-discard", data, objectName())) {
                    room->broadcastSkillInvoke(objectName(), 2);
                    daqiao->addMark("anxian");
                    use.from->drawCards(1);
                }
            }
        } else if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (daqiao->getMark("anxian") > 0) {
                LogMessage log;
                log.type = "#AnxianAvoid";
                log.from = effect.from;
                log.to << daqiao;
                log.arg = objectName();
                room->sendLog(log);
                daqiao->removeMark("anxian");
                return true;
            }
        }
        return false;
    }
};

class AnxianRemoveMark: public TriggerSkill {
public:
    AnxianRemoveMark(): TriggerSkill("#anxian") {
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) {
            foreach (ServerPlayer *to, use.to)
                to->setMark("anxian", 0);
        }
        return false;
    }
};

YinlingCard::YinlingCard() {
}

bool YinlingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void YinlingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (!effect.from->canDiscard(effect.to, "he") || effect.from->getPile("brocade").length() >= 4)
        return;
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "yinling", false, Card::MethodDiscard);
    effect.from->addToPile("brocade", card_id);
}

class Yinling: public OneCardViewAsSkill {
public:
    Yinling(): OneCardViewAsSkill("yinling") {
        filter_pattern = ".|black!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("brocade").length() < 4;
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        YinlingCard *card = new YinlingCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class Junwei: public TriggerSkill {
public:
    Junwei(): TriggerSkill("junwei") {
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *ganning, QVariant &) const{
        if (ganning->getPhase() == Player::Finish && ganning->getPile("brocade").length() >= 3) {
            ServerPlayer *target = room->askForPlayerChosen(ganning, room->getAllPlayers(), objectName(), "junwei-invoke", true, true);
            if (!target) return false;
            QList<int> brocade = ganning->getPile("brocade");
            room->broadcastSkillInvoke(objectName());

            int ai_delay = Config.AIDelay;
            Config.AIDelay = 0;

            QList<const Card *> to_throw;
            for (int i = 0; i < 3; i++) {
                int card_id = 0;
                room->fillAG(brocade, ganning);
                if (brocade.length() == 3 - i)
                    card_id = brocade.first();
                else
                    card_id = room->askForAG(ganning, brocade, false, objectName());
                room->clearAG(ganning);

                brocade.removeOne(card_id);

                to_throw << Sanguosha->getCard(card_id);
            }
            DummyCard *dummy = new DummyCard;
            dummy->addSubcards(to_throw);
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
            room->throwCard(dummy, reason, NULL);
            dummy->deleteLater();

            Config.AIDelay = ai_delay;

            QVariant ai_data = QVariant::fromValue((PlayerStar)ganning);
            const Card *card = room->askForCard(target, "Jink", "@junwei-show", ai_data, Card::MethodNone);
            if (card) {
                room->showCard(target, card->getEffectiveId());
                ServerPlayer *receiver = room->askForPlayerChosen(ganning, room->getAllPlayers(), "junweigive", "@junwei-give");
                if (receiver != target)
                    receiver->obtainCard(card);
            } else {
                room->loseHp(target, 1);
                if (!target->isAlive()) return false;
                if (target->hasEquip()) {
                    int card_id = room->askForCardChosen(ganning, target, "e", objectName());
                    target->addToPile("junwei_equip", card_id);
                }
            }
        }
        return false;
    }
};

class JunweiGot: public TriggerSkill {
public:
    JunweiGot(): TriggerSkill("#junwei-got") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive || player->getPile("junwei_equip").length() == 0)
            return false;
        foreach (int card_id, player->getPile("junwei_equip")) {
            const Card *card = Sanguosha->getCard(card_id);

            int equip_index = -1;
            const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
            equip_index = static_cast<int>(equip->location());

            QList<CardsMoveStruct> exchangeMove;
            CardsMoveStruct move1(card_id, player, Player::PlaceEquip,
                                  CardMoveReason(CardMoveReason::S_REASON_PUT, player->objectName()));
            exchangeMove.push_back(move1);
            if (player->getEquip(equip_index) != NULL) {
                CardsMoveStruct move2(player->getEquip(equip_index)->getId(), NULL, Player::DiscardPile,
                                      CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, player->objectName()));
                exchangeMove.push_back(move2);
            }
            LogMessage log;
            log.from = player;
            log.type = "$JunweiGot";
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->moveCardsAtomic(exchangeMove, true);
        }
        return false;
    }
};

class Fenyong: public TriggerSkill {
public:
    Fenyong(): TriggerSkill("fenyong") {
        events << Damaged << DamageInflicted;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (triggerEvent == Damaged) {
            if (player->getMark("@fenyong") == 0 && room->askForSkillInvoke(player, objectName())) {
                player->gainMark("@fenyong");
                room->broadcastSkillInvoke(objectName(), 1);
            }
        } else if (triggerEvent == DamageInflicted) {
            if (player->getMark("@fenyong") > 0) {
                room->broadcastSkillInvoke(objectName(), 2);
                LogMessage log;
                log.type = "#FenyongAvoid";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class FenyongWithoutXuehen: public TriggerSkill {
public:
    FenyongWithoutXuehen():TriggerSkill("#fenyong-without-xuehen") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *xiahou = room->findPlayerBySkillName("fenyong");
        if (xiahou == NULL)
            return false;
        if (xiahou->getPhase() == Player::Finish && xiahou->getMark("@fenyong") > 0) {
            if (xiahou->tag["HuashenSkill"].toString() == "fenyong"
                || xiahou->tag["XiaodeSkill"].toString() == "fenyong")
                room->setPlayerMark(xiahou, "@fenyong", 0);
        }
        return false;
    }
};

class FenyongClear: public DetachEffectSkill {
public:
    FenyongClear(): DetachEffectSkill("fenyong") {
    }

    virtual void onSkillDetached(Room *, ServerPlayer *player) const{
        player->loseAllMarks("@fenyong");
    }
};

class Xuehen: public TriggerSkill {
public:
    Xuehen(): TriggerSkill("xuehen") {
        events << EventPhaseStart;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *xiahou = room->findPlayerBySkillName(objectName());
        if (xiahou == NULL)
            return false;
        if (player->getPhase() == Player::Finish && xiahou->getMark("@fenyong") > 0) {
            LogMessage log;
            log.from = xiahou;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            xiahou->loseMark("@fenyong");
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(xiahou))
                if (xiahou->canSlash(p, NULL, false))
                    targets << p;
            QString choice;
            if (!Slash::IsAvailable(xiahou) || targets.isEmpty())
                choice = "discard";
            else
                choice = room->askForChoice(xiahou, objectName(), "discard+slash");
            if (choice == "slash") {
                room->broadcastSkillInvoke(objectName(), 2);

                ServerPlayer *victim = room->askForPlayerChosen(xiahou, targets, objectName(), "@dummy-slash");

                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                room->useCard(CardUseStruct(slash, xiahou, victim), false);
            } else {
                room->broadcastSkillInvoke(objectName(), 1);
                room->setPlayerFlag(player, "xuehen_InTempMoving");
                DummyCard *dummy = new DummyCard;
                QList<int> card_ids;
                QList<Player::Place> original_places;
                for (int i = 0; i < xiahou->getLostHp(); i++) {
                    if (!xiahou->canDiscard(player, "he"))
                        break;
                    card_ids << room->askForCardChosen(xiahou, player, "he", objectName(), false, Card::MethodDiscard);
                    original_places << room->getCardPlace(card_ids[i]);
                    dummy->addSubcard(card_ids[i]);
                    player->addToPile("#xuehen", card_ids[i], false);
                }
                for (int i = 0; i < dummy->subcardsLength(); i++)
                    room->moveCardTo(Sanguosha->getCard(card_ids[i]), player, original_places[i], false);
                room->setPlayerFlag(player, "-xuehen_InTempMoving");
                if (dummy->subcardsLength() > 0)
                    room->throwCard(dummy, player, xiahou);
                dummy->deleteLater();
            }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

BGMPackage::BGMPackage(): Package("BGM") {
    General *bgm_zhaoyun = new General(this, "bgm_zhaoyun", "qun", 3); // *SP 001
    bgm_zhaoyun->addSkill("longdan");
    bgm_zhaoyun->addSkill(new Chongzhen);

    General *bgm_diaochan = new General(this, "bgm_diaochan", "qun", 3, false); // *SP 002
    bgm_diaochan->addSkill(new Lihun);
    bgm_diaochan->addSkill("biyue");

    General *bgm_caoren = new General(this, "bgm_caoren", "wei"); // *SP 003
    bgm_caoren->addSkill(new Kuiwei);
    bgm_caoren->addSkill(new Yanzheng);

    General *bgm_pangtong = new General(this, "bgm_pangtong", "qun", 3); // *SP 004
    bgm_pangtong->addSkill(new Manjuan);
    bgm_pangtong->addSkill(new Zuixiang);
    bgm_pangtong->addSkill(new ZuixiangClear);
    related_skills.insertMulti("zuixiang", "#zuixiang-clear");

    General *bgm_zhangfei = new General(this, "bgm_zhangfei", "shu"); // *SP 005
    bgm_zhangfei->addSkill(new Jie);
    bgm_zhangfei->addSkill(new Dahe);
    bgm_zhangfei->addSkill(new DahePindian);
    related_skills.insertMulti("dahe", "#dahe");

    General *bgm_lvmeng = new General(this, "bgm_lvmeng", "wu", 3); // *SP 006
    bgm_lvmeng->addSkill(new Tanhu);
    bgm_lvmeng->addSkill(new MouduanStart);
    bgm_lvmeng->addSkill(new Mouduan);
    bgm_lvmeng->addSkill(new MouduanClear);
    related_skills.insertMulti("mouduan", "#mouduan-start");
    related_skills.insertMulti("mouduan", "#mouduan-clear");

    General *bgm_liubei = new General(this, "bgm_liubei$", "shu"); // *SP 007
    bgm_liubei->addSkill(new Zhaolie);
    bgm_liubei->addSkill(new ZhaolieAct);
    bgm_liubei->addSkill(new Shichou);
    bgm_liubei->addSkill(new ShichouDraw);
    related_skills.insertMulti("zhaolie", "#zhaolie");
    related_skills.insertMulti("shichou", "#shichou");

    General *bgm_daqiao = new General(this, "bgm_daqiao", "wu", 3, false); // *SP 008
    bgm_daqiao->addSkill(new Yanxiao);
    bgm_daqiao->addSkill(new Anxian);
    bgm_daqiao->addSkill(new AnxianRemoveMark);
    related_skills.insertMulti("anxian", "#anxian");

    General *bgm_ganning = new General(this, "bgm_ganning", "qun"); // *SP 009
    bgm_ganning->addSkill(new Yinling);
    bgm_ganning->addSkill(new DetachEffectSkill("yinling", "brocade"));
    bgm_ganning->addSkill(new Junwei);
    bgm_ganning->addSkill(new JunweiGot);
    related_skills.insertMulti("yinling", "#yinling-clear");
    related_skills.insertMulti("junwei", "#junwei-got");

    General *bgm_xiahoudun = new General(this, "bgm_xiahoudun", "wei"); // *SP 010
    bgm_xiahoudun->addSkill(new Fenyong);
    bgm_xiahoudun->addSkill(new FenyongClear);
    bgm_xiahoudun->addSkill(new FenyongWithoutXuehen);
    bgm_xiahoudun->addSkill(new Xuehen);
    bgm_xiahoudun->addSkill(new SlashNoDistanceLimitSkill("xuehen"));
    bgm_xiahoudun->addSkill(new FakeMoveSkill("xuehen"));
    related_skills.insertMulti("fenyong", "#fenyong-clear");
    related_skills.insertMulti("fenyong", "#fenyong-without-xuehen");
    related_skills.insertMulti("xuehen", "#xuehen-slash-ndl");
    related_skills.insertMulti("xuehen", "#xuehen-fake-move");

    addMetaObject<LihunCard>();
    addMetaObject<DaheCard>();
    addMetaObject<TanhuCard>();
    addMetaObject<ShichouCard>();
    addMetaObject<YanxiaoCard>();
    addMetaObject<YinlingCard>();
}

ADD_PACKAGE(BGM)

// DIY Generals
ZhaoxinCard::ZhaoxinCard() {
    mute = true;
}

bool ZhaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(NoSuit, 0);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

void ZhaoxinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->showAllCards(source);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("_zhaoxin");
    room->useCard(CardUseStruct(slash, source, targets));
}

class ZhaoxinViewAsSkill: public ZeroCardViewAsSkill {
public:
    ZhaoxinViewAsSkill(): ZeroCardViewAsSkill("zhaoxin") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhaoxin" && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new ZhaoxinCard;
    }
};

class Zhaoxin: public TriggerSkill {
public:
    Zhaoxin(): TriggerSkill("zhaoxin") {
        events << EventPhaseEnd;
        view_as_skill = new ZhaoxinViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *simazhao, QVariant &) const{
        if (simazhao->getPhase() != Player::Draw)
            return false;
        if (simazhao->isKongcheng() || !Slash::IsAvailable(simazhao))
            return false;

        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (simazhao->canSlash(p))
                targets << p;

        if (targets.isEmpty())
            return false;

        room->askForUseCard(simazhao, "@@zhaoxin", "@zhaoxin");
        return false;
    }
};

class Langgu: public TriggerSkill {
public:
    Langgu(): TriggerSkill("langgu") {
        events << Damaged << AskForRetrial << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *simazhao, QVariant &data) const{
        if (TriggerSkill::triggerable(simazhao) && triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();

            for (int i = 0; i < damage.damage; i++) {
                if (!simazhao->isAlive() || !simazhao->askForSkillInvoke(objectName(), data))
                    return false;
                room->broadcastSkillInvoke(objectName());

                JudgeStruct judge;
                judge.good = true;
                judge.play_animation = false;
                judge.who = simazhao;
                judge.reason = objectName();

                room->judge(judge);
                if (simazhao->isAlive() && damage.from && damage.from->isAlive() && !damage.from->isKongcheng()) {
                    QList<int> langgu_discard, other;
                    Card::Suit suit = (Card::Suit)(judge.pattern.toInt());
                    foreach (int card_id, damage.from->handCards()) {
                        if (simazhao->canDiscard(damage.from, card_id) && Sanguosha->getCard(card_id)->getSuit() == suit)
                            langgu_discard << card_id;
                        else
                            other << card_id;
                    }
                    if (langgu_discard.isEmpty()) {
                        room->showAllCards(damage.from, simazhao);
                        return false;
                    }

                    LogMessage log;
                    log.type = "$ViewAllCards";
                    log.from = simazhao;
                    log.to << damage.from;
                    log.card_str = IntList2StringList(damage.from->handCards()).join("+");
                    room->doNotify(simazhao, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

                    while (!langgu_discard.empty()) {
                        room->fillAG(langgu_discard + other, simazhao, other);
                        int id = room->askForAG(simazhao, langgu_discard, true, objectName());
                        if (id == -1) {
                            room->clearAG(simazhao);
                            break;
                        }
                        langgu_discard.removeOne(id);
                        other.prepend(id);
                        room->clearAG(simazhao);
                    }

                    if (!langgu_discard.empty()) {
                        DummyCard *dummy = new DummyCard(langgu_discard);
                        room->throwCard(dummy, damage.from, simazhao);
                        dummy->deleteLater();
                    }
                }
            }
        } else if (TriggerSkill::triggerable(simazhao) && triggerEvent == AskForRetrial) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason != objectName() || simazhao->isKongcheng())
                return false;

            QStringList prompt_list;
            prompt_list << "@langgu-card" << judge->who->objectName()
                        << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
            QString prompt = prompt_list.join(":");
            const Card *card = room->askForCard(simazhao, ".", prompt, data, Card::MethodResponse, judge->who, true);

            if (card)
                room->retrial(card, simazhao, judge, objectName());
        } else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason != objectName()) return false;
            judge->pattern = QString::number(int(judge->card->getSuit()));
        }

        return false;
    }
};

FuluanCard::FuluanCard() {
}

bool FuluanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
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

void FuluanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    effect.to->turnOver();
    room->setPlayerCardLimitation(effect.from, "use", "Slash", true);
}

class Fuluan: public ViewAsSkill {
public:
    Fuluan(): ViewAsSkill("fuluan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getCardCount() >= 3 && !player->hasUsed("FuluanCard") && !player->hasFlag("ForbidFuluan");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *card) const{
        if (selected.length() >= 3)
            return false;

        if (Self->isJilei(card))
            return false;

        if (!selected.isEmpty()) {
            Card::Suit suit = selected.first()->getSuit();
            return card->getSuit() == suit;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 3)
            return NULL;

        FuluanCard *card = new FuluanCard;
        card->addSubcards(cards);

        return card;
    }
};

class FuluanForbid: public TriggerSkill {
public:
    FuluanForbid(): TriggerSkill("#fuluan-forbid") {
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && player->getPhase() == Player::Play && !player->hasFlag("ForbidFuluan"))
            room->setPlayerFlag(player, "ForbidFuluan");
        return false;
    }
};

class Shude: public TriggerSkill {
public:
    Shude(): TriggerSkill("shude") {
        events << EventPhaseStart;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *wangyuanji, QVariant &) const{
        if (wangyuanji->getPhase() == Player::Finish) {
            int upper = wangyuanji->getMaxHp();
            int handcard = wangyuanji->getHandcardNum();
            if (handcard < upper && room->askForSkillInvoke(wangyuanji, objectName())) {
                room->broadcastSkillInvoke(objectName());
                wangyuanji->drawCards(upper - handcard);
            }
        }

        return false;
    }
};

HuangenCard::HuangenCard() {
}

bool HuangenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < Self->getHp() && to_select->hasFlag("HuangenTarget");
}

void HuangenCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->tag["Huangen"] = effect.from->tag["Huangen_user"].toString();
    effect.to->drawCards(1);
}

class HuangenViewAsSkill: public ZeroCardViewAsSkill {
public:
    HuangenViewAsSkill():ZeroCardViewAsSkill("huangen") {
        response_pattern = "@@huangen";
    }

    virtual const Card *viewAs() const{
        return new HuangenCard;
    }
};

class Huangen: public TriggerSkill {
public:
    Huangen(): TriggerSkill("huangen") {
        events << TargetConfirmed << CardEffected;
        view_as_skill = new HuangenViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *liuxie = room->findPlayerBySkillName(objectName());
        if (liuxie == NULL) return false;
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (player != liuxie || liuxie->getHp() <= 0) return false;
            if (use.to.length() <= 1 || !use.card->isKindOf("TrickCard") || use.card->isKindOf("Collateral"))
                    return false;

            liuxie->tag["Huangen_user"] = use.card->toString();
            foreach (ServerPlayer *p, use.to)
                room->setPlayerFlag(p, "HuangenTarget");
            room->askForUseCard(liuxie, "@@huangen", "@huangen-card");
            foreach (ServerPlayer *p, use.to)
                room->setPlayerFlag(p, "-HuangenTarget");
        } else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (liuxie->tag["Huangen_user"].toString() == effect.card->toString())
                liuxie->tag.remove("Huangen_user");
            if (!player->isAlive())
                return false;

            if (player->tag["Huangen"].isNull() || player->tag["Huangen"].toString() != effect.card->toString())
                return false;

            player->tag["Huangen"] = QVariant(QString());

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

#include "standard-skillcards.h"
HantongCard::HantongCard() {
    target_fixed = true;
    mute = true;
}

class HantongViewAsSkill: public ZeroCardViewAsSkill {
public:
    HantongViewAsSkill(): ZeroCardViewAsSkill("hantong") {
    }

    virtual const Card *viewAs() const{
        return new HantongCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        JijiangViewAsSkill *jijiang = new JijiangViewAsSkill;
        jijiang->deleteLater();
        return player->getPile("edict").length() > 0 && jijiang->isEnabledAtPlay(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        JijiangViewAsSkill *jijiang = new JijiangViewAsSkill;
        jijiang->deleteLater();
        return player->getPile("edict").length() > 0 && jijiang->isEnabledAtResponse(player, pattern);
    }
};


class Hantong: public TriggerSkill {
public:
    Hantong(): TriggerSkill("hantong") {
        events << BeforeCardsMove;
        view_as_skill = new HantongViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *liuxie, QVariant &data) const{
        if (liuxie->getPhase() != Player::Discard)
            return false;

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != liuxie)
            return false;
        if (move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)) {
            if (liuxie->askForSkillInvoke(objectName())) {
                room->broadcastSkillInvoke(objectName(), 1);
                int i = 0;
                QList<int> ids = move.card_ids, to_add;
                QList<Player::Place> places = move.from_places;
                foreach (int card_id, ids) {
                    if (places[i] == Player::PlaceHand) {
                        to_add.append(card_id);
                        move.card_ids.removeOne(card_id);
                        move.from_places.removeAt(i);
                    }
                    i++;
                }
                data = QVariant::fromValue(move);
                if (!to_add.isEmpty())
                    liuxie->addToPile("edict", to_add, true, move.reason);
            }
        }
        return false;
    }
};

class HantongAcquire: public TriggerSkill {
public:
    HantongAcquire(): TriggerSkill("#hantong-acquire") {
        events << CardAsked //For JiJiang and HuJia
               << TargetConfirmed //For JiuYuan
               << EventPhaseStart; //For XueYi
    }

    static void RemoveEdict(ServerPlayer *liuxie) {
        Room *room = liuxie->getRoom();
        QList<int> edict = liuxie->getPile("edict");
        room->broadcastSkillInvoke("hantong", 2);
        room->notifySkillInvoked(liuxie, "hantong");

        LogMessage log;
        log.type = "#InvokeSkill";
        log.arg = "hantong";
        log.from = liuxie;
        room->sendLog(log);

        int ai_delay = Config.AIDelay;
        Config.AIDelay = 0;

        int card_id = 0;
        room->fillAG(edict, liuxie);
        if (edict.length() == 1)
            card_id = edict.first();
        else
            card_id = room->askForAG(liuxie, edict, false, "hantong");
        room->clearAG(liuxie);

        edict.removeOne(card_id);

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "hantong_acquire", QString());
        room->throwCard(Sanguosha->getCard(card_id), reason, NULL);

        Config.AIDelay = ai_delay;
    }

    virtual int getPriority() const{
        return 4;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *liuxie, QVariant &data) const{
        ServerPlayer *current = room->getCurrent();
        if (current == NULL || !current->isAlive())
            return false;
        if (liuxie->getPile("edict").length() == 0)
            return false;
        switch (triggerEvent) {
        case CardAsked: {
                QString pattern = data.toStringList().first();
                if (pattern == "slash" && !liuxie->hasFlag("Global_JijiangFailed")) {
                    QVariant data_for_ai = "jijiang";
                    liuxie->tag["HantongOriginData"] = data; // For AI
                    if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                        RemoveEdict(liuxie);
                        room->acquireSkill(liuxie, "jijiang");
                    }
                } else if (pattern == "jink") {
                    QVariant data_for_ai = "hujia";
                    liuxie->tag["HantongOriginData"] = data; // For AI
                    if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                        RemoveEdict(liuxie);
                        room->acquireSkill(liuxie, "hujia");
                    }
                }
                break;
            }
        case TargetConfirmed: {
                CardUseStruct use = data.value<CardUseStruct>();
                if (!use.card->isKindOf("Peach") || use.from == NULL || use.from->getKingdom() != "wu"
                    || liuxie == use.from || !liuxie->hasFlag("Global_Dying"))
                    return false;

                QVariant data_for_ai = "jiuyuan";
                if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                    RemoveEdict(liuxie);
                    room->acquireSkill(liuxie, "jiuyuan");
                }
                break;
            }
        case EventPhaseStart: {
                if (liuxie->getPhase() != Player::Discard)
                    return false;
                QVariant data_for_ai = "xueyi";
                if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                    room->broadcastSkillInvoke("hantong", 2);
                    RemoveEdict(liuxie);
                    room->acquireSkill(liuxie, "xueyi");
                }
                break;
            }
        default:
            break;
        }
        liuxie->tag["Hantong_use"] = true;
        return false;
    }
};

class HantongDetach: public TriggerSkill {
public:
    HantongDetach(): TriggerSkill("#hantong-detach") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;

        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!p->tag.value("Hantong_use", false).toBool())
                continue;
            room->handleAcquireDetachSkills(p, "-hujia|-jijiang|-jiuyuan|-xueyi", true);
            p->tag.remove("Hantong_use");
        }
        return false;
    }
};

const Card *HantongCard::validate(CardUseStruct &cardUse) const{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *source = cardUse.from;
    Room *room = source->getRoom();

    HantongAcquire::RemoveEdict(source);
    source->tag["Hantong_use"] = true;
    room->acquireSkill(source, "jijiang");
    if (!room->askForUseCard(source, "@jijiang", "@hantong-jijiang")) {
        room->setPlayerFlag(source, "Global_JijiangFailed");
        return NULL;
    } else
        return this;
}

void HantongCard::onUse(Room *, const CardUseStruct &) const{
}

DIYYicongCard::DIYYicongCard() {
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
}

void DIYYicongCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    source->addToPile("retinue", this);
}

class DIYYicongViewAsSkill: public ViewAsSkill {
public:
    DIYYicongViewAsSkill(): ViewAsSkill("diyyicong") {
        response_pattern = "@@diyyicong";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 0)
            return NULL;

        Card *acard = new DIYYicongCard;
        acard->addSubcards(cards);
        return acard;
    }
};

class DIYYicong: public TriggerSkill {
public:
    DIYYicong(): TriggerSkill("diyyicong") {
        events << EventPhaseEnd;
        view_as_skill = new DIYYicongViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *gongsunzan, QVariant &) const{
        if (gongsunzan->getPhase() == Player::Discard && !gongsunzan->isNude()) {
            room->askForUseCard(gongsunzan, "@@diyyicong", "@diyyicong", -1, Card::MethodNone);
        }
        return false;
    }
};

class DIYYicongDistance: public DistanceSkill {
public:
    DIYYicongDistance(): DistanceSkill("#diyyicong-dist") {
    }

    virtual int getCorrect(const Player *, const Player *to) const{
        if (to->hasSkill("diyyicong"))
            return to->getPile("retinue").length();
        else
            return 0;
    }
};

class Tuqi: public TriggerSkill {
public:
    Tuqi(): TriggerSkill("tuqi") {
        events << EventPhaseStart << EventPhaseChanging;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *gongsunzan, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (gongsunzan->getPhase() == Player::Start && gongsunzan->getPile("retinue").length() > 0) {
                LogMessage log;
                log.from = gongsunzan;
                log.arg = objectName();
                log.type = "#TriggerSkill";
                room->sendLog(log);
                room->notifySkillInvoked(gongsunzan, objectName());

                int n = gongsunzan->getPile("retinue").length();
                room->setPlayerMark(gongsunzan, "tuqi_dist", n);
                gongsunzan->clearOnePrivatePile("retinue");

                int index = 1;

                if (n <= 2) {
                    index++;
                    gongsunzan->drawCards(1);
                }
                room->broadcastSkillInvoke(objectName(), index);
            }
        } else {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
            room->setPlayerMark(gongsunzan, "tuqi_dist", 0);
        }
        return false;
    }
};

class TuqiDistance: public DistanceSkill {
public:
    TuqiDistance(): DistanceSkill("#tuqi-dist") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasSkill("tuqi"))
            return -from->getMark("tuqi_dist");
        else
            return 0;
    }
};

BGMDIYPackage::BGMDIYPackage(): Package("BGMDIY") {
    General *diy_simazhao = new General(this, "diy_simazhao", "wei", 3); // DIY 001
    diy_simazhao->addSkill(new Zhaoxin);
    diy_simazhao->addSkill(new Langgu);

    General *diy_wangyuanji = new General(this, "diy_wangyuanji", "wei", 3, false); // DIY 002
    diy_wangyuanji->addSkill(new Fuluan);
    diy_wangyuanji->addSkill(new FuluanForbid);
    diy_wangyuanji->addSkill(new Shude);
    related_skills.insertMulti("fuluan", "#fuluan-forbid");

    General *diy_liuxie = new General(this, "diy_liuxie", "qun"); // DIY 003
    diy_liuxie->addSkill(new Huangen);
    diy_liuxie->addSkill(new Hantong);
    diy_liuxie->addSkill(new HantongAcquire);
    diy_liuxie->addSkill(new HantongDetach);
    diy_liuxie->addSkill(new DetachEffectSkill("hantong", "edict"));
    related_skills.insertMulti("hantong", "#hantong-acquire");
    related_skills.insertMulti("hantong", "#hantong-detach");
    related_skills.insertMulti("hantong", "#hantong-clear");

    General *diy_gongsunzan = new General(this, "diy_gongsunzan", "qun"); // DIY 004
    diy_gongsunzan->addSkill(new DIYYicong);
    diy_gongsunzan->addSkill(new DIYYicongDistance);
    diy_gongsunzan->addSkill(new DetachEffectSkill("diyyicong", "retinue"));
    diy_gongsunzan->addSkill(new Tuqi);
    diy_gongsunzan->addSkill(new TuqiDistance);
    related_skills.insertMulti("diyyicong", "#diyyicong-clear");
    related_skills.insertMulti("diyyicong", "#diyyicong-dist");
    related_skills.insertMulti("tuqi", "#tuqi-dist");

    General *diy_zhugeke = new General(this, "diy_zhugeke", "wu", 3, true, true);
    diy_zhugeke->addSkill("aocai");
    diy_zhugeke->addSkill("duwu");

    addMetaObject<ZhaoxinCard>();
    addMetaObject<FuluanCard>();
    addMetaObject<HuangenCard>();
    addMetaObject<HantongCard>();
    addMetaObject<DIYYicongCard>();
}

ADD_PACKAGE(BGMDIY)

