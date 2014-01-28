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

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *lidian, QVariant &, ServerPlayer* &ask_who) const {
        return (PhaseChangeSkill::triggerable(lidian) && lidian->getPhase() == Player::Draw);
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *lidian, QVariant &) const {
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
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
    
    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
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

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
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

class Guixiu: public TriggerSkill {
public:
    Guixiu(): TriggerSkill("guixiu") {
        events << GameStart << GeneralShown << GeneralRemoved;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (triggerEvent == GameStart) {
            if (TriggerSkill::triggerable(player))
                room->setPlayerMark(player, objectName(), 1); // for guixiu2
        } else if (triggerEvent == GeneralShown) {
            if (TriggerSkill::triggerable(player))
                return data.toBool() == player->inHeadSkills(objectName());
        } else if (data.toString() == "mifuren" && player->getMark(objectName()) > 0) {
            room->setPlayerMark(player, objectName(), 0);
            if (player->isWounded() && room->askForSkillInvoke(player, objectName())) {
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = player;
                log.arg = "guixiu";
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
        }

        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (room->askForSkillInvoke(player, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        player->drawCards(2, objectName());
        return false;
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
    effect.from->removeGeneral(effect.from->inHeadSkills("cunsi"));
    room->acquireSkill(effect.to, "yongjue");
    if (effect.to != effect.from)
        effect.to->drawCards(2);
}

class Cunsi: public ZeroCardViewAsSkill {
public:
    Cunsi(): ZeroCardViewAsSkill("cunsi") {
    }

    virtual const Card *viewAs() const{
        Card *card = new CunsiCard;
        card->setShowSkill(objectName());
        return card;
    }
};

class CunsiStart: public TriggerSkill {
public:
    CunsiStart(): TriggerSkill("#cunsi-start") {
        events << GameStart << EventAcquireSkill;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (!player || !player->isAlive() || !player->hasSkill("cunsi")) return false;
        room->getThread()->addTriggerSkill(Sanguosha->getTriggerSkill("yongjue"));
        return false;
    }
};

class Yongjue: public TriggerSkill {
public:
    Yongjue(): TriggerSkill("yongjue") {
        events << CardUsed << BeforeCardsMove;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (player == NULL) return false;
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
            return false;
        } else if (TriggerSkill::triggerable(player)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
                && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
                PlayerStar yongjue_user = room->getTag("yongjue_user").value<PlayerStar>();
                CardStar yongjue_card = room->getTag("yongjue_card").value<CardStar>();
                room->removeTag("yongjue_user");
                room->removeTag("yongjue_card");
                if (yongjue_user && player->isFriendWith(yongjue_user) && yongjue_card && yongjue_card->hasFlag("yongjue")) {
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
                    room->setTag("yongjue_user", QVariant::fromValue((PlayerStar)yongjue_user));
                    room->setTag("yongjue_card", QVariant::fromValue((CardStar)yongjue_card));
                    return true;
                }
            }
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (room->askForSkillInvoke(player, objectName())) {
            return true;
        } else {
            room->removeTag("yongjue_user");
            room->removeTag("yongjue_card");
        }

        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
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
            DummyCard *dummy = new DummyCard(ids);
            yongjue_user->obtainCard(dummy);
            delete dummy;
            move.card_ids.clear();
            data = QVariant::fromValue(move);
        }

        return false;
    }
};

class YongjueStart: public TriggerSkill {
public:
    YongjueStart(): TriggerSkill("#yongjue-start") {
        events << EventPhaseStart;
    }

    virtual int getPriority() const{
        return 10;
    }
    
    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *target, QVariant &data, ServerPlayer* &ask_who) const {
        if (!TriggerSkill::triggerable(target)) return false;
        if (target->getPhase() == Player::Play)
            target->setMark("yongjue", 0);
        return false;
    }
};

class Jiang: public TriggerSkill {
public:
    Jiang(): TriggerSkill("jiang") {
        events << TargetConfirmed;
        frequency = Frequent;
    }

    virtual bool triggerable(TriggerEvent, Room *room, ServerPlayer *sunce, QVariant &data, ServerPlayer* &ask_who) const {
		if (!TriggerSkill::triggerable(sunce)) return false;
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

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &ask_who) const {
        if (PhaseChangeSkill::triggerable(player) && player->getPhase() == Player::Finish && player->isChained())
            foreach (ServerPlayer *p, room->getAlivePlayers())
                if (p->isChained() && player->canDiscard(p, "he"))
                    return true;

        return false;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
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

class Hengzheng: public PhaseChangeSkill {
public:
    Hengzheng(): PhaseChangeSkill("hengzheng") {
    }

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &ask_who) const {
        if (PhaseChangeSkill::triggerable(player) && player->getPhase() == Player::Draw && (player->isKongcheng() || player->getHp() == 1))
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
                if (!p->isAllNude())
                    return true;

        return false;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->broadcastSkillInvoke(objectName());
        foreach (ServerPlayer *p, room->getAlivePlayers())
            if (!p->isAllNude()) {
                int card_id = room->askForCardChosen(player, p, "hej", objectName());
                room->obtainCard(player, card_id, false);
            }
        return true;
    }
};

class Baoling: public TriggerSkill {
public:
    Baoling(): TriggerSkill("baoling") {
        events << EventPhaseEnd;
        relate_to_place = "head";
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &ask_who) const {
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Play && player->hasShownSkill(this))
            return !player->getActualGeneral2Name().contains("sujiang");

        return false;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
        return true;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
        player->removeGeneral(false);
        room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 3);

        LogMessage log;
        log.type = "#GainMaxHp";
        log.from = player;
        log.arg = QString::number(3);
        room->sendLog(log);

        RecoverStruct recover;
        recover.recover = 3;
        recover.who = player;
        room->recover(player, recover);

        room->handleAcquireDetachSkills(player, "benghuai");
        return false;
    }
};

class Benghuai: public PhaseChangeSkill {
public:
    Benghuai(): PhaseChangeSkill("benghuai") {
        frequency = Compulsory;
    }

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &ask_who) const {
        if (!PhaseChangeSkill::triggerable(player)) return false;
        if (player->getPhase() == Player::Finish) {
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach (ServerPlayer *p, players)
                if (player->getHp() > p->getHp())
                    return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const{
        Room *room = dongzhuo->getRoom();
        LogMessage log;
        log.from = dongzhuo;
        log.arg = objectName();
        log.type = "#TriggerSkill";
        room->sendLog(log);
        room->notifySkillInvoked(dongzhuo, objectName());

        QString result = room->askForChoice(dongzhuo, "benghuai", "hp+maxhp");
        int index = (result == "hp") ? 2 : 1;
        room->broadcastSkillInvoke(objectName(), index);
        if (result == "hp")
            room->loseHp(dongzhuo);
        else
            room->loseMaxHp(dongzhuo);

        return false;
    }
};

class Chuanxin: public TriggerSkill {
public:
    Chuanxin(): TriggerSkill("chuanxin") {
        events << DamageCaused;
    }

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (!TriggerSkill::triggerable(player)) return false;
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.to || !damage.to->hasShownOneGeneral()) return false;
        if (!damage.card || !(damage.card->isKindOf("Slash") || damage.card->isKindOf("Duel"))) return false;
        if (!player->hasShownOneGeneral()) return false;
        if (player->isFriendWith(damage.to)) return false;
		if (damage.transfer || damage.chain) return false;
        if (damage.to->getActualGeneral2Name().contains("sujiang")) return false;
        return true;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const {
        return player->askForSkillInvoke(objectName());
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const {
        DamageStruct damage = data.value<DamageStruct>();
        QStringList choices;
        if (damage.to->hasEquip())
            choices << "discard";
        choices << "remove";
        QString choice = room->askForChoice(damage.to, objectName(), choices.join("+"));
        if (choice == "discard") {
            damage.to->throwAllEquips();
            room->loseHp(damage.to);
        } else
            damage.to->removeGeneral(false);
        
        return true;
    }
};

FengshiSummon::FengshiSummon() 
    : ArraySummonCard("fengshi")
{

}

class Fengshi: public BattleArraySkill {
public:
    Fengshi(): BattleArraySkill("fengshi", BattleArrayType::Siege) {
        events << TargetConfirmed;
    }
    
    virtual bool canPreshow() const{
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (!TriggerSkill::triggerable(player)) return false;
        if (!player->hasShownSkill(this) || player->aliveCount() < 4) return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash"))
            foreach (ServerPlayer *to, use.to)
                if (use.from->inSiegeRelation(player, to))
                    if (to->canDiscard(to, "e")) {
                        ask_who = player;
                        return true;
                    }

        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash"))
            foreach (ServerPlayer *to, use.to)
                if (use.from->inSiegeRelation(player, to))
                    if (to->canDiscard(to, "e")) {
                        int card_id = room->askForCardChosen(to, to, "e", objectName(), true, Card::MethodDiscard);
                        room->throwCard(card_id, to);
                    }
        return false;
    }
};

class Wuxin: public PhaseChangeSkill {
public:
    Wuxin(): PhaseChangeSkill("wuxin") {
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &ask_who) const {
        if (!PhaseChangeSkill::triggerable(player)) return false;
        if (player->getPhase() == Player::Draw) return true;

        return false;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
        return player->askForSkillInvoke(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        int num = 0;
        foreach (auto p, room->getAllPlayers())
            if (p->hasShownOneGeneral() && p->getKingdom() == player->getKingdom())
                num ++;
        int num2 = player->getPile("heavenly_army").length();
        QList<int> guanxing = room->getNCards(num + num2);

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(guanxing).join("+");
        room->doNotify(player, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

        room->askForGuanxing(player, guanxing, Room::GuanxingUpOnly);

        return false;
    }
};

HongfaCard::HongfaCard() {
    target_fixed = true;
    m_skillName = "hongfa_slash";
}

void HongfaCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *qunxiong = card_use.from;
    ServerPlayer *zhangjiao = room->findPlayerBySkillName("hongfa");
    if (!zhangjiao || !zhangjiao->isFriendWith(qunxiong)) return;
    QList<int> tianbings;
    QList<int> total = zhangjiao->getPile("heavenly_army");
    foreach (int id, total) {
        Slash *slash = new Slash(Card::SuitToBeDecided, -1);
        slash->addSubcard(id);
        if (!Slash::IsAvailable(qunxiong, slash))
            continue;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (!slash->targetFilter(QList<const Player *>(), p, qunxiong))
                continue;
            if (qunxiong->isProhibited(p, slash))
                continue;
            tianbings << id;
            break;
        }
        delete slash;
        slash = NULL;
    }

    if (tianbings.isEmpty())
        return;

    QList<int> disabled;
    foreach (int id, total) {
        if (!tianbings.contains(id))
            disabled << id;
    }

    int card_id;
    if (tianbings.length() == 1)
        card_id = tianbings.first();
    else {
        room->fillAG(total, qunxiong, disabled);
        card_id = room->askForAG(qunxiong, tianbings, false, "hongfa");
        room->clearAG(qunxiong);

        if (card_id == -1)
            return;
    }

    Slash *slash = new Slash(Card::SuitToBeDecided, -1);
    slash->setSkillName("hongfa");
    slash->addSubcard(card_id);

    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (!slash->targetFilter(QList<const Player *>(), p, qunxiong))
            continue;
        if (qunxiong->isProhibited(p, slash))
            continue;

        targets << p;
    }
    if (targets.isEmpty())
        return;

    room->setPlayerProperty(qunxiong, "hongfa_slash", slash->toString());

    CardUseStruct use;
    use.card = slash;
    use.from = qunxiong;

    if (room->askForUseCard(qunxiong, "@@hongfa!", "@hongfa-target")) {
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->hasFlag("HongfaTarget")) {
                room->setPlayerFlag(p, "-HongfaTarget");
                use.to << p;
            }
        }
    } else {
        use.to << targets.at(qrand() % targets.length());
    }
    room->setPlayerProperty(qunxiong, "hongfa_slash", QString());
    room->useCard(use);
}

