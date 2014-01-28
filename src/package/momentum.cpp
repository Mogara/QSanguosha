#include "momentum.h"
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
#include "jsonutils.h"

class Xunxun: public PhaseChangeSkill {
public:
    Xunxun(): PhaseChangeSkill("xunxun") {
        frequency = Frequent;
    }

    virtual bool triggerable(TriggerEvent , ServerPlayer *lidian, Room *room, QVariant &, ServerPlayer* &ask_who) const {
        return (PhaseChangeSkill::triggerable(lidian) && lidian->getPhase() == Player::Draw);
    }

    virtual bool cost(TriggerEvent , ServerPlayer *lidian, Room *room, QVariant &) const {
        return lidian->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *lidian) const{
        Room *room = lidian->getRoom();
        room->broadcastSkillInvoke(objectName());
        QList<int> card_ids = room->getNCards(4);
        QList<int> obtained;
        room->fillAG(card_ids, lidian);
        int id1 = room->askForAG(lidian, card_ids, false, objectName());
        card_ids.removeOne(id1);
        obtained << id1;
        room->takeAG(lidian, id1, false);
        int id2 = room->askForAG(lidian, card_ids, false, objectName());
        card_ids.removeOne(id2);
        obtained << id2;
        room->clearAG(lidian);

        room->askForGuanxing(lidian, card_ids, Room::GuanxingDownOnly);
        DummyCard *dummy = new DummyCard(obtained);
        lidian->obtainCard(dummy, false);
        delete dummy;

        return true;
    }
};

class Wangxi: public TriggerSkill {
public:
    Wangxi(): TriggerSkill("wangxi") {
        events << Damage << Damaged;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, ServerPlayer *player, Room *room, QVariant &data, ServerPlayer* &ask_who) const {
        if (!TriggerSkill::triggerable(player)) return false;
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = NULL;
        if (triggerEvent == Damage)
            target = damage.to;
        else
            target = damage.from;
        if (!target || !target->isAlive() || target == player) return false;
        return true;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = NULL;
        if (triggerEvent == Damage)
            target = damage.to;
        else
            target = damage.from;
        QList<ServerPlayer *> players;
        players << player << target;
        room->sortByActionOrder(players);

        for (int i = 1; i <= damage.damage; i++) {
            if (room->askForSkillInvoke(player, objectName(), QVariant::fromValue((PlayerStar)target))) {
                room->broadcastSkillInvoke(objectName());
                room->drawCards(players, 1, objectName());
            }
            else break;
        }
        return false;
    }
};

class Hengjiang: public MasochismSkill {
public:
    Hengjiang(): MasochismSkill("hengjiang") {
    }
    
    virtual bool triggerable(TriggerEvent triggerEvent, ServerPlayer *player, Room *room, QVariant &data, ServerPlayer* &ask_who) const {
        if (!TriggerSkill::triggerable(player)) return false;
        ServerPlayer *current = room->getCurrent();
        if (!current || current->isDead() || current->getPhase() == Player::NotActive) return false;
        return true;
    }
    
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *current = room->getCurrent();
        return current && player->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)current));
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        Room *room = target->getRoom();
        ServerPlayer *current = room->getCurrent();
        if (!current) return;
        for (int i = 1; i <= damage.damage; i++) {
            room->broadcastSkillInvoke(objectName());
            room->addPlayerMark(current, "@hengjiang");
            if (!room->askForSkillInvoke(target, objectName(), QVariant::fromValue((PlayerStar)current)))
                break;
        }

        return;
    }
};

