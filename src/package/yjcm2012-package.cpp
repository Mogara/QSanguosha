#include "yjcm2012-package.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "maneuvering.h"

class Zhenlie: public TriggerSkill {
public:
    Zhenlie(): TriggerSkill("zhenlie") {
        events << TargetConfirmed << CardEffected << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            if (TriggerSkill::triggerable(player)) {
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.to.contains(player) && use.from != player) {
                    if (use.card->isKindOf("Slash") || use.card->isNDTrick()) {
                        if (room->askForSkillInvoke(player, objectName(), data)) {
                            room->broadcastSkillInvoke(objectName());
                            room->setCardFlag(use.card, "ZhenlieNullify");
                            player->setFlags("ZhenlieTarget");
                            room->loseHp(player);
                            if (player->isAlive() && player->hasFlag("ZhenlieTarget") && player->canDiscard(use.from, "he")) {
                                int id = room->askForCardChosen(player, use.from, "he", objectName(), false, Card::MethodDiscard);
                                room->throwCard(id, use.from, player);
                            }
                        }
                    }
                }
            }
        } else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (!effect.card->isKindOf("Slash") && effect.card->hasFlag("ZhenlieNullify") && player->hasFlag("ZhenlieTarget")) {
                player->setFlags("-ZhenlieTarget");

                LogMessage log;
                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);

                return true;
            }
        } else if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.slash->hasFlag("ZhenlieNullify") && player->hasFlag("ZhenlieTarget")) {
                player->setFlags("-ZhenlieTarget");

                LogMessage log;
                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.slash->objectName();
                log.arg2 = objectName();
                room->sendLog(log);

                return true;
            }
        }
        return false;
    }
};

class Miji: public PhaseChangeSkill {
public:
    Miji(): PhaseChangeSkill("miji") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if (target->getPhase() == Player::Finish && target->isWounded() && target->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName(), 1);
            QStringList draw_num;
            for (int i = 1; i <= target->getLostHp(); draw_num << QString::number(i++)) {}
            int num = room->askForChoice(target, "miji_draw", draw_num.join("+")).toInt();
            target->drawCards(num, objectName());

            if (!target->isKongcheng()) {
                int n = 0;
                forever {
                    int original_handcardnum = target->getHandcardNum();
                    if (n < num && !target->isKongcheng()) {
                        QList<int> handcards = target->handCards();
                        if (!room->askForYiji(target, handcards, objectName(), false, false, false, num - n))
                            break;
                        n += original_handcardnum - target->getHandcardNum();
                    } else {
                        break;
                    }
                }
                // give the rest cards randomly
                if (n < num && !target->isKongcheng()) {
                    int rest_num = num - n;
                    forever {
                        QList<int> handcard_list = target->handCards();
                        qShuffle(handcard_list);
                        int give = qrand() % rest_num + 1;
                        rest_num -= give;
                        QList<int> to_give = handcard_list.length() < give ? handcard_list : handcard_list.mid(0, give);
                        ServerPlayer *receiver = room->getOtherPlayers(target).at(qrand() % (target->aliveCount() - 1));
                        DummyCard *dummy = new DummyCard;
                        foreach (int id, to_give)
                            dummy->addSubcard(id);
                        room->obtainCard(receiver, dummy, false);
                        delete dummy;
                        if (rest_num == 0 || target->isKongcheng())
                            break;
                    }
                }
                room->broadcastSkillInvoke(objectName(), qrand() % 2 + 2);
            }
        }
        return false;
    }
};

QiceCard::QiceCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool QiceCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag.value("qice").value<CardStar>();
    Card *mutable_card = const_cast<Card *>(card);
    foreach (int id, subcards)
        mutable_card->addSubcard(id);
    return mutable_card && mutable_card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, mutable_card, targets);
}

bool QiceCard::targetFixed() const{
    CardStar card = Self->tag.value("qice").value<CardStar>();
    Card *mutable_card = const_cast<Card *>(card);
    foreach (int id, subcards)
        mutable_card->addSubcard(id);
    return mutable_card && mutable_card->targetFixed();
}

bool QiceCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag.value("qice").value<CardStar>();
    Card *mutable_card = const_cast<Card *>(card);
    foreach (int id, subcards)
        mutable_card->addSubcard(id);
    return mutable_card && mutable_card->targetsFeasible(targets, Self);
}

