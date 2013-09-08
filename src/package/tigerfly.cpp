#include "skill.h"
#include "clientplayer.h"
#include "standard.h"
#include "tigerfly.h"
#include "engine.h"

class Shemi: public ViewAsSkill {
public:
	Shemi():ViewAsSkill("shemi") {
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		int hp = Self->getHp();
		if (hp <= 2) {
			return selected.length() < 1;
		}else {
			return selected.length() < 2;
		};
    };

	virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShemiAG");
    };

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
		int hp = Self->getHp();
		if ((hp <= 2 && cards.length() != 1) || (hp > 2 && cards.length() != 2))
			return NULL;

		Card *amazing_grace = new AmazingGrace(Card::SuitToBeDecided, -1);
		amazing_grace->addSubcards(cards);
		amazing_grace->setSkillName(objectName());
		return amazing_grace;
    };

};

class Kuanhui: public TriggerSkill{
public:
    Kuanhui(): TriggerSkill("kuanhui") {
        events << TargetConfirmed << CardEffected << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (triggerEvent == TargetConfirmed && TriggerSkill::triggerable(player)){
            CardUseStruct use = data.value<CardUseStruct>();
            const Card *card = use.card;
            if (card->isKindOf("SkillCard"))
                return false;
            if (use.to.length() >= 2){
                room->setTag("kuanhui_user", card->toString());
                if (room->askForSkillInvoke(selfplayer, "kuanhui", data)){
                    room->broadcastSkillInvoke("kuanhui", 1);
                    room->getThread()->delay(1000);
                    JudgeStruct judge;
                    judge.pattern = ".|diamond";
                    judge.good = false;
                    judge.reason = objectName();
                    judge.who = selfplayer;
                    room->judge(judge);
                    room->getThread()->delay(1000);
                    if (judge.isGood()){
                        room->broadcastSkillInvoke("kuanhui", 2);
                        ServerPlayer *target = room->askForPlayerChosen(selfplayer, use.to, objectName());
                        room->setPlayerFlag(target, "kuanhuitarget");
                        room->setPlayerMark(target, "kuanhuiCardId", card->getId() + 1);
                        LogMessage log;
                        log.type = "#Kuanhui1";
                        log.from = selfplayer;
                        log.arg = objectName();
                        log.to.append(target);
                        room->sendLog(log);
                    }
                    else
                        room->broadcastSkillInvoke("kuanhui", 3);
                }
                room->removeTag("kuanhui_user");
            }
        }
        else if (triggerEvent == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("Slash"))
                return false;
            if (effect.to != NULL && effect.to->hasFlag("kuanhuitarget") 
                    && (effect.card->getId() == effect.to->getMark("kuanhuiCardId") - 1)){
                room->setPlayerFlag(effect.to, "-kuanhuitarget");
                room->setPlayerMark(effect.to, "kuanhuiCardId", 0);
                LogMessage log;
                log.type = "#kuanhui2";
                log.from = selfplayer;
                log.arg = objectName();
                log.to.append(effect.to);
                room->sendLog(log);
                return true;
            }
        }
        else{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.to != NULL && effect.to->hasFlag("kuanhuitarget") 
                    && (effect.slash->getId() == effect.to->getMark("kuanhuiCardId") - 1)){
                room->setPlayerFlag(effect.to, "-kuanhuitarget");
                room->setPlayerMark(effect.to, "kuanhuiCardId", 0);
                LogMessage log;
                log.type = "#kuanhui2";
                log.from = selfplayer;
                log.arg = objectName();
                log.to.append(effect.to);
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class Hongliang: public MasochismSkill{
public:
    Hongliang(): MasochismSkill("hongliang$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill(objectName()) && target->isAlive();
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("wei", player);
        if (lieges.length() == 0)
            return;
        room->setTag("honglianglord", QVariant::fromValue(player));
        foreach (ServerPlayer *p, lieges){
            if (!p->isNude()){
                const Card *card = room->askForCard(p, ".|.|.|.|.", "@HongliangGive", QVariant::fromValue(damage), Card::MethodNone);
                if (card != NULL){
                    if (!player->isLord() && player->hasSkill("weidi"))
                        room->broadcastSkillInvoke("weidi");
                    else
                        room->broadcastSkillInvoke("hongliang");
                    room->getThread()->delay(1000);
                    CardMoveReason reason;
                    reason.m_reason = CardMoveReason::S_REASON_GIVE;
                    reason.m_playerId = player->objectName();
                    reason.m_targetId = p->objectName();
                    room->moveCardTo(card, p, player, Player::PlaceHand, reason);
                    p->drawCards(1);
                }
            }
        }
        room->setTag("honglianglord", QVariant());
    }
};

PozhenCard::PozhenCard(){
    mute = true;
}

bool PozhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if (targets.length() == 0)
        return to_select->hasEquip();
    return false;
}

QString PozhenCard::suittb(Card::Suit s) const{
    switch (s){
        case Card::Club:
            return "club";
            break;
        case Card::Diamond:
            return "diamond";
            break;
        case Card::Heart:
            return "heart";
            break;
        case Card::Spade:
            return "spade";
            break;
        default:
            return "unknown";
            break;
    }
    return "unknown";
}

void PozhenCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *dest = effect.to;
    Room *room = dest->getRoom();
    room->setTag("pozhen_dest", QVariant::fromValue(dest));
    room->broadcastSkillInvoke("pozhen", 1);
    ServerPlayer *source = effect.from;
    room->setTag("pozhen_source", QVariant::fromValue(source));
    Card::Suit suit = room->askForSuit(source, objectName());
    LogMessage log;
    log.type = "#pozhensuit";
    log.from = source;
    log.arg = suittb(suit);
    room->sendLog(log);
    QString pattern = ".|" + suittb(suit) + "|.|hand|.";
    if (!dest->isKongcheng() &&
            room->askForCard(dest, pattern, "@pozhen:" + source->objectName() + "::" + suittb(suit), QVariant(), objectName())){
        room->broadcastSkillInvoke("pozhen", 2);
        source->setFlags("Global_EndPlayPhase");
    }
    else{
        room->broadcastSkillInvoke("pozhen", 3);
        QList<int> idlist;
        QList<const Card *> equips = dest->getEquips();
        foreach (const Card *equip, equips)
            idlist << equip->getId();
        CardsMoveStruct move;
        move.card_ids = idlist;
        move.to = dest;
        move.to_place = Player::PlaceHand;
        room->moveCards(move, true);
        source->drawCards(1);
    }
    room->removeTag("pozhen_dest");
    room->removeTag("pozhen_source");
}