class HengjiangDraw: public TriggerSkill {
public:
    HengjiangDraw(): TriggerSkill("#hengjiang-draw") {
        events << TurnStart << CardsMoveOneTime << EventPhaseChanging;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, ServerPlayer *player, Room *room, QVariant &data, ServerPlayer* &ask_who) const {
        if (!player) return false;
        if (triggerEvent == TurnStart) {
            room->setPlayerMark(player, "@hengjiang", 0);
        } else if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from && player == move.from && player->getPhase() == Player::Discard
                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
                player->setFlags("HengjiangDiscarded");
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive) return false;
            ServerPlayer *zangba = room->findPlayerBySkillName("hengjiang");
            if (!zangba) return false;
            if (player->getMark("@hengjiang") > 0) {
                bool invoke = false;
                if (!player->hasFlag("HengjiangDiscarded")) {
                    LogMessage log;
                    log.type = "#HengjiangDraw";
                    log.from = player;
                    log.to << zangba;
                    log.arg = "hengjiang";
                    room->sendLog(log);

                    invoke = true;
                }
                player->setFlags("-HengjiangDiscarded");
                room->setPlayerMark(player, "@hengjiang", 0);
                if (invoke) zangba->drawCards(1);
            }
        }
        return false;
    }
};

class HengjiangMaxCards: public MaxCardsSkill {
public:
    HengjiangMaxCards(): MaxCardsSkill("#hengjiang-maxcard") {
    }

    virtual int getExtra(const Player *target) const{
        return -target->getMark("@hengjiang");
    }
};

class Qianxi: public TriggerSkill {
public:
    Qianxi(): TriggerSkill("qianxi") {
        events << EventPhaseStart << FinishJudge;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, ServerPlayer *player, Room *room, QVariant &data, ServerPlayer* &ask_who) const {
        if (!player) return false;
        if (triggerEvent == EventPhaseStart && TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start) return true;
        else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason != objectName() || !player->isAlive()) return false;

            QString color = judge->card->isRed() ? "red" : "black";
            player->tag[objectName()] = QVariant::fromValue(color);
            judge->pattern = color;
        }
        return false;
    }
    
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data) const{
        return room->askForSkillInvoke(target, objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data) const{
        room->broadcastSkillInvoke(objectName());

        JudgeStruct judge;
        judge.reason = objectName();
        judge.play_animation = false;
        judge.who = target;

        room->judge(judge);
        if (!target->isAlive()) return false;
        QString color = judge.pattern;
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

        return false;
    }
};