const Card *QiceCard::validate(CardUseStruct &card_use) const{
    Card *use_card = Sanguosha->cloneCard(user_string);
    use_card->setSkillName("qice");
    foreach (int id, this->getSubcards())
        use_card->addSubcard(id);
    bool available = true;
    foreach (ServerPlayer *to, card_use.to)
        if (card_use.from->isProhibited(to, use_card)) {
            available = false;
            break;
        }
    available = available && use_card->isAvailable(card_use.from);
    use_card->deleteLater();
    if (!available) return NULL;
    return use_card;
}

class Qice: public ViewAsSkill {
public:
    Qice(): ViewAsSkill("qice") {
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("qice", false);
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;

        CardStar c = Self->tag.value("qice").value<CardStar>();
        if (c) {
            QiceCard *card = new QiceCard;
            card->setUserString(c->objectName());
            card->addSubcards(cards);
            return card;
        } else
            return NULL;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (player->isKongcheng())
            return false;
        else
            return !player->hasUsed("QiceCard");
    }
};

class Zhiyu: public MasochismSkill {
public:
    Zhiyu(): MasochismSkill("zhiyu") {
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if (target->askForSkillInvoke(objectName(), QVariant::fromValue(damage))) {
            target->drawCards(1);

            Room *room = target->getRoom();
            room->broadcastSkillInvoke(objectName());

            if (target->isKongcheng())
                return;
            room->showAllCards(target);

            QList<const Card *> cards = target->getHandcards();
            Card::Color color = cards.first()->getColor();
            bool same_color = true;
            foreach (const Card *card, cards) {
                if (card->getColor() != color) {
                    same_color = false;
                    break;
                }
            }

            if (same_color && damage.from && damage.from->canDiscard(damage.from, "h"))
                room->askForDiscard(damage.from, objectName(), 1, 1);
        }
    }
};

class Jiangchi: public DrawCardsSkill {
public:
    Jiangchi(): DrawCardsSkill("jiangchi") {
    }

    virtual int getDrawNum(ServerPlayer *caozhang, int n) const{
        Room *room = caozhang->getRoom();
        QString choice = room->askForChoice(caozhang, objectName(), "jiang+chi+cancel");
        if (choice == "cancel")
            return n;

        room->notifySkillInvoked(caozhang, objectName());
        LogMessage log;
        log.from = caozhang;
        log.arg = objectName();
        if (choice == "jiang") {
            log.type = "#Jiangchi1";
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName(), 1);
            room->setPlayerCardLimitation(caozhang, "use,response", "Slash", true);
            return n + 1;
        } else {
            log.type = "#Jiangchi2";
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName(), 2);
            room->setPlayerFlag(caozhang, "JiangchiInvoke");
            return n - 1;
        }
    }
};

class JiangchiTargetMod: public TargetModSkill {
public:
    JiangchiTargetMod(): TargetModSkill("#jiangchi-target") {
        frequency = NotFrequent;
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill("jiangchi") && from->hasFlag("JiangchiInvoke"))
            return 1;
        else
            return 0;
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill("jiangchi") && from->hasFlag("JiangchiInvoke"))
            return 1000;
        else
            return 0;
    }
};

class Qianxi: public TriggerSkill {
public:
    Qianxi(): TriggerSkill("qianxi") {
        events << EventPhaseStart << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && TriggerSkill::triggerable(target)
            && target->getPhase() == Player::Start) {
            if (room->askForSkillInvoke(target, objectName())) {
                room->broadcastSkillInvoke(objectName());

                JudgeStruct judge;
                judge.reason = objectName();
                judge.play_animation = false;
                judge.who = target;

                room->judge(judge);
            }
        } else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason != objectName() || !target->isAlive()) return false;

            QString color = judge->card->isRed() ? "red" : "black";
            target->tag[objectName()] = QVariant::fromValue(color);

            QList<ServerPlayer *> to_choose;
            foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
                if (target->distanceTo(p) == 1)
                    to_choose << p;
            }
            if (to_choose.isEmpty())
                return false;

            ServerPlayer *victim = room->askForPlayerChosen(target, to_choose, objectName());
            QString pattern = QString(".|%1|.|hand$0").arg(color);

            room->broadcastSkillInvoke(objectName());
            room->setPlayerFlag(victim, "QianxiTarget");
            room->addPlayerMark(victim, QString("@qianxi_%1").arg(color));
            room->setPlayerCardLimitation(victim, "use,response", pattern, false);

            LogMessage log;
            log.type = "#Qianxi";
            log.from = victim;
            log.arg = QString("no_suit_%1").arg(color);
            room->sendLog(log);
        }
        return false;
    }
};