HongfaSlashCard::HongfaSlashCard() {
    m_skillName = "hongfa_slash";
}

bool HongfaSlashCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    const Card *card = Card::Parse(Self->property("hongfa_slash").toString());
    if (card == NULL)
        return false;
    else {
        const Slash *slash = qobject_cast<const Slash *>(card);
        return !Self->isProhibited(to_select, slash, targets) && slash->targetFilter(targets, to_select, Self);
    }
}

void HongfaSlashCard::onUse(Room *room, const CardUseStruct &card_use) const{
    foreach (ServerPlayer *to, card_use.to)
        room->setPlayerFlag(to, "HongfaSlashTarget");
}

class HongfaSlash: public ZeroCardViewAsSkill {
public:
    HongfaSlash(): ZeroCardViewAsSkill("hongfa_slash") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        const Player *zhangjiao;
        QList<const Player *> ps = player->getAliveSiblings();
        ps << player;
        foreach (const Player *p, ps)
            if (p->hasSkill("hongfa")) {
                zhangjiao = p;
                break;
            }
        if (!zhangjiao || zhangjiao->getPile("heavenly_army").isEmpty() || !zhangjiao->isFriendWith(player))
            return false;
        foreach (int id, zhangjiao->getPile("heavenly_army")) {
            Slash *slash = new Slash(Card::SuitToBeDecided, -1);
            slash->setSkillName("hongfa");
            slash->addSubcard(id);
            slash->deleteLater();
            if (!Slash::IsAvailable(player, slash))
                continue;
            foreach (const Player *p, player->getAliveSiblings()) {
                if (!slash->targetFilter(QList<const Player *>(), p, player))
                    continue;
                if (player->isProhibited(p, slash))
                    continue;
                return true;
            }
        }
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@hongfa!";
    }

    virtual const Card *viewAs() const{
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "@@hongfa!")
            return new HongfaSlashCard;
        else
            return new HongfaCard;
    }
};

