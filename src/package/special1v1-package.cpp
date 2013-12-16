#include "special1v1-package.h"
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

class KOFTuxi: public DrawCardsSkill {
public:
    KOFTuxi(): DrawCardsSkill("koftuxi") {
    }

    virtual int getDrawNum(ServerPlayer *zhangliao, int n) const{
        Room *room = zhangliao->getRoom();
        bool can_invoke = false;
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao))
            if (p->getHandcardNum() > zhangliao->getHandcardNum())
                targets << p;
        if (!targets.isEmpty())
            can_invoke = true;

        if (can_invoke) {
            ServerPlayer *target = room->askForPlayerChosen(zhangliao, targets, objectName(), "koftuxi-invoke", true, true);
            if (target) {
                target->setFlags("KOFTuxiTarget");
                zhangliao->setFlags("koftuxi");
                return n - 1;
            } else {
                return n;
            }
        } else
            return n;
    }
};

class KOFTuxiAct: public TriggerSkill {
public:
    KOFTuxiAct(): TriggerSkill("#koftuxi") {
        events << AfterDrawNCards;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhangliao, QVariant &) const{
        if (!zhangliao->hasFlag("koftuxi")) return false;
        zhangliao->setFlags("-koftuxi");

        ServerPlayer *target = NULL;
        foreach (ServerPlayer *p, room->getOtherPlayers(zhangliao)) {
            if (p->hasFlag("KOFTuxiTarget")) {
                p->setFlags("-KOFTuxiTarget");
                target = p;
                break;
            }
        }
        if (!target) return false;

        int card_id = room->askForCardChosen(zhangliao, target, "h", "koftuxi");
        room->broadcastSkillInvoke("tuxi");
        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, zhangliao->objectName());
        room->obtainCard(zhangliao, Sanguosha->getCard(card_id), reason, false);

        return false;
    }
};

XiechanCard::XiechanCard() {
}

bool XiechanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void XiechanCard::use(Room *room, ServerPlayer *xuchu, QList<ServerPlayer *> &targets) const{
    room->removePlayerMark(xuchu, "@twine");
    room->doLightbox("$XiechanAnimate");

    bool success = xuchu->pindian(targets.first(), "xiechan", NULL);
    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("_xiechan");
    ServerPlayer *from = NULL, *to = NULL;
    if (success) {
        from = xuchu;
        to = targets.first();
    } else {
        from = targets.first();
        to = xuchu;
    }
    if (!from->isLocked(duel) && !from->isProhibited(to, duel))
        room->useCard(CardUseStruct(duel, from, to));
}

class Xiechan: public ZeroCardViewAsSkill {
public:
    Xiechan(): ZeroCardViewAsSkill("xiechan") {
        frequency = Limited;
        limit_mark = "@twine";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@twine") > 0;
    }

    virtual const Card *viewAs() const{
        return new XiechanCard;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (card->isKindOf("Duel"))
            return -2;
        else
            return -1;
    }
};

class KOFQingguo: public OneCardViewAsSkill {
public:
    KOFQingguo(): OneCardViewAsSkill("kofqingguo") {
        filter_pattern = ".|.|.|equipped";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(originalCard->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" && !player->getEquips().isEmpty();
    }
};

class Suzi: public TriggerSkill {
public:
    Suzi(): TriggerSkill("suzi") {
        events << BuryVictim;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->isNude())
            return false;
        ServerPlayer *xiahouyuan = room->findPlayerBySkillName(objectName());
        DeathStruct death = data.value<DeathStruct>();
        if (!xiahouyuan || xiahouyuan == death.who)
            return false;
        if (room->askForSkillInvoke(xiahouyuan, objectName(), data)) {
            room->broadcastSkillInvoke(objectName());

            DummyCard *dummy = new DummyCard(player->handCards());
            QList <const Card *> equips = player->getEquips();
            foreach (const Card *card, equips)
                dummy->addSubcard(card);

            if (dummy->subcardsLength() > 0) {
                CardMoveReason reason(CardMoveReason::S_REASON_RECYCLE, xiahouyuan->objectName());
                room->obtainCard(xiahouyuan, dummy, reason, false);
            }
            delete dummy;
        }
        return false;
    }
};

