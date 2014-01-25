#include "mountainpackage.h"

#include "general.h"
#include "settings.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "ai.h"
#include "jsonutils.h"

#include <QCommandLinkButton>

class Tuntian: public TriggerSkill {
public:
    Tuntian(): TriggerSkill("tuntian") {
        events << CardsMoveOneTime << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && (move.from_places.contains(Player::PlaceHand) || move.from_places.contains(Player::PlaceEquip))
                && !(move.to == player && (move.to_place == Player::PlaceHand || move.to_place == Player::PlaceEquip))
                && player->askForSkillInvoke("tuntian", data)) {
                room->broadcastSkillInvoke("tuntian");
                JudgeStruct judge;
                judge.pattern = ".|heart";
                judge.good = false;
                judge.reason = "tuntian";
                judge.who = player;
                room->judge(judge);
            }
        } else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason == "tuntian" && judge->isGood())
                player->addToPile("field", judge->card->getEffectiveId());
        }

        return false;
    }
};

class TuntianDistance: public DistanceSkill {
public:
    TuntianDistance(): DistanceSkill("#tuntian-dist") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasSkill("tuntian"))
            return -from->getPile("field").length();
        else
            return 0;
    }
};

class Zaoxian: public PhaseChangeSkill {
public:
    Zaoxian(): PhaseChangeSkill("zaoxian") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Start
               && target->getMark("zaoxian") == 0
               && target->getPile("field").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *dengai) const{
        Room *room = dengai->getRoom();
        room->notifySkillInvoked(dengai, objectName());

        LogMessage log;
        log.type = "#ZaoxianWake";
        log.from = dengai;
        log.arg = QString::number(dengai->getPile("field").length());
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$ZaoxianAnimate", 4000);

        room->addPlayerMark(dengai, "zaoxian");
        if (room->changeMaxHpForAwakenSkill(dengai))
            room->acquireSkill(dengai, "jixi");

        return false;
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

    virtual Location getLocation() const{
        return Right;
    }
};

class Jiang: public TriggerSkill {
public:
    Jiang(): TriggerSkill("jiang") {
        events << TargetConfirmed;
        frequency = Frequent;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *sunce, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from == sunce || use.to.contains(sunce)) {
            if (use.card->isKindOf("Duel") || (use.card->isKindOf("Slash") && use.card->isRed())) {
                if (sunce->askForSkillInvoke(objectName(), data)) {
                    int index = 1;
                    if (use.from != sunce)
                        index = 2;
                    if (!sunce->hasInnateSkill(objectName()) && sunce->hasSkill("mouduan"))
                        index += 2;
                    room->broadcastSkillInvoke(objectName(), index);
                    sunce->drawCards(1);
                }
            }
        }

        return false;
    }
};

class Hunzi: public PhaseChangeSkill {
public:
    Hunzi(): PhaseChangeSkill("hunzi") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getMark("hunzi") == 0
               && target->getPhase() == Player::Start
               && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();
        room->notifySkillInvoked(sunce, objectName());

        LogMessage log;
        log.type = "#HunziWake";
        log.from = sunce;
        log.arg = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$HunziAnimate", 5000);

        room->addPlayerMark(sunce, "hunzi");
        if (room->changeMaxHpForAwakenSkill(sunce))
            room->handleAcquireDetachSkills(sunce, "yingzi|yinghun");
        return false;
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
        return new TiaoxinCard;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        int index = qrand() % 2 + 1;
        if (!player->hasInnateSkill(objectName()) && player->hasSkill("baobian"))
            index += 2;
        return index;
    }
};

class Zhiji: public PhaseChangeSkill {
public:
    Zhiji(): PhaseChangeSkill("zhiji") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getMark("zhiji") == 0
               && target->getPhase() == Player::Start
               && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *jiangwei) const{
        Room *room = jiangwei->getRoom();
        room->notifySkillInvoked(jiangwei, objectName());

        LogMessage log;
        log.type = "#ZhijiWake";
        log.from = jiangwei;
        log.arg = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$ZhijiAnimate", 4000);

        if (jiangwei->isWounded() && room->askForChoice(jiangwei, objectName(), "recover+draw") == "recover") {
            RecoverStruct recover;
            recover.who = jiangwei;
            room->recover(jiangwei, recover);
        } else {
            room->drawCards(jiangwei, 2);
        }
        room->addPlayerMark(jiangwei, "zhiji");
        if (room->changeMaxHpForAwakenSkill(jiangwei))
            room->acquireSkill(jiangwei, "guanxing");

        return false;
    }
};