class QianxiClear: public TriggerSkill {
public:
    QianxiClear(): TriggerSkill("#qianxi-clear") {
        events << EventPhaseChanging << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->tag["qianxi"].toString().isNull();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
        }

        QString color = player->tag["qianxi"].toString();
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->hasFlag("QianxiTarget")) {
                room->removePlayerCardLimitation(p, "use,response", QString(".|%1|.|hand$0").arg(color));
                room->setPlayerMark(p, QString("@qianxi_%1").arg(color), 0);
            }
        }
        return false;
    }
};

class Dangxian: public TriggerSkill {
public:
    Dangxian(): TriggerSkill("dangxian") {
        frequency = Compulsory;
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *liaohua, QVariant &) const{
        if (liaohua->getPhase() == Player::RoundStart) {
            room->broadcastSkillInvoke(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = liaohua;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(liaohua, objectName());

            liaohua->setPhase(Player::Play);
            room->broadcastProperty(liaohua, "phase");
            RoomThread *thread = room->getThread();
            if (!thread->trigger(EventPhaseStart, room, liaohua))
                thread->trigger(EventPhaseProceeding, room, liaohua);
            thread->trigger(EventPhaseEnd, room, liaohua);

            liaohua->setPhase(Player::RoundStart);
            room->broadcastProperty(liaohua, "phase");
        }
        return false;
    }
};

class Fuli: public TriggerSkill {
public:
    Fuli(): TriggerSkill("fuli") {
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@laoji") > 0;
    }

    int getKingdoms(Room *room) const{
        QSet<QString> kingdom_set;
        foreach (ServerPlayer *p, room->getAlivePlayers())
            kingdom_set << p->getKingdom();
        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *liaohua, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if (dying_data.who != liaohua)
            return false;
        if (liaohua->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$FuliAnimate", 3000);

            room->removePlayerMark(liaohua, "@laoji");

            RecoverStruct recover;
            recover.recover = qMin(getKingdoms(room), liaohua->getMaxHp()) - liaohua->getHp();
            room->recover(liaohua, recover);

            liaohua->turnOver();
        }
        return false;
    }
};

class Zishou: public DrawCardsSkill {
public:
    Zishou(): DrawCardsSkill("zishou") {
    }

    virtual int getDrawNum(ServerPlayer *liubiao, int n) const{
        Room *room = liubiao->getRoom();
        if (liubiao->isWounded() && room->askForSkillInvoke(liubiao, objectName())) {
            int losthp = liubiao->getLostHp();
            room->broadcastSkillInvoke(objectName(), qMin(3, losthp));
            liubiao->clearHistory();
            liubiao->skip(Player::Play);
            return n + losthp;
        } else
            return n;
    }
};

class Zongshi: public MaxCardsSkill {
public:
    Zongshi(): MaxCardsSkill("zongshi") {
    }

    virtual int getExtra(const Player *target) const{
        int extra = 0;
        QSet<QString> kingdom_set;
        if (target->parent()) {
            foreach (const Player *player, target->parent()->findChildren<const Player *>()) {
                if (player->isAlive())
                    kingdom_set << player->getKingdom();
            }
        }
        extra = kingdom_set.size();
        if (target->hasSkill(objectName()))
            return extra;
        else
            return 0;
    }
};

class Shiyong: public TriggerSkill {
public:
    Shiyong(): TriggerSkill("shiyong") {
        events << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash")
            && (damage.card->isRed() || damage.card->hasFlag("drank"))) {
            int index = 1;
            if (damage.from->getGeneralName().contains("guanyu"))
                index = 3;
            else if (damage.card->hasFlag("drank"))
                index = 2;
            room->broadcastSkillInvoke(objectName(), index);

            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            room->loseMaxHp(player);
        }
        return false;
    }
};

