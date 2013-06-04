#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "nostalgia.h"
#include "yjcm-package.h"
#include "settings.h"

class MoonSpearSkill: public WeaponSkill {
public:
    MoonSpearSkill(): WeaponSkill("MoonSpear") {
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

        player->setFlags("MoonspearUse");
        if (!room->askForUseCard(player, "slash", "@moon-spear-slash", -1, Card::MethodUse, false))
            player->setFlags("-MoonspearUse");

        return false;
    }
};

MoonSpear::MoonSpear(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("MoonSpear");
}

NostalgiaPackage::NostalgiaPackage()
    : Package("nostalgia")
{
    type = CardPack;

    Card *moon_spear = new MoonSpear;
    moon_spear->setParent(this);

    skills << new MoonSpearSkill;
}

// old yjcm's generals

class NosWuyan: public TriggerSkill {
public:
    NosWuyan(): TriggerSkill("noswuyan") {
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.to == effect.from)
            return false;
        if (effect.card->isNDTrick()) {
            if (effect.from && effect.from->hasSkill(objectName())) {
                LogMessage log;
                log.type = "#WuyanBaD";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(effect.from, objectName());
                room->broadcastSkillInvoke("wuyan");
                return true;
            }
            if (effect.to->hasSkill(objectName()) && effect.from) {
                LogMessage log;
                log.type = "#WuyanGooD";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(effect.to, objectName());
                room->broadcastSkillInvoke("wuyan");
                return true;
            }
        }
        return false;
    }
};

NosJujianCard::NosJujianCard() {
    mute = true;
}

void NosJujianCard::onEffect(const CardEffectStruct &effect) const{
    int n = subcardsLength();
    effect.to->drawCards(n);
    Room *room = effect.from->getRoom();
    room->broadcastSkillInvoke("jujian");

    if (n == 3) {
        QSet<Card::CardType> types;
        foreach (int card_id, effect.card->getSubcards())
            types << Sanguosha->getCard(card_id)->getTypeId();

        if (types.size() == 1) {
            LogMessage log;
            log.type = "#JujianRecover";
            log.from = effect.from;
            const Card *card = Sanguosha->getCard(subcards.first());
            log.arg = card->getType();
            room->sendLog(log);

            RecoverStruct recover;
            recover.card = this;
            recover.who = effect.from;
            room->recover(effect.from, recover);
        }
    }
}

class NosJujian: public ViewAsSkill {
public:
    NosJujian(): ViewAsSkill("nosjujian") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 3 && !Self->isJilei(to_select);
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("NosJujianCard");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        NosJujianCard *card = new NosJujianCard;
        card->addSubcards(cards);
        return card;
    }
};

class NosEnyuan: public TriggerSkill {
public:
    NosEnyuan(): TriggerSkill("nosenyuan") {
        events << HpRecover << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == HpRecover) {
            RecoverStruct recover = data.value<RecoverStruct>();
            if (recover.who && recover.who != player) {
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                room->broadcastSkillInvoke("enyuan", qrand() % 2 + 1);
                room->notifySkillInvoked(player, objectName());
                recover.who->drawCards(recover.recover);
            }
        } else if (triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *source = damage.from;
            if (source && source != player) {
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                room->broadcastSkillInvoke("enyuan", qrand() % 2 + 3);
                room->notifySkillInvoked(player, objectName());

                const Card *card = room->askForCard(source, ".|heart|.|hand", "@enyuanheart", data, Card::MethodNone);
                if (card)
                    player->obtainCard(card);
                else
                    room->loseHp(source);
            }
        }

        return false;
    }
};

NosXuanhuoCard::NosXuanhuoCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
    mute = true;
}

void NosXuanhuoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.from->getRoom();
    room->broadcastSkillInvoke("xuanhuo");
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "nosxuanhuo");
    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
    room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);

    QList<ServerPlayer *> targets = room->getOtherPlayers(effect.to);
    ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "nosxuanhuo", "@nosxuanhuo-give:" + effect.to->objectName());
    if (target != effect.from) {
        CardMoveReason reason2(CardMoveReason::S_REASON_GIVE, effect.from->objectName());
        reason2.m_playerId = target->objectName();
        room->obtainCard(target, Sanguosha->getCard(card_id), reason2, false);
    }
}