class Pozhen: public ZeroCardViewAsSkill{
public:
    Pozhen(): ZeroCardViewAsSkill("pozhen"){
    }

    virtual const Card *viewAs() const{
        return new PozhenCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("PozhenCard");
    }
};

class Huaming: public TriggerSkill{
public:
    Huaming(): TriggerSkill("huaming"){
        frequency = Limited;
        events << Death;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Death){
            DeathStruct death = data.value<DeathStruct>();
            if (player->objectName() == death.who->objectName() && death.who->hasSkill(objectName())){
                if (death.damage && death.damage->from && death.damage->from->objectName() != player->objectName()){
                    room->setTag("huamingkiller", QVariant::fromValue(death.damage->from));
                    room->setTag("shanfu", QVariant::fromValue(death.who));
                }
            }
        }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }
};

class Poli: public PhaseChangeSkill{
public:
    Poli(): PhaseChangeSkill("poli"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if (player->getPhase() == Player::Finish){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, players){
                if (p->inMyAttackRange(player))
                    targets << p;
            }
            if (targets.length() >= 3){
                QString choice;
                if (!player->isNude())
                    choice = room->askForChoice(player, objectName(), "discard+changehero");
                else
                    choice = "changehero";
                if (choice == "discard"){
                    room->broadcastSkillInvoke(objectName(), 1);
                    room->askForDiscard(player, objectName(), 1, 1, false, true);
                }
                else{
                    room->broadcastSkillInvoke(objectName(), 2);
                    room->doLightbox("$PoliAnimate", 5000);
                    room->loseMaxHp(player);
                    room->drawCards(player, 2);
                    room->showAllCards(player);
                    QStringList l;
                    l << "wuyan" << "jujian" << "-pozhen" << "-huaming" << "-poli";
                    room->handleAcquireDetachSkills(player, l);
                }
            }
        }
        return false;
    }
};

TigerFlyPackage::TigerFlyPackage(): Package("tigerfly") {
	General *caorui = new General(this, "caorui$", "wei", 3);
	caorui->addSkill(new Shemi);
    caorui->addSkill(new Kuanhui);
    caorui->addSkill(new Hongliang);

    General *shanfu = new General(this, "shanfu", "shu", 4);
    shanfu->addSkill(new Pozhen);
    shanfu->addSkill(new Huaming);
    shanfu->addSkill(new Poli);

    addMetaObject<PozhenCard>();
};

ADD_PACKAGE(TigerFly)