class Huwei: public TriggerSkill {
public:
    Huwei(): TriggerSkill("huwei") {
        events << Debut;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *opponent = player->getNext();
        if (!opponent->isAlive())
            return false;
        Drowning *drowning = new Drowning(Card::NoSuit, 0);
        drowning->setSkillName("_huwei");
        if (!drowning->isAvailable(player) || player->isProhibited(opponent, drowning)) {
            delete drowning;
            return false;
        }
        if (room->askForSkillInvoke(player, objectName()))
            room->useCard(CardUseStruct(drowning, player, opponent), false);
        return false;
    }
};

CangjiCard::CangjiCard() {
    will_throw = false;
}

bool CangjiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;
    QList<int> equip_loc;
    foreach (int id, subcards) {
        const Card *card = Sanguosha->getCard(id);
        const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
        if (equip)
            equip_loc << equip->location();
    }
    foreach (int loc, equip_loc) {
        if (to_select->getEquip(loc))
            return false;
    }
    return true;
}

void CangjiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    CardsMoveStruct move(subcards, effect.from, effect.to, Player::PlaceUnknown, Player::PlaceEquip, CardMoveReason());
    room->moveCardsAtomic(move, true);

    if (effect.from->getEquips().isEmpty())
        return;
    bool loop = false;
    for (int i = 0; i <= 3; i++) {
        if (effect.from->getEquip(i)) {
            foreach (ServerPlayer *p, room->getOtherPlayers(effect.from)) {
                if (!p->getEquip(i)) {
                    loop = true;
                    break;
                }
            }
            if (loop) break;
        }
    }
    if (loop)
        room->askForUseCard(effect.from, "@@cangji", "@cangji-install", -1, Card::MethodNone);
}

class CangjiViewAsSkill: public ViewAsSkill {
public:
    CangjiViewAsSkill(): ViewAsSkill("cangji") {
        response_pattern = "@@cangji";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        CangjiCard *card = new CangjiCard;
        card->addSubcards(cards);
        return card;
    }
};

class Cangji: public TriggerSkill {
public:
    Cangji(): TriggerSkill("cangji") {
        events << Death;
        view_as_skill = new CangjiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player || !player->hasSkill(objectName()) || player->getEquips().isEmpty())
            return false;
        if (room->getMode() == "02_1v1") {
            if (room->askForSkillInvoke(player, objectName())) {
                QVariantList equip_list;
                CardsMoveStruct move;
                move.from = player;
                move.to = NULL;
                move.to_place = Player::PlaceTable;

                foreach (const Card *equip, player->getEquips()) {
                    equip_list.append(QVariant(equip->getEffectiveId()));
                    move.card_ids.append(equip->getEffectiveId());
                }
                player->tag[objectName()] = QVariant::fromValue(equip_list);
                room->moveCardsAtomic(move, true);
            }
        } else {
            room->askForUseCard(player, "@@cangji", "@cangji-install", -1, Card::MethodNone);
        }
        return false;
    }
};

class CangjiInstall: public TriggerSkill {
public:
    CangjiInstall(): TriggerSkill("#cangji-install") {
        events << Debut;
    }

    virtual int getPriority() const{
        return 5;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->tag["cangji"].isNull();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        QList<int> equip_list;
        foreach (QVariant id, player->tag["cangji"].toList()) {
            int card_id = id.toInt();
            if (Sanguosha->getCard(card_id)->getTypeId() == Card::TypeEquip)
                equip_list << card_id;
        }
        player->tag.remove("cangji");
        if (equip_list.isEmpty())
            return false;

        LogMessage log;
        log.from = player;
        log.type = "$Install";
        log.card_str = IntList2StringList(equip_list).join("+");
        room->sendLog(log);

        CardsMoveStruct move(equip_list, player, Player::PlaceEquip, CardMoveReason());
        room->moveCardsAtomic(move, true);
        return false;
    }
};

