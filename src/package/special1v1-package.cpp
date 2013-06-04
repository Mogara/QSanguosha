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
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isEquipped();
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

            DummyCard *dummy = new DummyCard;
            QList <const Card *> handcards = player->getHandcards();
            foreach (const Card *card, handcards)
                dummy->addSubcard(card);

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

class Xiaoxi: public TriggerSkill {
public:
    Xiaoxi(): TriggerSkill("xiaoxi") {
        events << Debut;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *opponent = player->getNext();
        if (!opponent->isAlive())
            return false;
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_huwei");
        if (player->isLocked(slash) || !player->canSlash(opponent, slash, false)) {
            delete slash;
            return false;
        }
        if (room->askForSkillInvoke(player, objectName()))
            room->useCard(CardUseStruct(slash, player, opponent), false);
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

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = effect.from;
    log.arg = "cangji";
    room->sendLog(log);

    CardsMoveStruct move;
    move.from = effect.from;
    move.to = effect.to;
    move.to_place = Player::PlaceEquip;
    move.card_ids = subcards;
    room->moveCardsAtomic(move, true);

    if (effect.from->getEquips().isEmpty())
        return;
    bool loop = false;
    for (int i = 0; i < 3; i++) {
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
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@cangji";
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

        CardsMoveStruct move;
        move.card_ids = equip_list;
        move.to = player;
        move.to_place = Player::PlaceEquip;

        LogMessage log;
        log.from = player;
        log.type = "$Install";
        log.card_str = IntList2StringList(equip_list).join("+");
        room->sendLog(log);

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
            CardMoveReason reason = move.reason;
            int basic_reason = (reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON);
            if (basic_reason == CardMoveReason::S_REASON_USE || reason.m_reason == CardMoveReason::S_REASON_RESPONSE
                || (basic_reason == CardMoveReason::S_REASON_RECAST && reason.m_skillName != "weapon_recast"))
                return false;
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
            player->setMark(objectName(), player->getCardCount(true));
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

Drowning::Drowning(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("drowning");
}

bool Drowning::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    if (to_select == Self)
        return false;

    return true;
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
    kof_xuchu->addSkill(new MarkAssignSkill("@twine", 1));
    related_skills.insertMulti("xiechan", "#@twine-1");

    General *kof_zhenji = new General(this, "kof_zhenji", "wei", 3, false);
    kof_zhenji->addSkill(new KOFQingguo);
    kof_zhenji->addSkill("luoshen");

    General *kof_xiahouyuan = new General(this, "kof_xiahouyuan", "wei");
    kof_xiahouyuan->addSkill("shensu");
    kof_xiahouyuan->addSkill(new Suzi);

    General *kof_guanyu = new General(this, "kof_guanyu", "shu");
    kof_guanyu->addSkill("wusheng");
    kof_guanyu->addSkill(new Huwei);

    General *kof_machao = new General(this, "kof_machao", "shu");
    kof_machao->addSkill("tieji");
    kof_machao->addSkill(new Xiaoxi);

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
