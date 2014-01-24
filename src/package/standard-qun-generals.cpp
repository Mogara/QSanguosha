#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"

class Jijiu: public OneCardViewAsSkill {
public:
    Jijiu(): OneCardViewAsSkill("jijiu") {
        filter_pattern = ".|red";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach") && !player->hasFlag("Global_PreventPeach")
                && player->getPhase() == Player::NotActive && player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Peach *peach = new Peach(originalCard->getSuit(), originalCard->getNumber());
        peach->addSubcard(originalCard->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

QingnangCard::QingnangCard() {
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

class Qingnang: public OneCardViewAsSkill {
public:
    Qingnang(): OneCardViewAsSkill("qingnang") {
        filter_pattern = ".|.|.|hand!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "h") && !player->hasUsed("QingnangCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(originalCard->getId());
        return qingnang_card;
    }
};

class Wushuang: public TriggerSkill {
public:
    Wushuang(): TriggerSkill("wushuang") {
        events << TargetConfirmed << CardFinished;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
		if (player->hasShownSkill(this)) return true;
		if (player->askForSkillInvoke(objectName())) return true;
		return false;
	}

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            bool can_invoke = false;
            if (use.card->isKindOf("Slash") && TriggerSkill::triggerable(use.from) && use.from == player) {
                can_invoke = true;
                QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
                for (int i = 0; i < use.to.length(); i++) {
                    if (jink_list.at(i).toInt() == 1)
                        jink_list.replace(i, QVariant(2));
                }
                player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
            }
            if (use.card->isKindOf("Duel")) {
                if (TriggerSkill::triggerable(use.from) && use.from == player)
                    can_invoke = true;
                if (TriggerSkill::triggerable(player) && use.to.contains(player))
                    can_invoke = true;
            }
            if (!can_invoke) return false;

            LogMessage log;
            log.from = player;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            room->broadcastSkillInvoke(objectName());
            if (use.card->isKindOf("Duel"))
                room->setPlayerMark(player, "WushuangTarget", 1);
        } else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Duel")) {
                foreach (ServerPlayer *lvbu, room->getAllPlayers())
                    if (lvbu->getMark("WushuangTarget") > 0) room->setPlayerMark(lvbu, "WushuangTarget", 0);
            }
        }

        return false;
    }
};

LijianCard::LijianCard() {
    mute = true;
}

bool LijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!to_select->isMale())
        return false;

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->deleteLater();
    if (targets.isEmpty() && Self->isProhibited(to_select, duel))
        return false;

    if (targets.length() == 1 && to_select->isCardLimited(duel, Card::MethodUse))
        return false;

    return targets.length() < 2 && to_select != Self;
}

bool LijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LijianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *diaochan = card_use.from;

    LogMessage log;
    log.from = diaochan;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, diaochan, data);
    room->broadcastSkillInvoke("lijian");

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, diaochan->objectName(), QString(), "lijian", QString());
    room->moveCardTo(this, diaochan, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, diaochan, data);
    thread->trigger(CardFinished, room, diaochan, data);
}

void LijianCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName(QString("_%1").arg(getSkillName()));
    if (!from->isCardLimited(duel, Card::MethodUse) && !from->isProhibited(to, duel))
        room->useCard(CardUseStruct(duel, from, to));
    else
        delete duel;
}

class Lijian: public OneCardViewAsSkill {
public:
    Lijian(): OneCardViewAsSkill("lijian") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getAliveSiblings().length() > 1
               && player->canDiscard(player, "he") && !player->hasUsed("LijianCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(originalCard->getId());
        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        return card->isKindOf("Duel") ? 0 : -1;
    }
};

class Biyue: public PhaseChangeSkill {
public:
    Biyue(): PhaseChangeSkill("biyue") {
        frequency = Frequent;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->getPhase() == Player::Finish && player->askForSkillInvoke(objectName()))
		    return true;
		return false;
	}

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        Room *room = diaochan->getRoom();
        room->broadcastSkillInvoke(objectName());
        diaochan->drawCards(1);

        return false;
    }
};

class Luanji: public ViewAsSkill {
public:
    Luanji(): ViewAsSkill("luanji") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.isEmpty())
            return !to_select->isEquipped();
        else if (selected.length() == 1) {
            const Card *card = selected.first();
            return !to_select->isEquipped() && to_select->getSuit() == card->getSuit();
        } else
            return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 2) {
            ArcheryAttack *aa = new ArcheryAttack(Card::SuitToBeDecided, 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        } else
            return NULL;
    }
};