class KOFLiegong: public TriggerSkill {
public:
    KOFLiegong(): TriggerSkill("kofliegong") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (player != use.from || player->getPhase() != Player::Play || !use.card->isKindOf("Slash"))
            return false;
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        int index = 0;
        foreach (ServerPlayer *p, use.to) {
            int handcardnum = p->getHandcardNum();
            if (player->getHp() <= handcardnum && player->askForSkillInvoke(objectName(), QVariant::fromValue(p))) {
                room->broadcastSkillInvoke("liegong");

                LogMessage log;
                log.type = "#NoJink";
                log.from = p;
                room->sendLog(log);
                jink_list.replace(index, QVariant(0));
            }
            index++;
        }
        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }
};

class Manyi: public TriggerSkill {
public:
    Manyi(): TriggerSkill("manyi") {
        events << Debut;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *opponent = player->getNext();
        if (!opponent->isAlive())
            return false;
        SavageAssault *savage_assault = new SavageAssault(Card::NoSuit, 0);
        savage_assault->setSkillName("_manyi");
        if (!savage_assault->isAvailable(player)) {
            delete savage_assault;
            return false;
        }
        if (room->askForSkillInvoke(player, objectName()))
            room->useCard(CardUseStruct(savage_assault, player, NULL));
        return false;
    }
};

class ManyiAvoid: public TriggerSkill {
public:
    ManyiAvoid(): TriggerSkill("#manyi-avoid") {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.card->isKindOf("SavageAssault")) {
            room->broadcastSkillInvoke(player->isFemale() ? "juxiang" : "huoshou");

            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = "manyi";
            log.arg2 = "savage_assault";
            room->sendLog(log);

            return true;
        } else
            return false;
    }
};

class KOFXiaoji: public TriggerSkill {
public:
    KOFXiaoji(): TriggerSkill("kofxiaoji") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == sunshangxiang && move.from_places.contains(Player::PlaceEquip)) {
            for (int i = 0; i < move.card_ids.size(); i++) {
                if (!sunshangxiang->isAlive())
                    return false;
                if (move.from_places[i] == Player::PlaceEquip) {
                    QStringList choicelist;
                    choicelist << "draw" << "cancel";
                    if (sunshangxiang->isWounded())
                        choicelist.prepend("recover");
                    QString choice = room->askForChoice(sunshangxiang, objectName(), choicelist.join("+"));
                    if (choice == "cancel")
                        return false;
                    room->broadcastSkillInvoke("xiaoji");
                    room->notifySkillInvoked(sunshangxiang, objectName());

                    LogMessage log;
                    log.type = "#InvokeSkill";
                    log.from = sunshangxiang;
                    log.arg = objectName();
                    room->sendLog(log);
                    if (choice == "draw")
                        sunshangxiang->drawCards(2);
                    else {
                        RecoverStruct recover;
                        recover.who = sunshangxiang;
                        room->recover(sunshangxiang, recover);
                    }
                }
            }
        }
        return false;
    }
};