class NosXuanhuo: public OneCardViewAsSkill {
public:
    NosXuanhuo():OneCardViewAsSkill("nosxuanhuo") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("NosXuanhuoCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped() && to_select->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        NosXuanhuoCard *xuanhuoCard = new NosXuanhuoCard;
        xuanhuoCard->addSubcard(originalCard);
        return xuanhuoCard;
    }
};

class NosXuanfeng: public TriggerSkill {
public:
    NosXuanfeng(): TriggerSkill("nosxuanfeng") {
        events << CardsMoveOneTime;
        default_choice = "nothing";
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *lingtong, QVariant &data) const{
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == lingtong && move.from_places.contains(Player::PlaceEquip)) {
                QStringList choicelist;
                choicelist << "nothing";
                QList<ServerPlayer *> targets1;
                foreach (ServerPlayer *target, room->getAlivePlayers()) {
                    if (lingtong->canSlash(target, NULL, false))
                        targets1 << target;
                }
                Slash *slashx = new Slash(Card::NoSuit, 0);
                if (!targets1.isEmpty() && !lingtong->isCardLimited(slashx, Card::MethodUse))
                    choicelist << "slash";
                slashx->deleteLater();
                QList<ServerPlayer *> targets2;
                foreach (ServerPlayer *p, room->getOtherPlayers(lingtong)) {
                    if (lingtong->distanceTo(p) <= 1)
                        targets2 << p;
                }
                if (!targets2.isEmpty()) choicelist << "damage";

                QString choice = room->askForChoice(lingtong, objectName(), choicelist.join("+"));
                if (choice == "slash") {
                    ServerPlayer *target = room->askForPlayerChosen(lingtong, targets1, "nosxuanfeng_slash", "@dummy-slash");
                    room->broadcastSkillInvoke(objectName(), 1);
                    Slash *slash = new Slash(Card::NoSuit, 0);
                    slash->setSkillName(objectName());
                    room->useCard(CardUseStruct(slash, lingtong, target), false);
                } else if (choice == "damage") {
                    room->broadcastSkillInvoke(objectName(), 2);

                    LogMessage log;
                    log.type = "#InvokeSkill";
                    log.from = lingtong;
                    log.arg = objectName();
                    room->sendLog(log);
                    room->notifySkillInvoked(lingtong, objectName());

                    ServerPlayer *target = room->askForPlayerChosen(lingtong, targets2, "nosxuanfeng_damage", "@nosxuanfeng-damage");
                    room->damage(DamageStruct("nosxuanfeng", lingtong, target));
                }
            }
        }

        return false;
    }
};

class NosShangshi: public Shangshi {
public:
    NosShangshi(): Shangshi() {
        setObjectName("nosshangshi");
    }

    virtual int getMaxLostHp(ServerPlayer *zhangchunhua) const{
        return qMin(zhangchunhua->getLostHp(), zhangchunhua->getMaxHp());
    }
};

class NosFuhun: public TriggerSkill {
public:
    NosFuhun(): TriggerSkill("nosfuhun") {
        events << EventPhaseStart << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *shuangying, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && shuangying->getPhase() ==  Player::Draw && TriggerSkill::triggerable(shuangying)) {
            if (shuangying->askForSkillInvoke(objectName())) {
                int card1 = room->drawCard();
                int card2 = room->drawCard();
                bool diff = (Sanguosha->getCard(card1)->getColor() != Sanguosha->getCard(card2)->getColor());

                CardsMoveStruct move, move2;
                move.card_ids.append(card1);
                move.card_ids.append(card2);
                move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, shuangying->objectName(), "fuhun", QString());
                move.to_place = Player::PlaceTable;
                room->moveCardsAtomic(move, true);
                room->getThread()->delay();

                move2 = move;
                move2.to_place = Player::PlaceHand;
                move2.to = shuangying;
                move2.reason.m_reason = CardMoveReason::S_REASON_DRAW;
                room->moveCardsAtomic(move2, true);

                if (diff) {
                    room->handleAcquireDetachSkills(shuangying, "wusheng|paoxiao");
                    room->broadcastSkillInvoke(objectName(), qrand() % 2 + 1);
                    shuangying->setFlags(objectName());
                } else {
                    room->broadcastSkillInvoke(objectName(), 3);
                }

                return true;
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive && shuangying->hasFlag(objectName()))
                room->handleAcquireDetachSkills(shuangying, "-wusheng|-paoxiao");
        }

        return false;
    }
};