GongqiCard::GongqiCard() {
    target_fixed = true;
}

void GongqiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->setPlayerFlag(source, "InfinityAttackRange");
    const Card *cd = Sanguosha->getCard(subcards.first());
    if (cd->isKindOf("EquipCard")) {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(source))
            if (source->canDiscard(p, "he")) targets << p;
        if (!targets.isEmpty()) {
            ServerPlayer *to_discard = room->askForPlayerChosen(source, targets, "gongqi", "@gongqi-discard", true);
            if (to_discard)
                room->throwCard(room->askForCardChosen(source, to_discard, "he", "gongqi", false, Card::MethodDiscard), to_discard, source);
        }
    }
}

class FuhunViewAsSkill: public ViewAsSkill {
public:
    FuhunViewAsSkill(): ViewAsSkill("fuhun") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= 2 && Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getHandcardNum() >= 2 && pattern == "slash";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        Slash *slash = new Slash(Card::SuitToBeDecided, 0);
        slash->setSkillName(objectName());
        slash->addSubcards(cards);

        return slash;
    }
};

class Fuhun: public TriggerSkill {
public:
    Fuhun(): TriggerSkill("fuhun") {
        events << Damage << EventPhaseChanging;
        view_as_skill = new FuhunViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damage && TriggerSkill::triggerable(player)) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash") && damage.card->getSkillName() == objectName()
                && player->getPhase() == Player::Play) {
                room->handleAcquireDetachSkills(player, "wusheng|paoxiao");
                room->broadcastSkillInvoke(objectName(), 2);
                player->setFlags(objectName());
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive && player->hasFlag(objectName()))
                room->handleAcquireDetachSkills(player, "-wusheng|-paoxiao");
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class Gongqi: public OneCardViewAsSkill {
public:
    Gongqi(): OneCardViewAsSkill("gongqi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("GongqiCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        GongqiCard *card = new GongqiCard;
        card->addSubcard(originalcard->getId());
        card->setSkillName(objectName());
        return card;
    }
};

JiefanCard::JiefanCard() {
    mute = true;
}

bool JiefanCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.isEmpty();
}

void JiefanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->removePlayerMark(source, "@rescue");
    ServerPlayer *target = targets.first();
    source->tag["JiefanTarget"] = QVariant::fromValue((PlayerStar)target);
    int index = 1;
    if (target->isLord()
        && (target->getGeneralName().contains("sunquan")
            || target->getGeneralName().contains("sunjian")
            || target->getGeneralName().contains("sunce")))
        index = 2;
    room->broadcastSkillInvoke("jiefan", index);
    room->doLightbox("$JiefanAnimate", 2500);

    foreach (ServerPlayer *player, room->getAllPlayers()) {
        if (player->isAlive() && player->inMyAttackRange(target))
            room->cardEffect(this, source, player);
    }
    source->tag.remove("JiefanTarget");
}

void JiefanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    PlayerStar target = effect.from->tag["JiefanTarget"].value<PlayerStar>();
    QVariant data = effect.from->tag["JiefanTarget"];
    if (target && !room->askForCard(effect.to, ".Weapon", "@jiefan-discard::" + target->objectName(), data))
        target->drawCards(1);
}

class Jiefan: public ZeroCardViewAsSkill {
public:
    Jiefan(): ZeroCardViewAsSkill("jiefan") {
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new JiefanCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@rescue") >= 1;
    }
};

AnxuCard::AnxuCard() {
    mute = true;
}

bool AnxuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self)
        return false;
    if (targets.isEmpty())
        return true;
    else if (targets.length() == 1)
        return to_select->getHandcardNum() != targets.first()->getHandcardNum();
    else
        return false;
}

bool AnxuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void AnxuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> selecteds = targets;
    ServerPlayer *from = selecteds.first()->getHandcardNum() < selecteds.last()->getHandcardNum() ? selecteds.takeFirst() : selecteds.takeLast();
    ServerPlayer *to = selecteds.takeFirst();
    if (from->getGeneralName().contains("sunquan"))
        room->broadcastSkillInvoke("anxu", 2);
    else
        room->broadcastSkillInvoke("anxu", 1);
    int id = room->askForCardChosen(from, to, "h", "anxu");
    const Card *cd = Sanguosha->getCard(id);
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName());
    room->obtainCard(from, cd, reason);
    room->showCard(from, id);
    if (cd->getSuit() != Card::Spade)
        source->drawCards(1);
}