class Yinli: public TriggerSkill {
public:
    Yinli(): TriggerSkill("yinli") {
        events << BeforeCardsMove;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == sunshangxiang || move.from == NULL)
            return false;
        if (move.from->getPhase() != Player::NotActive && move.to_place == Player::DiscardPile) {
            QList<int> card_ids;
            int i = 0;
            foreach (int card_id, move.card_ids) {
                if (Sanguosha->getCard(card_id)->getTypeId() == Card::TypeEquip
                    && room->getCardOwner(card_id) == move.from
                    && (room->getCardPlace(card_id) == Player::PlaceHand || room->getCardPlace(card_id) == Player::PlaceEquip))
                    card_ids << card_id;
                i++;
            }
            if (card_ids.empty())
                return false;
            else if (sunshangxiang->askForSkillInvoke(objectName(), data)) {
                int ai_delay = Config.AIDelay;
                Config.AIDelay = 0;
                while (!card_ids.empty()) {
                    room->fillAG(card_ids, sunshangxiang);
                    int id = room->askForAG(sunshangxiang, card_ids, true, objectName());
                    if (id == -1) {
                        room->clearAG(sunshangxiang);
                        break;
                    }
                    card_ids.removeOne(id);
                    room->clearAG(sunshangxiang);
                }
                Config.AIDelay = ai_delay;

                if (!card_ids.empty()) {
                    room->broadcastSkillInvoke("yinli");
                    foreach (int id, card_ids) {
                        if (move.card_ids.contains(id)) {
                            move.from_places.removeAt(move.card_ids.indexOf(id));
                            move.card_ids.removeOne(id);
                            data = QVariant::fromValue(move);
                        }
                        room->moveCardTo(Sanguosha->getCard(id), sunshangxiang, Player::PlaceHand, move.reason, true);
                        if (!sunshangxiang->isAlive())
                            break;
                    }
                }
            }
        }
        return false;
    }
};

class Pianyi: public TriggerSkill {
public:
    Pianyi(): TriggerSkill("pianyi") {
        events << Debut;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *opponent = room->getOtherPlayers(player).first();
        if (opponent->getPhase() != Player::NotActive) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            LogMessage log2;
            log2.type = "#TurnBroken";
            log2.from = opponent;
            room->sendLog(log2);

            throw TurnBroken;
        }
        return false;
    }
};

MouzhuCard::MouzhuCard() {
}

bool MouzhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void MouzhuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    ServerPlayer *hejin = effect.from, *target = effect.to;
    if (target->isKongcheng()) return;

    const Card *card = NULL;
    if (target->getHandcardNum() > 1) {
        card = room->askForCard(target, ".!", "@mouzhu-give:" + hejin->objectName(), QVariant(), Card::MethodNone);
        if (!card)
            card = target->getHandcards().at(qrand() % target->getHandcardNum());
    } else {
        card = target->getHandcards().first();
    }
    Q_ASSERT(card != NULL);
    hejin->obtainCard(card, false);
    if (!hejin->isAlive() || !target->isAlive()) return;
    if (hejin->getHandcardNum() > target->getHandcardNum()) {
        QStringList choicelist;
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_mouzhu");
        Duel *duel = new Duel(Card::NoSuit, 0);
        duel->setSkillName("_mouzhu");
        if (!target->isLocked(slash) && target->canSlash(hejin, slash, false))
            choicelist.append("slash");
        if (!target->isLocked(duel) && !target->isProhibited(hejin, duel))
            choicelist.append("duel");
        if (choicelist.isEmpty()) {
            delete slash;
            delete duel;
            return;
        }
        QString choice = room->askForChoice(target, "mouzhu", choicelist.join("+"));
        CardUseStruct use;
        use.from = target;
        use.to << hejin;
        if (choice == "slash") {
            delete duel;
            use.card = slash;
        } else {
            delete slash;
            use.card = duel;
        }
        room->useCard(use);
    }
}

class Mouzhu: public ZeroCardViewAsSkill {
public:
    Mouzhu(): ZeroCardViewAsSkill("mouzhu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MouzhuCard");
    }

    virtual const Card *viewAs() const{
        return new MouzhuCard;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (card->isKindOf("MouzhuCard"))
            return -1;
        else
            return -2;
    }
};