class Hongfa: public TriggerSkill {
public:
    Hongfa(): TriggerSkill("hongfa$") {
        events << EventPhaseStart << PreHpLost << GeneralShown << GeneralHidden << GeneralRemoved << Death;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (triggerEvent == EventPhaseStart) {
            if (!TriggerSkill::triggerable(player)) return false;
            if (player->getPhase() != Player::Start) return false;
            if (!player->getPile("heavenly_army").isEmpty()) return false;
            return true;
        } else if (triggerEvent == PreHpLost) {
            if (!TriggerSkill::triggerable(player)) return false;
            if (player->getPile("heavenly_army").isEmpty()) return false;
            return true;
        } else {
            if (player == NULL) return false;
            if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who->hasSkill(objectName())) {
                    foreach (ServerPlayer *p, room->getAllPlayers())
                        if (p->hasSkill("hongfa_slash")) {
                            room->detachSkillFromPlayer(p, "hongfa_slash", true);
                        }
                return false;
                }
            }
            foreach (ServerPlayer *p, room->getAllPlayers())
                if (p->hasSkill("hongfa_slash")) {
                    room->detachSkillFromPlayer(p, "hongfa_slash", true);
                }

                ServerPlayer *zhangjiao = room->findPlayerBySkillName(objectName());
                if (!zhangjiao || !zhangjiao->hasShownSkill(this)) return false;
                foreach (ServerPlayer *p, room->getAllPlayers())
                    if (!p->hasSkill("hongfa_slash"))
                        room->attachSkillToPlayer(p, "hongfa_slash");
        }
        return true;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == EventPhaseStart) return true;
        if (triggerEvent == PreHpLost) return player->askForSkillInvoke(objectName());
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == EventPhaseStart) {
            int num = 0;
            foreach (auto p, room->getAllPlayers())
                if (p->hasShownOneGeneral() && p->getKingdom() == player->getKingdom())
                    num ++;
            int num2 = player->getPile("heavenly_army").length();
            QList<int> tianbing = room->getNCards(num + num2);
            player->addToPile("heavenly_army", tianbing);
            return false;
        } else {
            room->fillAG(player->getPile("heavenly_army"), player);
            int card_id = room->askForAG(player, player->getPile("heavenly_army"), false, "hongfa");
            room->clearAG(player);

            if (card_id != -1) {
                CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                return true;
            }
        }

        return false;
    }
};