class Anxu: public ZeroCardViewAsSkill {
public:
    Anxu(): ZeroCardViewAsSkill("anxu") {
    }

    virtual const Card *viewAs() const{
        return new AnxuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("AnxuCard");
    }
};

class Zhuiyi: public TriggerSkill {
public:
    Zhuiyi(): TriggerSkill("zhuiyi") {
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;
        QList<ServerPlayer *> targets = (death.damage && death.damage->from) ? room->getOtherPlayers(death.damage->from) :
                                                                               room->getAlivePlayers();

        if (targets.isEmpty())
            return false;

        QString prompt = "zhuiyi-invoke";
        if (death.damage && death.damage->from && death.damage->from != player)
            prompt = QString("%1x:%2").arg(prompt).arg(death.damage->from->objectName());
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), prompt, true, true);
        if (!target) return false;

        if (target->getGeneralName().contains("sunquan"))
            room->broadcastSkillInvoke(objectName(), 2);
        else
            room->broadcastSkillInvoke(objectName(), 1);

        target->drawCards(3);
        RecoverStruct recover;
        recover.who = player;
        recover.recover = 1;
        room->recover(target, recover, true);
        return false;
    }
};

class LihuoViewAsSkill: public OneCardViewAsSkill {
public:
    LihuoViewAsSkill(): OneCardViewAsSkill("lihuo") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
               && pattern == "slash";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->objectName() == "slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *acard = new FireSlash(originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        return acard;
    }
};

class Lihuo: public TriggerSkill {
public:
    Lihuo(): TriggerSkill("lihuo") {
        events << PreDamageDone << CardFinished;
        view_as_skill = new LihuoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == PreDamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash") && damage.card->getSkillName() == objectName()) {
                QVariantList slash_list = damage.from->tag["InvokeLihuo"].toList();
                slash_list << QVariant::fromValue((CardStar)damage.card);
                damage.from->tag["InvokeLihuo"] = QVariant::fromValue(slash_list);
            }
        } else if (TriggerSkill::triggerable(player)) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash"))
                return false;

            bool can_invoke = false;
            QVariantList slash_list = use.from->tag["InvokeLihuo"].toList();
            foreach (QVariant card, slash_list) {
                if (card.value<CardStar>() == (CardStar)use.card) {
                    can_invoke = true;
                    slash_list.removeOne(card);
                    use.from->tag["InvokeLihuo"] = QVariant::fromValue(slash_list);
                    break;
                }
            }
            if (!can_invoke) return false;

            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->broadcastSkillInvoke("lihuo", 2);
            room->loseHp(player, 1);
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class LihuoTargetMod: public TargetModSkill {
public:
    LihuoTargetMod(): TargetModSkill("#lihuo-target") {
        frequency = NotFrequent;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if (from->hasSkill("lihuo") && card->isKindOf("FireSlash"))
            return 1;
        else
            return 0;
    }
};

ChunlaoCard::ChunlaoCard() {
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
}

void ChunlaoCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    source->addToPile("wine", this);
}

ChunlaoWineCard::ChunlaoWineCard() {
    m_skillName = "chunlao";
    mute = true;
    target_fixed = true;
}

void ChunlaoWineCard::use(Room *room, ServerPlayer *chengpu, QList<ServerPlayer *> &) const{
    if (chengpu->getPile("wine").isEmpty()) return;
    ServerPlayer *who = room->getCurrentDyingPlayer();
    if (!who) return;

    QList<int> cards = chengpu->getPile("wine");
    room->fillAG(cards, chengpu);
    int card_id = room->askForAG(chengpu, cards, false, "chunlao");
    room->clearAG();
    if (card_id != -1) {
        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "chunlao", QString());
        room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
        Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
        analeptic->setSkillName("_chunlao");
        room->useCard(CardUseStruct(analeptic, who, who, false));
    }
}