class Yanhuo: public TriggerSkill {
public:
    Yanhuo(): TriggerSkill("yanhuo") {
        events << BeforeGameOverJudge << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->isAlive() && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == BeforeGameOverJudge) {
            player->setMark(objectName(), player->getCardCount());
        } else {
            int n = player->getMark(objectName());
            if (n == 0) return false;
            bool normal = false;
            ServerPlayer *killer = NULL;
            if (room->getMode() == "02_1v1")
                killer = room->getOtherPlayers(player).first();
            else {
                normal = true;
                QList<ServerPlayer *> targets;
                foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                    if (player->canDiscard(p, "he"))
                        targets << p;
                }
                if (!targets.isEmpty())
                    killer = room->askForPlayerChosen(player, targets, objectName(), "yanhuo-invoke", true, true);
            }
            if (killer && killer->isAlive() && player->canDiscard(killer, "he")
                && (normal || room->askForSkillInvoke(player, objectName()))) {
                for (int i = 0; i < n; i++) {
                    if (player->canDiscard(killer, "he")) {
                        int card_id = room->askForCardChosen(player, killer, "he", objectName(), false, Card::MethodDiscard);
                        room->throwCard(Sanguosha->getCard(card_id), killer, player);
                    } else {
                        break;
                    }
                }
            }
        }
        return false;
    }
};

class Renwang: public TriggerSkill {
public:
    Renwang(): TriggerSkill("renwang") {
        events << CardUsed << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardUsed && player->getPhase() == Player::Play) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash") && !use.card->isNDTrick()) return false;
            QList<ServerPlayer *> first;
            foreach (ServerPlayer *to, use.to) {
                if (to != player && !to->hasFlag("RenwangEffect")) {
                    first << to;
                    to->setFlags("RenwangEffect");
                }
            }
            foreach (ServerPlayer *p, room->getOtherPlayers(use.from)) {
                if (use.to.contains(p) && !first.contains(p) && p->canDiscard(use.from, "he")
                    && p->hasFlag("RenwangEffect") && TriggerSkill::triggerable(p)
                    && room->askForSkillInvoke(p, objectName(), data)) {
                    room->throwCard(room->askForCardChosen(p, use.from, "he", objectName(), false, Card::MethodDiscard), use.from, p);
                }
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                foreach (ServerPlayer *to, room->getAlivePlayers()) {
                    if (to->hasFlag("RenwangEffect"))
                        to->setFlags("-RenwangEffect");
                }
            }
        }
        return false;
    }
};

class KOFKuanggu: public TriggerSkill {
public:
    KOFKuanggu(): TriggerSkill("kofkuanggu") {
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (room->askForSkillInvoke(player, objectName(), data)) {
            JudgeStruct judge;
            judge.pattern = ".|black";
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);
            if (judge.isGood() && player->isWounded()) {
                room->broadcastSkillInvoke("kuanggu");

                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class Shenju: public MaxCardsSkill {
public:
    Shenju(): MaxCardsSkill("shenju") {
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasSkill(objectName()))
            return target->getMark(objectName());
        else
            return 0;
    }
};

class ShenjuMark: public TriggerSkill {
public:
    ShenjuMark(): TriggerSkill("#shenju") {
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() == Player::Discard) {
            int max_hp = -1000;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                int hp = p->getHp();
                if (hp > max_hp)
                    max_hp = hp;
            }
            player->setMark("shenju", qMax(max_hp, 0));
        }
        return false;
    }
};

class Wanrong: public TriggerSkill {
public:
    Wanrong(): TriggerSkill("wanrong") {
        events << TargetConfirmed;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && use.to.contains(player)
            && room->askForSkillInvoke(player, objectName(), data))
            player->drawCards(1);
        return false;
    }
};

PujiCard::PujiCard() {
}

bool PujiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canDiscard(to_select, "he") && to_select != Self;
}

void PujiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int id = room->askForCardChosen(effect.from, effect.to, "he", "puji");
    room->throwCard(id, effect.to, effect.from);

    if (effect.from->isAlive() && this->getSuit() == Card::Spade)
        effect.from->drawCards(1);
    if (effect.to->isAlive() && Sanguosha->getCard(id)->getSuit() == Card::Spade)
        effect.to->drawCards(1);
}

