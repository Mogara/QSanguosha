#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "standard.h"

SavageAssaultAvoid::SavageAssaultAvoid(const QString &avoid_skill): TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill){
    events << CardEffected;
}

bool SavageAssaultAvoid::effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
    CardEffectStruct effect = data.value<CardEffectStruct>();
    if (effect.card->isKindOf("SavageAssault")) {
        room->broadcastSkillInvoke(avoid_skill);

        LogMessage log;
        log.type = "#SkillNullify";
        log.from = player;
        log.arg = avoid_skill;
        log.arg2 = "savage_assault";
        room->sendLog(log);

        return true;
    } else
        return false;
}



class Huoshou: public TriggerSkill {
public:
    Huoshou(): TriggerSkill("huoshou") {
        events << TargetConfirmed << ConfirmDamage << CardFinished;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed && TriggerSkill::triggerable(player)) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault") && use.from != player) {
                room->notifySkillInvoked(player, objectName());
                room->broadcastSkillInvoke(objectName());
                room->setTag("HuoshouSource", QVariant::fromValue((PlayerStar)player));
            }
        } else if (triggerEvent == ConfirmDamage && !room->getTag("HuoshouSource").isNull()) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->isKindOf("SavageAssault"))
                return false;

            ServerPlayer *menghuo = room->getTag("HuoshouSource").value<PlayerStar>();
            damage.from = menghuo->isAlive() ? menghuo : NULL;
            data = QVariant::fromValue(damage);
        } else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault"))
                room->removeTag("HuoshouSource");
        }

        return false;
    }
};

class Lieren: public TriggerSkill {
public:
    Lieren(): TriggerSkill("lieren") {
        events << Damage;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if (damage.card && damage.card->isKindOf("Slash") && !zhurong->isKongcheng()
            && !target->isKongcheng() && target != zhurong && !damage.chain && !damage.transfer
            && room->askForSkillInvoke(zhurong, objectName(), data)) {
            room->broadcastSkillInvoke(objectName(), 1);

            bool success = zhurong->pindian(target, "lieren", NULL);
            if (!success) return false;

            room->broadcastSkillInvoke(objectName(), 2);
            if (!target->isNude()) {
                int card_id = room->askForCardChosen(zhurong, target, "he", objectName());
                CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, zhurong->objectName());
                room->obtainCard(zhurong, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
            }
        }

        return false;
    }
};

class Zaiqi: public PhaseChangeSkill {
public:
    Zaiqi(): PhaseChangeSkill("zaiqi") {
    }

    virtual bool onPhaseChange(ServerPlayer *menghuo) const{
        if (menghuo->getPhase() == Player::Draw && menghuo->isWounded()) {
            Room *room = menghuo->getRoom();
            if (room->askForSkillInvoke(menghuo, objectName())) {
                room->broadcastSkillInvoke(objectName(), 1);

                bool has_heart = false;
                int x = menghuo->getLostHp();
                QList<int> ids = room->getNCards(x, false);
                CardsMoveStruct move(ids, menghuo, Player::PlaceTable,
                                     CardMoveReason(CardMoveReason::S_REASON_TURNOVER, menghuo->objectName(), "zaiqi", QString()));
                room->moveCardsAtomic(move, true);

                room->getThread()->delay();
                room->getThread()->delay();

                QList<int> card_to_throw;
                QList<int> card_to_gotback;
                for (int i = 0; i < x; i++) {
                    if (Sanguosha->getCard(ids[i])->getSuit() == Card::Heart)
                        card_to_throw << ids[i];
                    else
                        card_to_gotback << ids[i];
                }
                if (!card_to_throw.isEmpty()) {
                    DummyCard *dummy = new DummyCard(card_to_throw);

                    RecoverStruct recover;
                    recover.who = menghuo;
                    recover.recover = card_to_throw.length();
                    room->recover(menghuo, recover);

                    CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, menghuo->objectName(), "zaiqi", QString());
                    room->throwCard(dummy, reason, NULL);
                    dummy->deleteLater();
                    has_heart = true;
                }
                if (!card_to_gotback.isEmpty()) {
                    DummyCard *dummy2 = new DummyCard(card_to_gotback);
                    CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, menghuo->objectName());
                    room->obtainCard(menghuo, dummy2, reason);
                    dummy2->deleteLater();
                }

                if (has_heart)
                    room->broadcastSkillInvoke(objectName(), 2);

                return true;
            }
        }

        return false;
    }
};