class QianxiClear: public TriggerSkill {
public:
    QianxiClear(): TriggerSkill("#qianxi-clear") {
        events << EventPhaseChanging << Death;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, ServerPlayer *player, Room *room, QVariant &data, ServerPlayer* &ask_who) const {
        if (player->tag["qianxi"].toString().isNull()) return false;
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

/*GuixiuCard::GuixiuCard() {
    target_fixed = true;
}

void GuixiuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->removePlayerMark(source, "guixiu");
    source->drawCards(2, "guixiu");
}

class GuixiuViewAsSkill: public ZeroCardViewAsSkill {
public:
    GuixiuViewAsSkill(): ZeroCardViewAsSkill("guixiu") {
    }

    virtual const Card *viewAs() const{
        return new GuixiuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("guixiu") >= 1;
    }
};

class Guixiu: public TriggerSkill {
public:
    Guixiu(): TriggerSkill("guixiu") {
        frequency = Limited;
        limit_mark = "guixiu";
        events << EventPhaseStart;
        view_as_skill = new GuixiuViewAsSkill;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getMark("guixiu") >= 1 && player->getPhase() == Player::Start
            && room->askForSkillInvoke(player, objectName())) {
            room->broadcastSkillInvoke(objectName(), 1);
            room->removePlayerMark(player, "guixiu");
            player->drawCards(2, objectName());
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class GuixiuDetach: public DetachEffectSkill {
public:
    GuixiuDetach(): DetachEffectSkill("guixiu") {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        if (player->isWounded() && room->askForSkillInvoke(player, "guixiu_rec", "recover")) {
            room->broadcastSkillInvoke("guixiu", 2);
            room->notifySkillInvoked(player, "guixiu");
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        }
    }
};

CunsiCard::CunsiCard() {
}

bool CunsiCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.isEmpty();
}

void CunsiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->doLightbox("$CunsiAnimate", 3000);
    room->handleAcquireDetachSkills(effect.from, "-guixiu|-cunsi");

    room->acquireSkill(effect.to, "yongjue");
    if (effect.to != effect.from)
        effect.to->drawCards(2);
}

class Cunsi: public ZeroCardViewAsSkill {
public:
    Cunsi(): ZeroCardViewAsSkill("cunsi") {
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new CunsiCard;
    }
};

class CunsiStart: public TriggerSkill {
public:
    CunsiStart(): TriggerSkill("#cunsi-start") {
        events << GameStart << EventAcquireSkill;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &) const{
        room->getThread()->addTriggerSkill(Sanguosha->getTriggerSkill("yongjue"));
        return false;
    }
};

class Yongjue: public TriggerSkill {
public:
    Yongjue(): TriggerSkill("yongjue") {
        events << CardUsed << BeforeCardsMove;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from->getPhase() == Player::Play && use.from->getMark(objectName()) == 0) {
                use.from->addMark(objectName());
                if (use.card->isKindOf("Slash")) {
                    QList<int> ids;
                    if (!use.card->isVirtualCard())
                        ids << use.card->getEffectiveId();
                    else if (use.card->subcardsLength() > 0)
                        ids = use.card->getSubcards();
                    if (!ids.isEmpty()) {
                        room->setCardFlag(use.card, "yongjue");
                        room->setTag("yongjue_user", QVariant::fromValue((PlayerStar)use.from));
                        room->setTag("yongjue_card", QVariant::fromValue((CardStar)use.card));
                    }
                }
            }
        } else if (TriggerSkill::triggerable(player)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
                && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
                PlayerStar yongjue_user = room->getTag("yongjue_user").value<PlayerStar>();
                CardStar yongjue_card = room->getTag("yongjue_card").value<CardStar>();
                room->removeTag("yongjue_user");
                room->removeTag("yongjue_card");
                if (yongjue_user && yongjue_card && yongjue_card->hasFlag("yongjue")) {
                    QList<int> ids;
                    if (!yongjue_card->isVirtualCard())
                        ids << yongjue_card->getEffectiveId();
                    else if (yongjue_card->subcardsLength() > 0)
                        ids = yongjue_card->getSubcards();
                    if (!ids.isEmpty()) {
                        foreach (int id, ids) {
                            if (!move.card_ids.contains(id)) return false;
                        }
                    } else {
                        return false;
                    }
                    if (room->askForSkillInvoke(player, objectName(), QVariant::fromValue(yongjue_user))) {
                        DummyCard *dummy = new DummyCard(ids);
                        yongjue_user->obtainCard(dummy);
                        delete dummy;
                        move.card_ids.clear();
                        data = QVariant::fromValue(move);
                    }
                }
            }
        }

        return false;
    }
};

class YongjueStart: public PhaseChangeSkill {
public:
    YongjueStart(): PhaseChangeSkill("#yongjue-start") {
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Play)
            target->setMark("yongjue", 0);
        return false;
    }
};*/

class Jiang: public TriggerSkill {
public:
    Jiang(): TriggerSkill("jiang") {
        events << TargetConfirmed;
        frequency = Frequent;
    }

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *sunce, QVariant &data, ServerPlayer* &ask_who) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from == sunce || use.to.contains(sunce)) {
            if (use.card->isKindOf("Duel") || (use.card->isKindOf("Slash") && use.card->isRed()))
                return true;
        }
        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *sunce, QVariant &data) const {
        return sunce->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *sunce, QVariant &data) const {
        CardUseStruct use = data.value<CardUseStruct>();
        
        int index = 1;
        if (use.from != sunce)
            index = 2;
        room->broadcastSkillInvoke(objectName(), index);
        sunce->drawCards(1);

        return false;
    }
};