class Puji: public OneCardViewAsSkill {
public:
    Puji(): OneCardViewAsSkill("puji") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("PujiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        PujiCard *card = new PujiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Cuorui: public TriggerSkill {
public:
    Cuorui(): TriggerSkill("cuorui") {
        events << DrawInitialCards << EventPhaseChanging;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == DrawInitialCards) {
            int n = 3;
            if (room->getMode() == "02_1v1") {
                n = player->tag["1v1Arrange"].toStringList().length();
                if (Config.value("1v1/Rule", "2013").toString() != "OL")
                    n += 3;
                int origin = (Config.value("1v1/Rule", "2013").toString() != "Classical") ? 4 : player->getMaxHp();
                n += (2 - origin);
            }

            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = "cuorui";
            room->sendLog(log);
            room->broadcastSkillInvoke("cuorui");
            room->notifySkillInvoked(player, "cuorui");

            data = data.toInt() + n;
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::Judge && player->getMark("CuoruiSkipJudge") == 0) {
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = "cuorui";
                room->sendLog(log);
                room->broadcastSkillInvoke("cuorui");
                room->notifySkillInvoked(player, "cuorui");

                player->skip(Player::Judge);
                player->addMark("CuoruiSkipJudge");
            }
        }
        return false;
    }
};

class Liewei: public TriggerSkill {
public:
    Liewei(): TriggerSkill("liewei") {
        events << BuryVictim;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.damage && death.damage->from && TriggerSkill::triggerable(death.damage->from)
            && room->askForSkillInvoke(death.damage->from, objectName(), data))
            death.damage->from->drawCards(3, objectName());
        return false;
    }
};

class NiluanViewAsSkill: public OneCardViewAsSkill {
public:
    NiluanViewAsSkill(): OneCardViewAsSkill("niluan") {
        filter_pattern = ".|black";
        response_pattern = "@@niluan";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(Card::SuitToBeDecided, -1);
        slash->addSubcard(originalCard);
        slash->setSkillName("niluan");
        return slash;
    }
};

class Niluan: public TriggerSkill {
public:
    Niluan(): TriggerSkill("niluan") {
        events << EventPhaseStart;
        view_as_skill = new NiluanViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() == Player::Finish) {
            ServerPlayer *hansui = room->findPlayerBySkillName(objectName());
            if (hansui && hansui != player && hansui->canSlash(player, false)
                && (player->getHp() > hansui->getHp() || hansui->hasFlag("NiluanSlashTarget"))) {
                if (hansui->isKongcheng()) {
                    bool has_black = false;
                    for (int i = 0; i < 4; i++) {
                        const EquipCard *equip = hansui->getEquip(i);
                        if (equip && equip->isBlack()) {
                            has_black = true;
                            break;
                        }
                    }
                    if (!has_black) return false;
                }

                room->setPlayerFlag(hansui, "slashTargetFix");
                room->setPlayerFlag(hansui, "slashNoDistanceLimit");
                room->setPlayerFlag(hansui, "slashTargetFixToOne");
                room->setPlayerFlag(player, "SlashAssignee");

                const Card *slash = room->askForUseCard(hansui, "@@niluan", "@niluan-slash:" + player->objectName());
                if (slash == NULL) {
                    room->setPlayerFlag(hansui, "-slashTargetFix");
                    room->setPlayerFlag(hansui, "-slashNoDistanceLimit");
                    room->setPlayerFlag(hansui, "-slashTargetFixToOne");
                    room->setPlayerFlag(player, "-SlashAssignee");
                }
            }
        }
        return false;
    }
};

class NiluanRecord: public TriggerSkill {
public:
    NiluanRecord(): TriggerSkill("#niluan-record") {
        events << TargetConfirmed << EventPhaseStart;
    }

    virtual int getPriority() const{
        return 4;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from && use.from == player && use.card->isKindOf("Slash")) {
                foreach (ServerPlayer *to, use.to) {
                    if (!to->hasFlag("NiluanSlashTarget"))
                        to->setFlags("NiluanSlashTarget");
                }
            }
        } else if (player->getPhase() == Player::RoundStart) {
            foreach (ServerPlayer *p, room->getAlivePlayers())
                p->setFlags("-NiluanSlashTarget");
        }
        return false;
    }
};