class ChunlaoViewAsSkill: public ViewAsSkill {
public:
    ChunlaoViewAsSkill(): ViewAsSkill("chunlao") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@chunlao"
               || (pattern.contains("peach") && !player->getPile("wine").isEmpty());
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "@@chunlao")
            return to_select->isKindOf("Slash");
        else
            return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "@@chunlao") {
            if (cards.length() == 0) return NULL;

            Card *acard = new ChunlaoCard;
            acard->addSubcards(cards);
            acard->setSkillName(objectName());
            return acard;
        } else {
            if (cards.length() != 0) return NULL;
            return new ChunlaoWineCard;
        }
    }
};

class Chunlao: public TriggerSkill {
public:
    Chunlao(): TriggerSkill("chunlao") {
        events << EventPhaseStart;
        view_as_skill = new ChunlaoViewAsSkill;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *chengpu, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && chengpu->getPhase() == Player::Finish
            && !chengpu->isKongcheng() && chengpu->getPile("wine").isEmpty()) {
            room->askForUseCard(chengpu, "@@chunlao", "@chunlao", -1, Card::MethodNone);
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        if (card->isKindOf("Analeptic")) {
            if (player->getGeneralName().contains("zhouyu"))
                return 3;
            else
                return 2;
        } else
            return 1;
    }
};

YJCM2012Package::YJCM2012Package()
    : Package("YJCM2012")
{
    General *bulianshi = new General(this, "bulianshi", "wu", 3, false); // YJ 101
    bulianshi->addSkill(new Anxu);
    bulianshi->addSkill(new Zhuiyi);

    General *caozhang = new General(this, "caozhang", "wei"); // YJ 102
    caozhang->addSkill(new Jiangchi);
    caozhang->addSkill(new JiangchiTargetMod);
    related_skills.insertMulti("jiangchi", "#jiangchi-target");

    General *chengpu = new General(this, "chengpu", "wu"); // YJ 103
    chengpu->addSkill(new Lihuo);
    chengpu->addSkill(new LihuoTargetMod);
    chengpu->addSkill(new Chunlao);
    chengpu->addSkill(new DetachEffectSkill("chunlao", "wine"));
    related_skills.insertMulti("lihuo", "#lihuo-target");
    related_skills.insertMulti("chunlao", "#chunlao-clear");

    General *guanxingzhangbao = new General(this, "guanxingzhangbao", "shu"); // YJ 104
    guanxingzhangbao->addSkill(new Fuhun);

    General *handang = new General(this, "handang", "wu"); // YJ 105
    handang->addSkill(new Gongqi);
    handang->addSkill(new Jiefan);
    handang->addSkill(new MarkAssignSkill("@rescue", 1));
    related_skills.insertMulti("jiefan", "#@rescue-1");

    General *huaxiong = new General(this, "huaxiong", "qun", 6); // YJ 106
    huaxiong->addSkill(new Shiyong);

    General *liaohua = new General(this, "liaohua", "shu"); // YJ 107
    liaohua->addSkill(new Dangxian);
    liaohua->addSkill(new MarkAssignSkill("@laoji", 1));
    liaohua->addSkill(new Fuli);
    related_skills.insertMulti("fuli", "#@laoji-1");

    General *liubiao = new General(this, "liubiao", "qun", 4); // YJ 108
    liubiao->addSkill(new Zishou);
    liubiao->addSkill(new Zongshi);

    General *madai = new General(this, "madai", "shu"); // YJ 109
    madai->addSkill("mashu");
    madai->addSkill(new Qianxi);
    madai->addSkill(new QianxiClear);
    related_skills.insertMulti("qianxi", "#qianxi-clear");

    General *wangyi = new General(this, "wangyi", "wei", 3, false); // YJ 110
    wangyi->addSkill(new Zhenlie);
    wangyi->addSkill(new Miji);

    General *xunyou = new General(this, "xunyou", "wei", 3); // YJ 111
    xunyou->addSkill(new Qice);
    xunyou->addSkill(new Zhiyu);

    addMetaObject<QiceCard>();
    addMetaObject<ChunlaoCard>();
    addMetaObject<ChunlaoWineCard>();
    addMetaObject<GongqiCard>();
    addMetaObject<JiefanCard>();
    addMetaObject<AnxuCard>();
}

ADD_PACKAGE(YJCM2012)