class Yingyang: public TriggerSkill {
public:
    Yingyang(): TriggerSkill("yingyang") {
        events << PindianVerifying;
    }

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (player == NULL) return false;
        ServerPlayer *sunce = room->findPlayerBySkillName(objectName());
        if (!TriggerSkill::triggerable(sunce)) return false;
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->from != sunce && pindian->to != sunce) return false;
        ask_who = pindian->from == sunce ? player : sunce;
        return true;
    }

    virtual bool cost(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *sunce = room->findPlayerBySkillName(objectName());
        return sunce && sunce->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *sunce = room->findPlayerBySkillName(objectName());
        if (!sunce) return false;
        PindianStar pindian = data.value<PindianStar>();
        bool isFrom = pindian->from == sunce;

        QString choice = room->askForChoice(sunce, objectName(), "jia3+jian3", objectName());
        int to_add = choice == "jia3" ? 3 : -3;
        
        LogMessage log;
        log.type = "$Yingyang";
        log.from = sunce;

        if (isFrom) {
            pindian->from_number += to_add;

            log.arg = QString::number(pindian->from_number);
        } else {
            pindian->to_number += to_add;

            log.arg = QString::number(pindian->to_number);
        }

        room->sendLog(log);

        return false;
    }
};

class Hunshang: public TriggerSkill {
public:
    Hunshang(): TriggerSkill("hunshang") {
        events << EventPhaseStart << DrawNCards;
        frequency = Compulsory;
        relate_to_place = "deputy";
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (!TriggerSkill::triggerable(player)) return false;
        if (player->getMark("@waked") > 0) return false;
        if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() == Player::Start)
                return player->getHp() == 1;
            else if (player->getPhase() == Player::NotActive)
                if (player->getMark("hunshang_invoke") > 0) {
                    room->setPlayerMark(player, "hunshang_invoke", 0);
                    player->gainMark("@waked");
                }
        } else if (triggerEvent == DrawNCards)
            return player->getMark("hunshang_invoke") > 0;

        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (player->hasShownSkill(this)) return true;
            return player->askForSkillInvoke(objectName());
        } else {
            if (player->askForSkillInvoke("yingzi")) {
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = player;
                log.arg = "yingzi";
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room* room, ServerPlayer *sunce, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            room->notifySkillInvoked(sunce, objectName());
            room->setPlayerMark(sunce, "hunshang_invoke", 1);

            if (sunce->askForSkillInvoke("yinghun")) {
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = sunce;
                log.arg = "yinghun";
                room->sendLog(log);

                ServerPlayer *to = room->askForPlayerChosen(sunce, room->getOtherPlayers(sunce), "yinghun", "yinghun-invoke", false, true);
                if (to) {
                    int x = sunce->getLostHp();

                    int index = 3;

                    if (x == 1) {
                        room->broadcastSkillInvoke("yihun", index);

                        to->drawCards(1);
                        room->askForDiscard(to, objectName(), 1, 1, false, true);
                    } else {
                        to->setFlags("YinghunTarget");
                        QString choice = room->askForChoice(sunce, "yihun", "d1tx+dxt1");
                        to->setFlags("-YinghunTarget");
                        if (choice == "d1tx") {
                            room->broadcastSkillInvoke("yihun", index + 1);

                            to->drawCards(1);
                            room->askForDiscard(to, "yihun", x, x, false, true);
                        } else {
                            room->broadcastSkillInvoke("yihun", index);

                            to->drawCards(x);
                            room->askForDiscard(to, "yihun", 1, 1, false, true);
                        }
                    }
                }
            }
        } else {
            int n = data.toInt();
            data = n + 1;
        }

        return false;
    }
};

DuanxieCard::DuanxieCard() {
}

bool DuanxieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isChained() && to_select != Self;
}

void DuanxieCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->setPlayerProperty(effect.to, "chained", true);
    if (!effect.from->isChained())
        room->setPlayerProperty(effect.from, "chained", true);
}