Drowning::Drowning(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("drowning");
}

bool Drowning::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    return targets.length() < total_num && to_select != Self;
}

void Drowning::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (!effect.to->getEquips().isEmpty()
        && room->askForChoice(effect.to, objectName(), "throw+damage", QVariant::fromValue(effect)) == "throw")
        effect.to->throwAllEquips();
    else
        room->damage(DamageStruct(this, effect.from->isAlive() ? effect.from : NULL, effect.to));
}

Special1v1Package::Special1v1Package()
    : Package("Special1v1")
{
    General *kof_zhangliao = new General(this, "kof_zhangliao", "wei");
    kof_zhangliao->addSkill(new KOFTuxi);
    kof_zhangliao->addSkill(new KOFTuxiAct);
    related_skills.insertMulti("koftuxi", "#koftuxi");

    General *kof_xuchu = new General(this, "kof_xuchu", "wei");
    kof_xuchu->addSkill("luoyi");
    kof_xuchu->addSkill(new Xiechan);

    General *kof_zhenji = new General(this, "kof_zhenji", "wei", 3, false);
    kof_zhenji->addSkill(new KOFQingguo);
    kof_zhenji->addSkill("luoshen");

    General *kof_xiahouyuan = new General(this, "kof_xiahouyuan", "wei");
    kof_xiahouyuan->addSkill("shensu");
    kof_xiahouyuan->addSkill(new Suzi);

    General *kof_guanyu = new General(this, "kof_guanyu", "shu");
    kof_guanyu->addSkill("wusheng");
    kof_guanyu->addSkill(new Huwei);

    General *kof_nos_huangyueying = new General(this, "kof_nos_huangyueying", "shu", 3, false);
    kof_nos_huangyueying->addSkill("nosjizhi");
    kof_nos_huangyueying->addSkill(new Cangji);
    kof_nos_huangyueying->addSkill(new CangjiInstall);
    related_skills.insertMulti("cangji", "#cangji-install");

    General *kof_huangzhong = new General(this, "kof_huangzhong", "shu");
    kof_huangzhong->addSkill(new KOFLiegong);

    General *kof_jiangwei = new General(this, "kof_jiangwei", "shu");
    kof_jiangwei->addSkill("tiaoxin");

    General *kof_menghuo = new General(this, "kof_menghuo", "shu");
    kof_menghuo->addSkill(new Manyi);
    kof_menghuo->addSkill(new ManyiAvoid);
    kof_menghuo->addSkill("zaiqi");
    related_skills.insertMulti("manyi", "#manyi-avoid");

    General *kof_zhurong = new General(this, "kof_zhurong", "shu", 4, false);
    kof_zhurong->addSkill("manyi");
    kof_zhurong->addSkill("lieren");

    General *kof_sunshangxiang = new General(this, "kof_sunshangxiang", "wu", 3, false);
    kof_sunshangxiang->addSkill(new Yinli);
    kof_sunshangxiang->addSkill(new KOFXiaoji);

    General *kof_nos_diaochan = new General(this, "kof_nos_diaochan", "qun", 3, false);
    kof_nos_diaochan->addSkill(new Pianyi);
    kof_nos_diaochan->addSkill("biyue");

    General *hejin = new General(this, "hejin", "qun", 4); // QUN 025
    hejin->addSkill(new Mouzhu);
    hejin->addSkill(new Yanhuo);

    addMetaObject<XiechanCard>();
    addMetaObject<CangjiCard>();
    addMetaObject<MouzhuCard>();
}

ADD_PACKAGE(Special1v1)