class Xiangle: public TriggerSkill {
public:
    Xiangle(): TriggerSkill("xiangle") {
        events << SlashEffected << TargetConfirming;
        frequency = Compulsory;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *liushan, QVariant &data) const{
        if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash")) {
                room->broadcastSkillInvoke(objectName());
                room->notifySkillInvoked(liushan, objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = liushan;
                log.arg = objectName();
                room->sendLog(log);

                QVariant dataforai = QVariant::fromValue(liushan);
                if (!room->askForCard(use.from, ".Basic", "@xiangle-discard", dataforai))
                    liushan->addMark("xiangle");
            }
        } else {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (liushan->getMark("xiangle") > 0) {
                LogMessage log;
                log.type = "#XiangleAvoid";
                log.from = effect.from;
                log.to << liushan;
                log.arg = objectName();
                room->sendLog(log);
                liushan->removeMark("xiangle");
                return true;
            }
        }

        return false;
    }
};

class XiangleRemoveMark: public TriggerSkill {
public:
    XiangleRemoveMark(): TriggerSkill("#xiangle") {
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) {
            foreach (ServerPlayer *to, use.to)
                to->setMark("xiangle", 0);
        }
        return false;
    }
};

FangquanCard::FangquanCard() {
}

bool FangquanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FangquanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    ServerPlayer *liushan = effect.from, *player = effect.to;

    LogMessage log;
    log.type = "#Fangquan";
    log.from = liushan;
    log.to << player;
    room->sendLog(log);

    room->setTag("FangquanTarget", QVariant::fromValue((PlayerStar)player));
}

class FangquanViewAsSkill: public OneCardViewAsSkill {
public:
    FangquanViewAsSkill(): OneCardViewAsSkill("fangquan") {
        filter_pattern = ".|.|.|hand!";
        response_pattern = "@@fangquan";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FangquanCard *fangquan = new FangquanCard;
        fangquan->addSubcard(originalCard);
        return fangquan;
    }
};

class Fangquan: public TriggerSkill {
public:
    Fangquan(): TriggerSkill("fangquan") {
        events << EventPhaseChanging;
        view_as_skill = new FangquanViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *liushan, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        switch (change.to) {
        case Player::Play: {
                bool invoked = false;
                if (!TriggerSkill::triggerable(liushan) || liushan->isSkipped(Player::Play))
                    return false;
                invoked = liushan->askForSkillInvoke(objectName());
                if (invoked) {
                    liushan->setFlags(objectName());
                    liushan->skip(Player::Play);
                }
                break;
            }
        case Player::NotActive: {
                if (liushan->hasFlag(objectName())) {
                    if (!liushan->canDiscard(liushan, "h"))
                        return false;
                    room->askForUseCard(liushan, "@@fangquan", "@fangquan-give", -1, Card::MethodDiscard);
                }
                break;
            }
        default:
                break;
        }
        return false;
    }
};

class FangquanGive: public PhaseChangeSkill {
public:
    FangquanGive(): PhaseChangeSkill("#fangquan-give") {
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        Room *room = liushan->getRoom();
        if (!room->getTag("FangquanTarget").isNull()) {
            PlayerStar target = room->getTag("FangquanTarget").value<PlayerStar>();
            room->removeTag("FangquanTarget");
            if (target->isAlive())
                target->gainAnExtraTurn();
        }
        return false;
    }
};


MountainPackage::MountainPackage()
    : Package("mountain")
{

    General *dengai = new General(this, "dengai", "wei", 4); // WEI 015
    dengai->addSkill(new Tuntian);
    dengai->addSkill(new TuntianDistance);
    dengai->addSkill(new Zaoxian);
    dengai->addRelateSkill("jixi");
    related_skills.insertMulti("tuntian", "#tuntian-dist");

    General *jiangwei = new General(this, "jiangwei", "shu"); // SHU 012
    jiangwei->addSkill(new Tiaoxin);
    jiangwei->addSkill(new Zhiji);

    General *liushan = new General(this, "liushan$", "shu", 3); // SHU 013
    liushan->addSkill(new Xiangle);
    liushan->addSkill(new XiangleRemoveMark);
    liushan->addSkill(new Fangquan);
    liushan->addSkill(new FangquanGive);
    related_skills.insertMulti("xiangle", "#xiangle");
    related_skills.insertMulti("fangquan", "#fangquan-give");

    General *sunce = new General(this, "sunce$", "wu"); // WU 010
    sunce->addSkill(new Jiang);
    sunce->addSkill(new Hunzi);

    addMetaObject<TiaoxinCard>();
    addMetaObject<JixiCard>();
    addMetaObject<JixiSnatchCard>();
    addMetaObject<FangquanCard>();

    skills << new Jixi;
}

ADD_PACKAGE(Mountain)