class ShuangxiongViewAsSkill: public OneCardViewAsSkill {
public:
    ShuangxiongViewAsSkill():OneCardViewAsSkill("shuangxiong") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("shuangxiong") != 0 && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card *card) const{
        if (card->isEquipped())
            return false;

        int value = Self->getMark("shuangxiong");
        if (value == 1)
            return card->isBlack();
        else if (value == 2)
            return card->isRed();

        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Duel *duel = new Duel(originalCard->getSuit(), originalCard->getNumber());
        duel->addSubcard(originalCard);
        duel->setSkillName(objectName());
        return duel;
    }
};

class Shuangxiong: public TriggerSkill {
public:
    Shuangxiong(): TriggerSkill("shuangxiong") {
        events << EventPhaseStart << FinishJudge << EventPhaseChanging;
        view_as_skill = new ShuangxiongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *shuangxiong, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
		    if (shuangxiong->getPhase() == Player::Start) {
				room->setPlayerMark(shuangxiong, "shuangxiong", 0);
				return false;
			} else if (shuangxiong->getPhase() == Player::Draw && TriggerSkill::triggerable(shuangxiong)) {
				if (shuangxiong->askForSkillInvoke(objectName())) return true;
			}
		} else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason == "shuangxiong"){
                shuangxiong->obtainCard(judge->card);
                judge->pattern = judge->card->isRed() ? "red" : "black";
            }
			return false;
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive && shuangxiong->hasFlag("shuangxiong"))
                room->setPlayerFlag(shuangxiong, "-shuangxiong");
			return false;
        }

		return false;
	}

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *shuangxiong, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (shuangxiong->getPhase() == Player::Draw && TriggerSkill::triggerable(shuangxiong)) {
                room->setPlayerFlag(shuangxiong, "shuangxiong");

                room->broadcastSkillInvoke("shuangxiong", 1);
                JudgeStruct judge;
                judge.good = true;
                judge.play_animation = false;
                judge.reason = objectName();
                judge.who = shuangxiong;

                room->judge(judge);
                room->setPlayerMark(shuangxiong, "shuangxiong", judge.pattern == "red" ? 1 : 2);

                return true;
            }
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 2;
    }
};

class Wansha: public TriggerSkill {
public:
    Wansha(): TriggerSkill("wansha") {
        events << AskForPeaches << EventPhaseChanging << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
		if (triggerEvent == AskForPeaches) {
			if (player->hasShownSkill(this)) return true;
			if (player->askForSkillInvoke(objectName())) return true;
			return false;
		} else {
            if (triggerEvent == EventPhaseChanging) {
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.to != Player::NotActive) return false;
            } else if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player || death.who->getPhase() == Player::NotActive) return false;
            }
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("Global_PreventPeach"))
                    room->setPlayerFlag(p, "-Global_PreventPeach");
            }
        }
		return false;
	}

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == AskForPeaches) {
            DyingStruct dying = data.value<DyingStruct>();
            ServerPlayer *jiaxu = room->getCurrent();
            if (!jiaxu || !TriggerSkill::triggerable(jiaxu) || jiaxu->getPhase() == Player::NotActive)
                return false;
            if (player == room->getAllPlayers().first()) {
                if (jiaxu->hasInnateSkill("wansha") || !jiaxu->hasSkill("jilve"))
                    room->broadcastSkillInvoke(objectName());
                else
                    room->broadcastSkillInvoke("jilve", 3);

                room->notifySkillInvoked(jiaxu, objectName());

                LogMessage log;
                log.from = jiaxu;
                log.arg = objectName();
                if (jiaxu != dying.who) {
                    log.type = "#WanshaTwo";
                    log.to << dying.who;
                } else {
                    log.type = "#WanshaOne";
                }
                room->sendLog(log);
            }
            if (dying.who != player && jiaxu != player)
                room->setPlayerFlag(player, "Global_PreventPeach");
        }
        return false;
    }
};

LuanwuCard::LuanwuCard() {
    mute = true;
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->removePlayerMark(source, "@chaos");
    room->broadcastSkillInvoke("luanwu");
    QString lightbox = "$LuanwuAnimate";
    if (source->getGeneralName() != "jiaxu" && (source->getGeneralName() == "sp_jiaxu" || source->getGeneral2Name() == "sp_jiaxu"))
        lightbox = lightbox + "SP";
    room->doLightbox(lightbox, 3000);

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach (ServerPlayer *player, players) {
        if (player->isAlive())
            room->cardEffect(this, source, player);
            room->getThread()->delay();
    }
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach (ServerPlayer *player, players) {
        int distance = effect.to->distanceTo(player);
        distance_list << distance;
        nearest = qMin(nearest, distance);
    }

    QList<ServerPlayer *> luanwu_targets;
    for (int i = 0; i < distance_list.length(); i++) {
        if (distance_list[i] == nearest && effect.to->canSlash(players[i], NULL, false))
            luanwu_targets << players[i];
    }

    if (luanwu_targets.isEmpty() || !room->askForUseSlashTo(effect.to, luanwu_targets, "@luanwu-slash"))
        room->loseHp(effect.to);
}