Special1v1OLPackage::Special1v1OLPackage()
    : Package("Special1v1OL")
{
    General *kof_liubei = new General(this, "kof_liubei$", "shu");
    kof_liubei->addSkill(new Renwang);
    kof_liubei->addSkill("jijiang");

    General *kof_weiyan = new General(this, "kof_weiyan", "shu");
    kof_weiyan->addSkill(new KOFKuanggu);

    General *kof_lvmeng = new General(this, "kof_lvmeng", "wu");
    kof_lvmeng->addSkill(new Shenju);
    kof_lvmeng->addSkill(new ShenjuMark);
    related_skills.insertMulti("shenju", "#shenju");

    General *kof_daqiao = new General(this, "kof_daqiao", "wu", 3, false);
    kof_daqiao->addSkill("guose");
    kof_daqiao->addSkill(new Wanrong);

    General *kof_huatuo = new General(this, "kof_huatuo", "qun", 3);
    kof_huatuo->addSkill("jijiu");
    kof_huatuo->addSkill(new Puji);

    addMetaObject<PujiCard>();
}

ADD_PACKAGE(Special1v1OL)

Special1v1ExtPackage::Special1v1ExtPackage()
    : Package("Special1v1Ext")
{
    General *niujin = new General(this, "niujin", "wei"); // WEI 025
    niujin->addSkill(new Cuorui);
    niujin->addSkill(new Liewei);

    General *hansui = new General(this, "hansui", "qun"); // QUN 027
    hansui->addSkill("mashu");
    hansui->addSkill(new Niluan);
    hansui->addSkill(new NiluanRecord);
    related_skills.insertMulti("niluan", "#niluan-record");
}

ADD_PACKAGE(Special1v1Ext)

#include "maneuvering.h"
New1v1CardPackage::New1v1CardPackage()
    : Package("New1v1Card")
{
    QList<Card *> cards;
    cards << new Duel(Card::Spade, 1)
          << new EightDiagram(Card::Spade, 2)
          << new Dismantlement(Card::Spade, 3)
          << new Snatch(Card::Spade, 4)
          << new Slash(Card::Spade, 5)
          << new QinggangSword(Card::Spade, 6)
          << new Slash(Card::Spade, 7)
          << new Slash(Card::Spade, 8)
          << new IceSword(Card::Spade, 9)
          << new Slash(Card::Spade, 10)
          << new Snatch(Card::Spade, 11)
          << new Spear(Card::Spade, 12)
          << new SavageAssault(Card::Spade, 13);

    cards << new ArcheryAttack(Card::Heart, 1)
          << new Jink(Card::Heart, 2)
          << new Peach(Card::Heart, 3)
          << new Peach(Card::Heart, 4)
          << new Jink(Card::Heart, 5)
          << new Indulgence(Card::Heart, 6)
          << new ExNihilo(Card::Heart, 7)
          << new ExNihilo(Card::Heart, 8)
          << new Peach(Card::Heart, 9)
          << new Slash(Card::Heart, 10)
          << new Slash(Card::Heart, 11)
          << new Dismantlement(Card::Heart, 12)
          << new Nullification(Card::Heart, 13);

    cards << new Duel(Card::Club, 1)
          << new RenwangShield(Card::Club, 2)
          << new Dismantlement(Card::Club, 3)
          << new Slash(Card::Club, 4)
          << new Slash(Card::Club, 5)
          << new Slash(Card::Club, 6)
          << new Drowning(Card::Club, 7)
          << new Slash(Card::Club, 8)
          << new Slash(Card::Club, 9)
          << new Slash(Card::Club, 10)
          << new Slash(Card::Club, 11)
          << new SupplyShortage(Card::Club, 12)
          << new Nullification(Card::Club, 13);

    cards << new Crossbow(Card::Diamond, 1)
          << new Jink(Card::Diamond, 2)
          << new Jink(Card::Diamond, 3)
          << new Snatch(Card::Diamond, 4)
          << new Axe(Card::Diamond, 5)
          << new Slash(Card::Diamond, 6)
          << new Jink(Card::Diamond, 7)
          << new Jink(Card::Diamond, 8)
          << new Slash(Card::Diamond, 9)
          << new Jink(Card::Diamond, 10)
          << new Jink(Card::Diamond, 11)
          << new Peach(Card::Diamond, 12)
          << new Slash(Card::Diamond, 13);

    foreach (Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(New1v1Card)
