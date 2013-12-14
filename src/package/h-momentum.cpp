#include "h-momentum.h"
#include "general.h"
#include "standard.h"
#include "standard-equips.h"
#include "maneuvering.h"
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

    virtual bool onPhaseChange(ServerPlayer *lidian) const{
        if (lidian->getPhase() == Player::Draw) {
            Room *room = lidian->getRoom();
            if (room->askForSkillInvoke(lidian, objectName())) {
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
        }

        return false;
    }
};

class Wangxi: public TriggerSkill {
public:
    Wangxi(): TriggerSkill("wangxi") {
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = NULL;
        if (triggerEvent == Damage)
            target = damage.to;
        else
            target = damage.from;
        if (!target || target == player) return false;
        QList<ServerPlayer *> players;
        players << player << target;
        room->sortByActionOrder(players);

        for (int i = 1; i <= damage.damage; i++) {
            if (!target->isAlive() || !player->isAlive())
                return false;
            if (room->askForSkillInvoke(player, objectName(), QVariant::fromValue((PlayerStar)target))) {
                room->broadcastSkillInvoke(objectName());
                room->drawCards(players, 1, objectName());
            }
        }
        return false;
    }
};

class Hengjiang: public MasochismSkill {
public:
    Hengjiang(): MasochismSkill("hengjiang") {
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        Room *room = target->getRoom();
        for (int i = 1; i <= damage.damage; i++) {
            ServerPlayer *current = room->getCurrent();
            if (!current || current->isDead() || current->getPhase() == Player::NotActive)
                break;
            if (room->askForSkillInvoke(target, objectName(), QVariant::fromValue((PlayerStar)current))) {
                room->broadcastSkillInvoke(objectName());
                room->addPlayerMark(current, "@hengjiang");
            }
        }
    }
};

class HengjiangDraw: public TriggerSkill {
public:
    HengjiangDraw(): TriggerSkill("#hengjiang-draw") {
        events << TurnStart << CardsMoveOneTime << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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

GuixiuCard::GuixiuCard() {
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

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
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

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &) const{
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

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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
};

HMomentumPackage::HMomentumPackage()
    : Package("h_momentum")
{
    General *lidian = new General(this, "lidian", "wei", 3); // WEI 017
    lidian->addSkill(new Xunxun);
    lidian->addSkill(new Wangxi);

    General *zangba = new General(this, "zangba", "wei", 4); // WEI 023
    zangba->addSkill(new Hengjiang);
    zangba->addSkill(new HengjiangDraw);
    zangba->addSkill(new HengjiangMaxCards);
    related_skills.insertMulti("hengjiang", "#hengjiang-draw");
    related_skills.insertMulti("hengjiang", "#hengjiang-maxcard");

    General *heg_madai = new General(this, "heg_madai", "shu", 4, true, true); // SHU 019
    heg_madai->addSkill("mashu");
    heg_madai->addSkill("qianxi");

    General *mifuren = new General(this, "mifuren", "shu", 3, false); // SHU 021
    mifuren->addSkill(new Guixiu);
    mifuren->addSkill(new GuixiuDetach);
    mifuren->addSkill(new Cunsi);
    mifuren->addSkill(new CunsiStart);
    related_skills.insertMulti("guixiu", "#guixiu-clear");
    related_skills.insertMulti("cunsi", "#cunsi-start");
    mifuren->addRelateSkill("yongjue");

    /*General *chenwudongxi = new General(this, "chenwudongxi", "wu", 4);

    General *heg_sunce = new General(this, "heg_sunce", "wu", 4); // WU 010 G

    General *heg_dongzhuo = new General(this, "heg_dongzhuo", "qun", 4); // QUN 006 G

    General *zhangren = new General(this, "zhangren", "qun", 3);*/

    skills << new Yongjue << new YongjueStart;
    related_skills.insertMulti("yongjue", "#yongjue-start");

    addMetaObject<GuixiuCard>();
    addMetaObject<CunsiCard>();
}

ADD_PACKAGE(HMomentum)