class Luanwu: public ZeroCardViewAsSkill {
public:
    Luanwu(): ZeroCardViewAsSkill("luanwu") {
        frequency = Limited;
        limit_mark = "@chaos";
    }

    virtual const Card *viewAs() const{
        Card *card = new LuanwuCard;
		card->setShowSkill(objectName());
		return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@chaos") >= 1;
    }
};

class Weimu: public TriggerSkill {
public:
    Weimu(): TriggerSkill("weimu") {
        events << TargetConfirming;
		frequency = Compulsory;
    }

	virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
		if (player->hasShownSkill(this)) return true;
		CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card || use.card->getTypeId() != Card::TypeTrick)
            return false;
        if (!use.card->isBlack()) return false;
		return player->askForSkillInvoke(objectName());
	}

	virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        room->broadcastSkillInvoke(objectName());
        room->notifySkillInvoked(player, objectName());
        LogMessage log;
        if (use.from) {
            log.type = "$CancelTarget";
            log.from = use.from;
        } else {
            log.type = "$CancelTargetNoUser";
        }
        log.to = use.to;
        log.arg = use.card->objectName();
        room->sendLog(log);


        use.to.removeOne(player);
        data = QVariant::fromValue(use);
        return false;
    }
};

class Mengjin: public TriggerSkill {
public:
    Mengjin():TriggerSkill("mengjin") {
        events << SlashMissed;
    }

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *pangde, QVariant &data) const{
		SlashEffectStruct effect = data.value<SlashEffectStruct>();
		if (effect.to->isAlive() && pangde->canDiscard(effect.to, "he"))
            return pangde->askForSkillInvoke(objectName(), data);
		return false;
	}

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *pangde, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        room->broadcastSkillInvoke(objectName());
        int to_throw = room->askForCardChosen(pangde, effect.to, "he", objectName(), false, Card::MethodDiscard);
        room->throwCard(Sanguosha->getCard(to_throw), effect.to, pangde);

        return false;
    }
};

class Leiji: public TriggerSkill {
public:
    Leiji(): TriggerSkill("leiji") {
        events << CardResponded;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardStar card_star = data.value<CardResponseStruct>().m_card;
        if (card_star->isKindOf("Jink")) {
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "leiji-invoke", true, true);
            if (target) {
				player->tag["leiji-target"] = QVariant::fromValue(target);
				return true;
			} else {
				player->tag.remove("leiji-target");
				return false;
			}
		}
		return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardResponseStruct>().m_card;
        if (card_star->isKindOf("Jink")) {
            PlayerStar target = zhangjiao->tag["leiji-target"].value<PlayerStar>();
            if (target) {
                room->broadcastSkillInvoke(objectName());

                JudgeStruct judge;
                judge.pattern = ".|spade";
                judge.good = false;
                judge.negative = true;
                judge.reason = objectName();
                judge.who = target;

                room->judge(judge);

                if (judge.isBad())
                    room->damage(DamageStruct(objectName(), zhangjiao, target, 2, DamageStruct::Thunder));
            }
			zhangjiao->tag.remove("leiji-target");
        }
        return false;
    }
};