class NosGongqi: public OneCardViewAsSkill {
public:
    NosGongqi(): OneCardViewAsSkill("nosgongqi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *to_select) const{
        if (to_select->getTypeId() != Card::TypeEquip)
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY
            && Self->getWeapon() && to_select->getEffectiveId() == Self->getWeapon()->getId() && to_select->isKindOf("Crossbow"))
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard);
        slash->setSkillName(objectName());
        return slash;
    }
};

class NosGongqiTargetMod: public TargetModSkill {
public:
    NosGongqiTargetMod(): TargetModSkill("#nosgongqi-target") {
        frequency = NotFrequent;
    }

    virtual int getDistanceLimit(const Player *, const Card *card) const{
        if (card->getSkillName() == "nosgongqi")
            return 1000;
        else
            return 0;
    }
};

NosJiefanCard::NosJiefanCard() {
    target_fixed = true;
    mute = true;
}

void NosJiefanCard::use(Room *room, ServerPlayer *handang, QList<ServerPlayer *> &) const{
    ServerPlayer *current = room->getCurrent();
    if (!current || current->isDead() || current->getPhase() == Player::NotActive) return;
    ServerPlayer *who = room->getCurrentDyingPlayer();
    if (!who) return;

    handang->setFlags("NosJiefanUsed");
    room->setTag("NosJiefanTarget", QVariant::fromValue((PlayerStar)who));
    bool use_slash = room->askForUseSlashTo(handang, current, "jiefan-slash:" + current->objectName(), false);
    if (!use_slash) {
        handang->setFlags("-NosJiefanUsed");
        room->removeTag("NosJiefanTarget");
        room->setPlayerFlag(handang, "Global_NosJiefanFailed");
    }
}

class NosJiefanViewAsSkill: public ZeroCardViewAsSkill {
public:
    NosJiefanViewAsSkill(): ZeroCardViewAsSkill("nosjiefan") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (!pattern.contains("peach")) return false;
        if (player->hasFlag("Global_NosJiefanFailed")) return false;
        foreach (const Player *p, player->getSiblings()) {
            if (p->isAlive() && p->getPhase() != Player::NotActive)
                return true;
        }
        return false;
    }

    virtual const Card *viewAs() const{
        return new NosJiefanCard;
    }
};

class NosJiefan: public TriggerSkill {
public:
    NosJiefan(): TriggerSkill("nosjiefan") {
        events << DamageCaused << CardFinished << PreCardUsed;
        view_as_skill = new NosJiefanViewAsSkill;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *handang, QVariant &data) const{
        if (triggerEvent == PreCardUsed) {
            if (!handang->hasFlag("NosJiefanUsed"))
                return false;

            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash")) {
                handang->setFlags("-NosJiefanUsed");
                room->setCardFlag(use.card, "nosjiefan-slash");
            }
        } else if (triggerEvent == DamageCaused) {
            ServerPlayer *current = room->getCurrent();
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash") && damage.card->hasFlag("nosjiefan-slash")) {
                LogMessage log2;
                log2.type = "#NosJiefanPrevent";
                log2.from = handang;
                log2.to << damage.to;
                room->sendLog(log2);

                PlayerStar target = room->getTag("NosJiefanTarget").value<PlayerStar>();
                if (target && target->getHp() > 0) {
                    LogMessage log;
                    log.type = "#NosJiefanNull1";
                    log.from = target;
                    room->sendLog(log);
                } else if (target && target->isDead()) {
                    LogMessage log;
                    log.type = "#NosJiefanNull2";
                    log.from = target;
                    log.to << handang;
                    room->sendLog(log);
                } else if (handang->hasFlag("Global_PreventPeach")) {
                    LogMessage log;
                    log.type = "#NosJiefanNull3";
                    log.from = current;
                    room->sendLog(log);
                } else {
                    Peach *peach = new Peach(Card::NoSuit, 0);
                    peach->setSkillName("_jiefan");

                    room->setCardFlag(damage.card, "nosjiefan_success");
                    if ((target->getGeneralName().contains("sunquan")
                         || target->getGeneralName().contains("sunce")
                         || target->getGeneralName().contains("sunjian"))
                        && target->isLord())
                        handang->setFlags("NosJiefanToLord");
                    room->useCard(CardUseStruct(peach, handang, target));
                    handang->setFlags("-NosJiefanToLord");
                }
                return true;
            }
            return false;
        } else if (triggerEvent == CardFinished && !room->getTag("NosJiefanTarget").isNull()) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash") && use.card->hasFlag("nosjiefan-slash")) {
                if (!use.card->hasFlag("nosjiefan_success"))
                    room->setPlayerFlag(handang, "Global_NosJiefanFailed");
                room->removeTag("NosJiefanTarget");
            }
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        if (player->hasFlag("NosJiefanToLord"))
            return 2;
        else
            return 1;
    }
};