class Duanxie : public ZeroCardViewAsSkill {
public:
    Duanxie(): ZeroCardViewAsSkill("duanxie") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuanxieCard");
    }

    virtual const Card *viewAs() const{
        Card *card = new DuanxieCard;
        card->setShowSkill(objectName());
        return card;
    }
};

class Fenming: public PhaseChangeSkill {
public:
    Fenming(): PhaseChangeSkill("fenming") {
    }

    virtual bool triggerable(TriggerEvent , ServerPlayer *player, Room *room, QVariant &, ServerPlayer* &ask_who) const {
        if (PhaseChangeSkill::triggerable(player) && player->getPhase() == Player::Finish && player->isChained())
            foreach (ServerPlayer *p, room->getAlivePlayers())
                if (p->isChained() && player->canDiscard(p, "he"))
                    return true;

        return false;
    }

    virtual bool cost(TriggerEvent , ServerPlayer *player, Room *room, QVariant &) const {
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->broadcastSkillInvoke(objectName());
        foreach (ServerPlayer *p, room->getAlivePlayers())
            if (p->isChained() && player->canDiscard(p, "he")) {
                int card_id = room->askForCardChosen(player, p, "he", objectName(), false, Card::MethodDiscard);
                room->throwCard(card_id, p, player);
            }
        return false;
    }
};

MomentumPackage::MomentumPackage()
    : Package("momentum")
{
    General *lidian = new General(this, "lidian", "wei", 3); // WEI 017
    lidian->addCompanion("yuejin");
    lidian->addSkill(new Xunxun);
    lidian->addSkill(new Wangxi);

    General *zangba = new General(this, "zangba", "wei", 4); // WEI 023
    zangba->addCompanion("zhangliao");
    zangba->addSkill(new Hengjiang);
    zangba->addSkill(new HengjiangDraw);
    zangba->addSkill(new HengjiangMaxCards);
    related_skills.insertMulti("hengjiang", "#hengjiang-draw");
    related_skills.insertMulti("hengjiang", "#hengjiang-maxcard");

    General *madai = new General(this, "madai", "shu", 4); // SHU 019
    madai->addCompanion("machao");
    madai->addSkill(new Mashu("madai"));
    madai->addSkill(new Qianxi);
    madai->addSkill(new QianxiClear);
    related_skills.insertMulti("qianxi", "#qianxi-clear");

    /*General *mifuren = new General(this, "mifuren", "shu", 3, false); // SHU 021
    mifuren->addSkill(new Guixiu);
    mifuren->addSkill(new GuixiuDetach);
    mifuren->addSkill(new Cunsi);
    mifuren->addSkill(new CunsiStart);
    related_skills.insertMulti("guixiu", "#guixiu-clear");
    related_skills.insertMulti("cunsi", "#cunsi-start");
    mifuren->addRelateSkill("yongjue");*/

    General *sunce = new General(this, "sunce", "wu", 4); // WU 010
    sunce->addCompanion("zhouyu");
    sunce->addCompanion("taishici");
    sunce->addCompanion("daqiao");
    sunce->addSkill(new Jiang);
    sunce->addSkill(new Yingyang);
    sunce->addSkill(new Hunshang);
    sunce->setDeputyMaxHpAdjustedValue(-1);

    General *chenwudongxi = new General(this, "chenwudongxi", "wu", 4); // WU 023
    chenwudongxi->addSkill(new Duanxie);
    chenwudongxi->addSkill(new Fenming);

    /*General *dongzhuo = new General(this, "dongzhuo", "qun", 4); // QUN 006 G

    General *zhangren = new General(this, "zhangren", "qun", 3);

    skills << new Yongjue << new YongjueStart;
    related_skills.insertMulti("yongjue", "#yongjue-start");

    addMetaObject<GuixiuCard>();
    addMetaObject<CunsiCard>();*/
    addMetaObject<DuanxieCard>();
}

ADD_PACKAGE(Momentum)