class Guidao: public TriggerSkill {
public:
    Guidao(): TriggerSkill("guidao") {
        events << AskForRetrial;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer *ask_who) const{
        if (!TriggerSkill::triggerable(target))
            return false;

        if (target->isKongcheng()) {
            bool has_black = false;
            for (int i = 0; i < 4; i++) {
                const EquipCard *equip = target->getEquip(i);
                if (equip && equip->isBlack()) {
                    has_black = true;
                    break;
                }
            }
            return has_black;
        } else
            return true;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                    << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");

        const Card *card = room->askForCard(player, ".|black", prompt, data, Card::MethodResponse, judge->who, true);

        if (card) {
            room->broadcastSkillInvoke(objectName());
            player->tag["guidao_card"] = QVariant::fromValue(card);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        const Card *card = player->tag["guidao_card"].value<const Card *>();

        if (card != NULL) {
            room->broadcastSkillInvoke(objectName());
            room->retrial(card, player, judge, objectName(), true);
        }
        return false;
    }
};

class Beige: public TriggerSkill {
public:
    Beige(): TriggerSkill("beige") {
        events << Damaged << FinishJudge;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer* &ask_who) const{
        if (triggerEvent == Damaged) ask_who = room->findPlayerBySkillName(objectName());
		return target != NULL;
    }

	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
		if (triggerEvent == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
            if (damage.card == NULL || !damage.card->isKindOf("Slash") || damage.to->isDead())
                return false;

            ServerPlayer * caiwenji = ask_who;
			if (caiwenji->canDiscard(caiwenji, "he") && room->askForCard(caiwenji, "..", "@beige", data, objectName()))
				return true;
		} else {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason != objectName()) return false;
            judge->pattern = QString::number(int(judge->card->getSuit()));
			return false;
        }
		return false;
	}

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        ServerPlayer *caiwenji = ask_who;
		DamageStruct damage = data.value<DamageStruct>();

        room->broadcastSkillInvoke(objectName());

        JudgeStruct judge;
        judge.good = true;
        judge.play_animation = false;
        judge.who = player;
        judge.reason = objectName();

        room->judge(judge);

        Card::Suit suit = (Card::Suit)(judge.pattern.toInt());
        switch (suit) {
        case Card::Heart: {
                RecoverStruct recover;
                recover.who = caiwenji;
                room->recover(player, recover);

                break;
            }
        case Card::Diamond: {
                player->drawCards(2);
                break;
            }
        case Card::Club: {
                if (damage.from && damage.from->isAlive())
                    room->askForDiscard(damage.from, "beige", 2, 2, false, true);

                break;
            }
        case Card::Spade: {
                if (damage.from && damage.from->isAlive())
                    damage.from->turnOver();

                break;
            }
        default:
                break;
        }
        return false;
    }
};

class Duanchang: public TriggerSkill {
public:
    Duanchang(): TriggerSkill("duanchang") {
        events << Death;
        frequency = Compulsory;
    }

	virtual bool canPreshow() const {
		return false;
	}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;

        if (death.damage && death.damage->from) {
			ServerPlayer *target = death.damage->from;
			QString choice = room->askForChoice(player, objectName(), "head_general+deputy_general", data);
            LogMessage log;
            log.type = choice == "head_general" ? "#DuanchangLoseHeadSkills" : "#DuanchangLoseDeputySkills";
            log.from = player;
            log.to << target;
            log.arg = objectName();
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(player, objectName());

			if (choice == "head_general")
				target->tag["Duanchang"] = QString("head");
			else
				target->tag["Duanchang"] = QString("deputy");
			
			QList<const Skill *> skills = choice == "head_general" ? target->getHeadSkillList()
																   : target->getDeputySkillList();
			QStringList detachList;
			foreach (const Skill *skill, skills) {
				if (skill->getLocation() == Skill::Right && !skill->isAttachedLordSkill())
					detachList.append("-" + skill->objectName());
			}
			room->handleAcquireDetachSkills(death.damage->from, detachList);

			if (death.damage->from->isAlive())
				death.damage->from->gainMark("@duanchang");
        }

        return false;
    }
};

void StandardPackage::addQunGenerals()
{
	General *huatuo = new General(this, "huatuo", "qun", 3); // QUN 001
    huatuo->addSkill(new Jijiu);
    huatuo->addSkill(new Qingnang);

    General *lvbu = new General(this, "lvbu", "qun", 5); // QUN 002
    lvbu->addCompanion("diaochan");
    lvbu->addSkill(new Wushuang);

    General *diaochan = new General(this, "diaochan", "qun", 3, false); // QUN 003
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);

    General *yuanshao = new General(this, "yuanshao", "qun"); // QUN 004
    yuanshao->addCompanion("yanliangwenchou");
    yuanshao->addSkill(new Luanji);

    General *yanliangwenchou = new General(this, "yanliangwenchou", "qun"); // QUN 005
    yanliangwenchou->addSkill(new Shuangxiong);

    General *jiaxu = new General(this, "jiaxu", "qun", 3); // QUN 007
    jiaxu->addSkill(new Wansha);
    jiaxu->addSkill(new Luanwu);
    jiaxu->addSkill(new Weimu);

    General *pangde = new General(this, "pangde", "qun"); // QUN 008
    pangde->addSkill(new Mashu("pangde"));
    pangde->addSkill(new Mengjin);

    General *zhangjiao = new General(this, "zhangjiao", "qun", 3); // QUN 010
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Guidao);

    General *caiwenji = new General(this, "caiwenji", "qun", 3, false); // QUN 012
    caiwenji->addSkill(new Beige);
    caiwenji->addSkill(new Duanchang);

    addMetaObject<QingnangCard>();
    addMetaObject<LijianCard>();
    addMetaObject<LuanwuCard>();
}