WendaoCard::WendaoCard() {
    target_fixed = true;
}

void WendaoCard::onUse(Room *room, const CardUseStruct &card_use) const{
    Card *card = Sanguosha->getCard(109);
    if (!card) return;
    if (room->getCardPlace(109) == Player::PlaceTable || room->getCardPlace(109) == Player::PlaceEquip || room->getCardPlace(109) == Player::DiscardPile)
        room->obtainCard(card_use.from, card);
}

class Wendao: public OneCardViewAsSkill {
public:
    Wendao(): OneCardViewAsSkill("wendao") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("WendaoCard") && player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        WendaoCard *card = new WendaoCard;
        card->addSubcard(originalCard);
        card->setShowSkill(objectName());
        return card;
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

    General *mifuren = new General(this, "mifuren", "shu", 3, false); // SHU 021
    mifuren->addSkill(new Guixiu);
    mifuren->addSkill(new Cunsi);
    mifuren->addSkill(new CunsiStart);
    related_skills.insertMulti("cunsi", "#cunsi-start");
    mifuren->addRelateSkill("yongjue");

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

    General *dongzhuo = new General(this, "dongzhuo", "qun", 4); // QUN 006
    dongzhuo->addSkill(new Hengzheng);
    dongzhuo->addSkill(new Baoling);