class NosQianxi: public TriggerSkill {
public:
    NosQianxi(): TriggerSkill("nosqianxi") {
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if (player->distanceTo(damage.to) == 1 && damage.card && damage.card->isKindOf("Slash")
            && damage.by_user && !damage.chain && !damage.transfer && player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = ".|heart";
            judge.good = false;
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);
            if (judge.isGood()) {
                room->broadcastSkillInvoke(objectName(), 2);
                room->loseMaxHp(damage.to);
                return true;
            } else
                room->broadcastSkillInvoke(objectName(), 3);
        }
        return false;
    }
};

class NosZhenlie: public TriggerSkill {
public:
    NosZhenlie(): TriggerSkill("noszhenlie") {
        events << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if (judge->who != player)
            return false;

        if (player->askForSkillInvoke(objectName(), data)) {
            int card_id = room->drawCard();
            room->broadcastSkillInvoke(objectName(), room->getCurrent() == player ? 2 : 1);
            room->getThread()->delay();
            const Card *card = Sanguosha->getCard(card_id);

            room->retrial(card, player, judge, objectName());
        }
        return false;
    }
};

class NosMiji: public PhaseChangeSkill {
public:
    NosMiji(): PhaseChangeSkill("nosmiji") {
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *wangyi) const{
        if (!wangyi->isWounded())
            return false;
        if (wangyi->getPhase() == Player::Start || wangyi->getPhase() == Player::Finish) {
            if (!wangyi->askForSkillInvoke(objectName()))
                return false;
            Room *room = wangyi->getRoom();
            room->broadcastSkillInvoke(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = ".|black";
            judge.good = true;
            judge.reason = objectName();
            judge.who = wangyi;

            room->judge(judge);

            if (judge.isGood() && wangyi->isAlive()) {
                QList<int> pile_ids = room->getNCards(wangyi->getLostHp(), false);
                room->fillAG(pile_ids, wangyi);
                ServerPlayer *target = room->askForPlayerChosen(wangyi, room->getAllPlayers(), objectName());
                room->clearAG(wangyi);
                if (target == wangyi)
                    room->broadcastSkillInvoke(objectName(), 2);
                else if (target->getGeneralName().contains("machao"))
                    room->broadcastSkillInvoke(objectName(), 4);
                else
                    room->broadcastSkillInvoke(objectName(), 3);

                DummyCard *dummy = new DummyCard;
                foreach (int id, pile_ids)
                    dummy->addSubcard(id);
                wangyi->setFlags("Global_GongxinOperator");
                target->obtainCard(dummy, false);
                wangyi->setFlags("-Global_GongxinOperator");
                delete dummy;
            }
        }
        return false;
    }
};


// old stantard generals

NosRendeCard::NosRendeCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void NosRendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->broadcastSkillInvoke("rende");
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName());
    reason.m_playerId = target->objectName();
    room->obtainCard(target, this, reason, false);

    int old_value = source->getMark("nosrende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "nosrende", new_value);

    if (old_value < 2 && new_value >= 2) {
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

class NosRendeViewAsSkill: public ViewAsSkill {
public:
    NosRendeViewAsSkill(): ViewAsSkill("nosrende") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (ServerInfo.GameMode == "04_1v3" && selected.length() + Self->getMark("nosrende") >= 2)
            return false;
        else
            return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (ServerInfo.GameMode == "04_1v3" && player->getMark("nosrende") >= 2)
           return false;
        return !player->isKongcheng();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        NosRendeCard *rende_card = new NosRendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class NosRende: public TriggerSkill {
public:
    NosRende(): TriggerSkill("nosrende") {
        events << EventPhaseChanging;
        view_as_skill = new NosRendeViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getMark("nosrende") > 0;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        room->setPlayerMark(player, "nosrende", 0);
        return false;
    }
};

class NosJizhi: public TriggerSkill {
public:
    NosJizhi(): TriggerSkill("nosjizhi") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *yueying, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isNDTrick() && room->askForSkillInvoke(yueying, objectName())) {
            room->broadcastSkillInvoke("jizhi");
            yueying->drawCards(1);
        }

        return false;
    }
};

class NosQicai: public TargetModSkill {
public:
    NosQicai(): TargetModSkill("nosqicai") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

NosLijianCard::NosLijianCard(): LijianCard(false) {
}

class NosLijian: public OneCardViewAsSkill {
public:
    NosLijian(): OneCardViewAsSkill("noslijian") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("NosLijianCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        NosLijianCard *lijian_card = new NosLijianCard;
        lijian_card->addSubcard(originalCard->getId());
        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        return card->isKindOf("Duel") ? 0 : -1;
    }
};

NostalStandardPackage::NostalStandardPackage()
    : Package("nostal_standard")
{
    General *nos_liubei = new General(this, "nos_liubei$", "shu");
    nos_liubei->addSkill(new NosRende);
    nos_liubei->addSkill("jijiang");

    General *huangyueying = new General(this, "nos_huangyueying", "shu", 3, false);
    huangyueying->addSkill(new NosJizhi);
    huangyueying->addSkill(new NosQicai);
    huangyueying->addSkill(new SPConvertSkill("nos_huangyueying", "heg_huangyueying+tw_huangyueying"));

    General *nos_diaochan = new General(this, "nos_diaochan", "qun", 3, false);
    nos_diaochan->addSkill(new NosLijian);
    nos_diaochan->addSkill("biyue");

    addMetaObject<NosRendeCard>();
    addMetaObject<NosLijianCard>();
}

NostalYJCMPackage::NostalYJCMPackage()
    : Package("nostal_yjcm")
{
    General *nos_fazheng = new General(this, "nos_fazheng", "shu", 3);
    nos_fazheng->addSkill(new NosEnyuan);
    nos_fazheng->addSkill(new NosXuanhuo);

    General *nos_lingtong = new General(this, "nos_lingtong", "wu");
    nos_lingtong->addSkill(new NosXuanfeng);
    nos_lingtong->addSkill(new SlashNoDistanceLimitSkill("nosxuanfeng"));
    related_skills.insertMulti("nosxuanfeng", "#nosxuanfeng-slash-ndl");

    General *nos_xushu = new General(this, "nos_xushu", "shu", 3);
    nos_xushu->addSkill(new NosWuyan);
    nos_xushu->addSkill(new NosJujian);

    General *nos_zhangchunhua = new General(this, "nos_zhangchunhua", "wei", 3, false);
    nos_zhangchunhua->addSkill("jueqing");
    nos_zhangchunhua->addSkill(new NosShangshi);

    addMetaObject<NosXuanhuoCard>();
    addMetaObject<NosJujianCard>();
}

NostalYJCM2012Package::NostalYJCM2012Package()
    : Package("nostal_yjcm2012")
{
    General *nos_guanxingzhangbao = new General(this, "nos_guanxingzhangbao", "shu");
    nos_guanxingzhangbao->addSkill(new NosFuhun);

    General *nos_handang = new General(this, "nos_handang", "wu");
    nos_handang->addSkill(new NosGongqi);
    nos_handang->addSkill(new NosGongqiTargetMod);
    nos_handang->addSkill(new NosJiefan);
    related_skills.insertMulti("nosgongqi", "#nosgongqi-target");

    General *nos_madai = new General(this, "nos_madai", "shu");
    nos_madai->addSkill("mashu");
    nos_madai->addSkill(new NosQianxi);

    General *nos_wangyi = new General(this, "nos_wangyi", "wei", 3, false);
    nos_wangyi->addSkill(new NosZhenlie);
    nos_wangyi->addSkill(new NosMiji);

    addMetaObject<NosJiefanCard>();
}

ADD_PACKAGE(Nostalgia)
ADD_PACKAGE(NostalStandard)
ADD_PACKAGE(NostalYJCM)
ADD_PACKAGE(NostalYJCM2012)