class Juxiang: public TriggerSkill {
public:
    Juxiang(): TriggerSkill("juxiang") {
        events << CardUsed << BeforeCardsMove;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SavageAssault")) {
                if (use.card->isVirtualCard() && use.card->subcardsLength() != 1)
                    return false;
                if (Sanguosha->getEngineCard(use.card->getEffectiveId())
                    && Sanguosha->getEngineCard(use.card->getEffectiveId())->isKindOf("SavageAssault"))
                    room->setCardFlag(use.card->getEffectiveId(), "real_SA");
            }
        } else if (TriggerSkill::triggerable(player)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.card_ids.length() == 1 && move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
                && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
                Card *card = Sanguosha->getCard(move.card_ids.first());
                if (card->hasFlag("real_SA") && player != move.from) {
                    room->notifySkillInvoked(player, objectName());
                    room->broadcastSkillInvoke(objectName());
                    LogMessage log;
                    log.type = "#TriggerSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);

                    player->obtainCard(card);
                    move.card_ids.clear();
                    data = QVariant::fromValue(move);
                }
            }
        }

        return false;
    }
};

class Jiuchi: public OneCardViewAsSkill {
public:
    Jiuchi(): OneCardViewAsSkill("jiuchi") {
        filter_pattern = ".|spade|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Analeptic *analeptic = new Analeptic(originalCard->getSuit(), originalCard->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(originalCard->getId());
        return analeptic;
    }
};

class Roulin: public TriggerSkill {
public:
    Roulin(): TriggerSkill("roulin") {
        events << TargetConfirmed;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && (target->hasSkill(objectName()) || target->isFemale());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && player == use.from) {
            QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();
            int index = 0;
            bool play_effect = false;
            if (TriggerSkill::triggerable(use.from)) {
                foreach (ServerPlayer *p, use.to) {
                    if (p->isFemale()) {
                        play_effect = true;
                        if (jink_list.at(index).toInt() == 1)
                            jink_list.replace(index, QVariant(2));
                    }
                    index++;
                }
                use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
                if (play_effect) {
                    LogMessage log;
                    log.from = use.from;
                    log.arg = objectName();
                    log.type = "#TriggerSkill";
                    room->sendLog(log);
                    room->notifySkillInvoked(use.from, objectName());

                    room->broadcastSkillInvoke(objectName(), 1);
                }
            } else if (use.from->isFemale()) {
                foreach (ServerPlayer *p, use.to) {
                    if (p->hasSkill(objectName())) {
                        play_effect = true;
                        if (jink_list.at(index).toInt() == 1)
                            jink_list.replace(index, QVariant(2));
                    }
                    index++;
                }
                use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
                if (play_effect) {
                    foreach (ServerPlayer *p, use.to) {
                        if (p->hasSkill(objectName())) {
                            LogMessage log;
                            log.from = p;
                            log.arg = objectName();
                            log.type = "#TriggerSkill";
                            room->sendLog(log);
                            room->notifySkillInvoked(p, objectName());
                        }
                    }
                    room->broadcastSkillInvoke(objectName(), 2);
                }
            }
        }

        return false;
    }
};

class Benghuai: public PhaseChangeSkill {
public:
    Benghuai(): PhaseChangeSkill("benghuai") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const{
        bool trigger_this = false;
        Room *room = dongzhuo->getRoom();

        if (dongzhuo->getPhase() == Player::Finish) {
            QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
            foreach (ServerPlayer *player, players) {
                if (dongzhuo->getHp() > player->getHp()) {
                    trigger_this = true;
                    break;
                }
            }
        }

        if (trigger_this) {
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
        }

        return false;
    }
};

ThicketPackage::ThicketPackage()
    : Package("thicket")
{
    General *menghuo = new General(this, "menghuo", "shu"); // SHU 014
    menghuo->addCompanion("zhurong");
    menghuo->addSkill(new SavageAssaultAvoid("huoshou"));
    menghuo->addSkill(new Huoshou);
    menghuo->addSkill(new Zaiqi);
    related_skills.insertMulti("huoshou", "#sa_avoid_huoshou");

    General *zhurong = new General(this, "zhurong", "shu", 4, false); // SHU 015
    zhurong->addSkill(new SavageAssaultAvoid("juxiang"));
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);
    related_skills.insertMulti("juxiang", "#sa_avoid_juxiang");

    General *dongzhuo = new General(this, "dongzhuo$", "qun", 8); // QUN 006
    dongzhuo->addSkill(new Jiuchi);
    dongzhuo->addSkill(new Roulin);
    dongzhuo->addSkill(new Benghuai);
}

ADD_PACKAGE(Thicket)