    General *zhangren = new General(this, "zhangren", "qun", 3); // QUN 024
    zhangren->addSkill(new Chuanxin);
    zhangren->addSkill(new Fengshi);

    skills << new Yongjue << new YongjueStart << new Benghuai << new HongfaSlash;
    related_skills.insertMulti("yongjue", "#yongjue-start");

    addMetaObject<CunsiCard>();
    addMetaObject<DuanxieCard>();
    addMetaObject<FengshiSummon>();
    addMetaObject<HongfaCard>();
    addMetaObject<HongfaSlashCard>();
    addMetaObject<WendaoCard>();

    General *lord_zhangjiao = new General(this, "lord_zhangjiao$", "qun", 4, true, true);
    lord_zhangjiao->addSkill(new Wuxin);
    lord_zhangjiao->addSkill(new Hongfa);
    lord_zhangjiao->addSkill(new Wendao);
}

ADD_PACKAGE(Momentum)

/*PeaceSpell::PeaceSpell(Suit suit, int number)
    : Armor(Card::Heart, 3)
{
    setObjectName("PeaceSpell");
}

class PeaceSpellSkill: public ArmorSkill{
public:
    PeaceSpellSkill(): ArmorSkill("PeaceSpell") {
        events << DamageInflicted;

    
MomentumEquipPackage::MomentumEquipPackage(): Package("momentum_equip", CardPack){
    PeaceSpell *dp = new PeaceSpell();
    dp->setParent(this);
}

ADD_PACKAGE(MomentumEquip)*/
