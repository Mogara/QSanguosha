#include "yjcm-package.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "engine.h"
#include "settings.h"
#include "ai.h"
#include "general.h"

class Yizhong: public TriggerSkill {
public:
    Yizhong(): TriggerSkill("yizhong") {
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && target->getArmor() == NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.slash->isBlack()) {
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(player, objectName());

            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            room->sendLog(log);

            return true;
        }

        return false;
    }
};

class Luoying: public TriggerSkill {
public:
    Luoying(): TriggerSkill("luoying") {
        events << BeforeCardsMove;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *caozhi, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == caozhi || move.from == NULL)
            return false;
        if (move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD
                ||move.reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE)) {
            QList<int> card_ids;
            int i = 0;
            foreach (int card_id, move.card_ids) {
                if (Sanguosha->getCard(card_id)->getSuit() == Card::Club
                    && ((move.reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE
                         && move.from_places[i] == Player::PlaceJudge
                         && move.to_place == Player::DiscardPile)
                        || (move.reason.m_reason != CardMoveReason::S_REASON_JUDGEDONE
                            && room->getCardOwner(card_id) == move.from
                            && (move.from_places[i] == Player::PlaceHand || move.from_places[i] == Player::PlaceEquip))))
                    card_ids << card_id;
                i++;
            }
            if (card_ids.empty())
                return false;
            else if (caozhi->askForSkillInvoke(objectName(), data)) {
                int ai_delay = Config.AIDelay;
                Config.AIDelay = 0;
                while (!card_ids.empty()) {
                    room->fillAG(card_ids, caozhi);
                    int id = room->askForAG(caozhi, card_ids, true, objectName());
                    if (id == -1) {
                        room->clearAG(caozhi);
                        break;
                    }
                    card_ids.removeOne(id);
                    room->clearAG(caozhi);
                }
                Config.AIDelay = ai_delay;

                if (!card_ids.empty()) {
                    room->broadcastSkillInvoke("luoying", move.from->getGeneralName().contains("zhenji") ? 2 : 1);
                    foreach (int id, card_ids) {
                        if (move.card_ids.contains(id)) {
                            move.from_places.removeAt(move.card_ids.indexOf(id));
                            move.card_ids.removeOne(id);
                            data = QVariant::fromValue(move);
                        }
                        room->moveCardTo(Sanguosha->getCard(id), caozhi, Player::PlaceHand, move.reason, true);
                        if (!caozhi->isAlive())
                            break;
                    }
                }
            }
        }
        return false;
    }
};

class Jiushi: public ZeroCardViewAsSkill {
public:
    Jiushi(): ZeroCardViewAsSkill("jiushi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player) && player->faceUp();
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic") && player->faceUp();
    }

    virtual const Card *viewAs() const{
        Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
        analeptic->setSkillName(objectName());
        return analeptic;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class JiushiFlip: public TriggerSkill {
public:
    JiushiFlip(): TriggerSkill("#jiushi-flip") {
        events << PreCardUsed << PreDamageDone << DamageComplete;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == PreCardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->getSkillName() == "jiushi")
                player->turnOver();
        } else if (triggerEvent == PreDamageDone) {
            player->tag["PredamagedFace"] = player->faceUp();
        } else if (triggerEvent == DamageComplete) {
            bool faceup = player->tag.value("PredamagedFace").toBool();
            player->tag.remove("PredamagedFace");
            if (!faceup && !player->faceUp() && player->askForSkillInvoke("jiushi", data)) {
                room->broadcastSkillInvoke("jiushi", 3);
                player->turnOver();
            }
        }

        return false;
    }
};

class Wuyan: public TriggerSkill {
public:
    Wuyan(): TriggerSkill("wuyan") {
        events << DamageCaused << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->getTypeId() == Card::TypeTrick) {
            if (triggerEvent == DamageInflicted && player->hasSkill(objectName())) {
                LogMessage log;
                log.type = "#WuyanGood";
                log.from = player;
                log.arg = damage.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(player, objectName());
                room->broadcastSkillInvoke(objectName());

                return true;
            } else if (triggerEvent == DamageCaused && damage.from && TriggerSkill::triggerable(damage.from)) {
                LogMessage log;
                log.type = "#WuyanBad";
                log.from = player;
                log.arg = damage.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(player, objectName());
                room->broadcastSkillInvoke(objectName());

                return true;
            }
        }

        return false;
    }
};

JujianCard::JujianCard() {
}

bool JujianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void JujianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QStringList choicelist;
    choicelist << "draw";
    if (effect.to->isWounded())
        choicelist << "recover";
    if (!effect.to->faceUp() || effect.to->isChained())
        choicelist << "reset";
    QString choice = room->askForChoice(effect.to, "jujian", choicelist.join("+"));

    if (choice == "draw")
        effect.to->drawCards(2);
    else if (choice == "recover") {
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }
    else if (choice == "reset") {
        if (effect.to->isChained())
            room->setPlayerProperty(effect.to, "chained", false);
        if (!effect.to->faceUp())
            effect.to->turnOver();
    }
}

class JujianViewAsSkill: public OneCardViewAsSkill {
public:
    JujianViewAsSkill(): OneCardViewAsSkill("jujian") {
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        JujianCard *jujianCard = new JujianCard;
        jujianCard->addSubcard(originalCard);
        return jujianCard;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isKindOf("BasicCard") && !Self->isJilei(to_select);
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jujian";
    }
};

class Jujian: public PhaseChangeSkill {
public:
    Jujian(): PhaseChangeSkill("jujian") {
        view_as_skill = new JujianViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xushu) const{
        Room *room = xushu->getRoom();
        if (xushu->getPhase() == Player::Finish && xushu->canDiscard(xushu, "he"))
            room->askForUseCard(xushu, "@@jujian", "@jujian-card", -1, Card::MethodDiscard);
        return false;
    }
};

class Enyuan: public TriggerSkill {
public:
    Enyuan(): TriggerSkill("enyuan") {
        events << CardsMoveOneTime << Damaged;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to == player && move.from && move.from->isAlive() && move.from != move.to
                && move.card_ids.size() >= 2
                && move.reason.m_reason != CardMoveReason::S_REASON_PREVIEWGIVE) {
                move.from->setFlags("EnyuanDrawTarget");
                bool invoke = room->askForSkillInvoke(player, objectName(), data);
                move.from->setFlags("-EnyuanDrawTarget");
                if (invoke) {
                    room->drawCards((ServerPlayer *)move.from, 1);
                    room->broadcastSkillInvoke(objectName(), qrand() % 2 + 1);
                }
            }
        } else if (triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *source = damage.from;
            if (!source || source == player) return false;
            int x = damage.damage;
            for (int i = 0; i < x; i++) {
                if (source->isAlive() && player->isAlive() && room->askForSkillInvoke(player, objectName(), data)) {
                    room->broadcastSkillInvoke(objectName(), qrand() % 2 + 3);
                    const Card *card = NULL;
                    if (!source->isKongcheng())
                        card = room->askForExchange(source, objectName(), 1, false, "EnyuanGive", true);
                    if (card) {
                        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(),
                                              player->objectName(), objectName(), QString());
                        reason.m_playerId = player->objectName();
                        room->moveCardTo(card, source, player, Player::PlaceHand, reason);
                    } else {
                        room->loseHp(source);
                    }
                } else {
                    break;
                }
            }
        }
        return false;
    }
};

class Xuanhuo: public PhaseChangeSkill {
public:
    Xuanhuo(): PhaseChangeSkill("xuanhuo") {
    }

    virtual bool onPhaseChange(ServerPlayer *fazheng) const{
        Room *room = fazheng->getRoom();
        if (fazheng->getPhase() == Player::Draw) {
            ServerPlayer *to = room->askForPlayerChosen(fazheng, room->getOtherPlayers(fazheng), objectName(), "xuanhuo-invoke", true, true);
            if (to) {
                room->broadcastSkillInvoke(objectName());
                room->drawCards(to, 2);
                if (!fazheng->isAlive() || !to->isAlive())
                    return true;

                QList<ServerPlayer *> targets;
                foreach (ServerPlayer *vic, room->getOtherPlayers(to)) {
                    if (to->canSlash(vic))
                        targets << vic;
                }
                ServerPlayer *victim = NULL;
                if (!targets.isEmpty()) {
                    victim = room->askForPlayerChosen(fazheng, targets, "xuanhuo_slash", "@dummy-slash2:" + to->objectName());

                    LogMessage log;
                    log.type = "#CollateralSlash";
                    log.from = fazheng;
                    log.to << victim;
                    room->sendLog(log);
                }

                if (victim == NULL || !room->askForUseSlashTo(to, victim, QString("xuanhuo-slash:%1:%2").arg(fazheng->objectName()).arg(victim->objectName()))) {
                    if (to->isNude())
                        return true;
                    room->setPlayerFlag(to, "xuanhuo_InTempMoving");
                    int first_id = room->askForCardChosen(fazheng, to, "he", "xuanhuo");
                    Player::Place original_place = room->getCardPlace(first_id);
                    DummyCard *dummy = new DummyCard;
                    dummy->addSubcard(first_id);
                    to->addToPile("#xuanhuo", dummy, false);
                    if (!to->isNude()) {
                        int second_id = room->askForCardChosen(fazheng, to, "he", "xuanhuo");
                        dummy->addSubcard(second_id);
                    }

                    //move the first card back temporarily
                    room->moveCardTo(Sanguosha->getCard(first_id), to, original_place, false);
                    room->setPlayerFlag(to, "-xuanhuo_InTempMoving");
                    room->moveCardTo(dummy, fazheng, Player::PlaceHand, false);
                    delete dummy;
                }

                return true;
            }
        }

        return false;
    }
};

class Huilei: public TriggerSkill {
public:
    Huilei():TriggerSkill("huilei") {
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;
        ServerPlayer *killer = death.damage ? death.damage->from : NULL;
        if (killer) {
            LogMessage log;
            log.type = "#HuileiThrow";
            log.from = player;
            log.to << killer;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            QString killer_name = killer->getGeneralName();
            if (killer_name.contains("zhugeliang") || killer_name == "wolong")
                room->broadcastSkillInvoke(objectName(), 1);
            else
                room->broadcastSkillInvoke(objectName(), 2);

            killer->throwAllHandCardsAndEquips();
        }

        return false;
    }
};

XuanfengCard::XuanfengCard(){
}

bool XuanfengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return Self->canDiscard(to_select, "he");
}

void XuanfengCard::use(Room *room, ServerPlayer *lingtong, QList<ServerPlayer *> &targets) const{
    QMap<ServerPlayer*,int> map;
    int totaltarget = 0;
    foreach(ServerPlayer* sp, targets)
        map[sp]++;
    for (int i = 0; i < map.keys().size(); i++) {
        totaltarget++;
    }
    // only chose one and throw only one card of him is forbiden
    if(totaltarget == 1){
        foreach(ServerPlayer* sp,map.keys()){
            map[sp]++;
        }
    }
    foreach(ServerPlayer* sp,map.keys()){
        while(map[sp] > 0){
            if(lingtong->isAlive() && sp->isAlive() && lingtong->canDiscard(sp, "he")){
                int card_id = room->askForCardChosen(lingtong, sp, "he", "xuanfeng", false, Card::MethodDiscard);
                room->throwCard(card_id, sp, lingtong);
            }
            map[sp]--;
        }
    }
}

class XuanfengViewAsSkill: public ZeroCardViewAsSkill{
public:
    XuanfengViewAsSkill():ZeroCardViewAsSkill("xuanfeng"){
    }

    virtual const Card *viewAs() const{
        return new XuanfengCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@xuanfeng";
    }
};

class Xuanfeng: public TriggerSkill {
public:
    Xuanfeng(): TriggerSkill("xuanfeng") {
        events << CardsMoveOneTime << EventPhaseStart;
        view_as_skill = new XuanfengViewAsSkill;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *lingtong, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            lingtong->setMark("xuanfeng", 0);
        } else if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != lingtong)
                return false;

            if (move.to_place == Player::DiscardPile && lingtong->getPhase() == Player::Discard
                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
                lingtong->setMark("xuanfeng", lingtong->getMark("xuanfeng") + move.card_ids.length());

            if ((lingtong->getMark("xuanfeng") >= 2 && !lingtong->hasFlag("XuanfengUsed"))
                || move.from_places.contains(Player::PlaceEquip)) {
                QList<ServerPlayer *> targets;
                foreach (ServerPlayer *target, room->getOtherPlayers(lingtong)) {
                    if (lingtong->canDiscard(target, "he"))
                        targets << target;
                }
                if (targets.isEmpty())
                    return false;
                QString choice = room->askForChoice(lingtong, objectName(), "throw+nothing");
                if (choice == "throw") {
                    lingtong->setFlags("XuanfengUsed");
                    room->askForUseCard(lingtong, "@@xuanfeng", "@xuanfeng-card");
                }

            }
        }

        return false;
    }
};

class Pojun: public TriggerSkill {
public:
    Pojun(): TriggerSkill("pojun") {
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash") && !damage.chain && !damage.transfer
            && damage.to->isAlive() && room->askForSkillInvoke(player, objectName(), data)) {
            int x = qMin(5, damage.to->getHp());
            room->broadcastSkillInvoke(objectName(), (x >= 3 || !damage.to->faceUp()) ? 2 : 1);
            damage.to->drawCards(x);
            damage.to->turnOver();
        }
        return false;
    }
};

XianzhenCard::XianzhenCard() {
}

bool XianzhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void XianzhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if (effect.from->pindian(effect.to, "xianzhen", NULL)) {
        PlayerStar target = effect.to;
        effect.from->tag["XianzhenTarget"] = QVariant::fromValue(target);
        room->setPlayerFlag(effect.from, "XianzhenSuccess");
        room->setFixedDistance(effect.from, effect.to, 1);
        room->addPlayerMark(effect.to, "Armor_Nullified");
    } else {
        room->setPlayerCardLimitation(effect.from, "use", "Slash", true);
    }
}

XianzhenSlashCard::XianzhenSlashCard() {
    target_fixed = true;
    handling_method = Card::MethodUse;
}

void XianzhenSlashCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *target = card_use.from->tag["XianzhenTarget"].value<PlayerStar>();
    if (target == NULL || target->isDead())
        return;

    if (!card_use.from->canSlash(target, NULL, false))
        return;

    room->askForUseSlashTo(card_use.from, target, "@xianzhen-slash");
}

class XianzhenViewAsSkill: public ZeroCardViewAsSkill {
public:
    XianzhenViewAsSkill(): ZeroCardViewAsSkill("xianzhen") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (!player->hasUsed("XianzhenCard") && !player->isKongcheng())
            return true;
        Slash *slashx = new Slash(Card::NoSuit, 0);
        slashx->deleteLater();
        if (!player->isCardLimited(slashx, Card::MethodUse) && player->hasFlag("XianzhenSuccess"))
            return true;

        return false;
    }

    virtual const Card *viewAs() const{
        if (!Self->hasUsed("XianzhenCard"))
            return new XianzhenCard;
        else if (Self->hasFlag("XianzhenSuccess"))
            return new XianzhenSlashCard;
        else
            return NULL;
    }
};

class Xianzhen: public TriggerSkill {
public:
    Xianzhen(): TriggerSkill("xianzhen") {
        events << EventPhaseChanging << Death << EventLoseSkill;
        view_as_skill = new XianzhenViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->tag["XianzhenTarget"].value<PlayerStar>() != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *gaoshun, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else if (triggerEvent == EventLoseSkill) {
            if (data.toString() != "xianzhen")
                return false;
        }
        ServerPlayer *target = gaoshun->tag["XianzhenTarget"].value<PlayerStar>();
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != gaoshun) {
                if (death.who == target) {
                    room->setFixedDistance(gaoshun, target, -1);
                    gaoshun->tag.remove("XianzhenTarget");
                    room->setPlayerFlag(gaoshun, "-XianzhenSuccess");
                }
                return false;
            }
        }
        if (target) {
            room->setFixedDistance(gaoshun, target, -1);
            gaoshun->tag.remove("XianzhenTarget");
            room->removePlayerMark(target, "Armor_Nullified");
        }
        return false;
    }
};

class Jinjiu: public FilterSkill {
public:
    Jinjiu(): FilterSkill("jinjiu") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->objectName() == "analeptic";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->setSkillName(objectName());
        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
        card->takeOver(slash);
        return card;
    }
};

MingceCard::MingceCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void MingceCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    QList <ServerPlayer *> targets;
    if (Slash::IsAvailable(effect.to)) {
        foreach (ServerPlayer *p, room->getOtherPlayers(effect.to)) {
            if (effect.to->canSlash(p))
                targets << p;
        }
    }

    ServerPlayer *target;
    QStringList choicelist;
    choicelist << "draw";
    if (!targets.isEmpty() && effect.from->isAlive()) {
        target = room->askForPlayerChosen(effect.from, targets, "mingce", "@dummy-slash2:" + effect.to->objectName());
        target->setFlags("MingceTarget"); // For AI

        LogMessage log;
        log.type = "#CollateralSlash";
        log.from = effect.from;
        log.to << target;
        room->sendLog(log);

        choicelist << "use";
    }

    try {
        effect.to->obtainCard(this);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange)
            if (target && target->hasFlag("MingceTarget")) target->setFlags("-MingceTarget");
        throw triggerEvent;
    }

    QString choice = room->askForChoice(effect.to, "mingce", choicelist.join("+"));
    if (target && target->hasFlag("MingceTarget")) target->setFlags("-MingceTarget");

    if (choice == "use") {
        if (effect.to->canSlash(target, NULL, false)) {
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName("_mingce");
            room->useCard(CardUseStruct(slash, effect.to, target), false);
        }
    } else if (choice == "draw") {
        effect.to->drawCards(1);
    }
}

class Mingce: public OneCardViewAsSkill {
public:
    Mingce(): OneCardViewAsSkill("mingce") {
        default_choice = "draw";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MingceCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("EquipCard") || to_select->isKindOf("Slash");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        MingceCard *mingceCard = new MingceCard;
        mingceCard->addSubcard(originalCard);

        return mingceCard;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (card->isKindOf("Slash"))
            return -2;
        else
            return -1;
    }
};

class Zhichi: public TriggerSkill {
public:
    Zhichi(): TriggerSkill("zhichi") {
        events << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() != Player::NotActive)
            return false;

        ServerPlayer *current = room->getCurrent();
        if (current && current->isAlive() && current->getPhase() != Player::NotActive) {
            room->broadcastSkillInvoke(objectName(), 1);
            room->notifySkillInvoked(player, objectName());
            if (player->getMark("@late") == 0)
                room->addPlayerMark(player, "@late");

            LogMessage log;
            log.type = "#ZhichiDamaged";
            log.from = player;
            room->sendLog(log);
        }

        return false;
    }
};

class ZhichiProtect: public TriggerSkill {
public:
    ZhichiProtect(): TriggerSkill("#zhichi-protect") {
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if ((effect.card->isKindOf("Slash") || effect.card->isNDTrick()) && effect.to->getMark("@late") > 0) {
            room->broadcastSkillInvoke("zhichi", 2);
            room->notifySkillInvoked(effect.to, "zhichi");
            LogMessage log;
            log.type = "#ZhichiAvoid";
            log.from = effect.to;
            log.arg = "zhichi";
            room->sendLog(log);

            return true;
        }
        return false;
    }
};

class ZhichiClear: public TriggerSkill {
public:
    ZhichiClear(): TriggerSkill("#zhichi-clear") {
        events << EventPhaseChanging << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player || player != room->getCurrent())
                return false;
        }

        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->getMark("@late") > 0)
                room->setPlayerMark(p, "@late", 0);
        }

        return false;
    }
};

GanluCard::GanluCard() {
}

void GanluCard::swapEquip(ServerPlayer *first, ServerPlayer *second) const{
    Room *room = first->getRoom();

    QList<int> equips1, equips2;
    foreach (const Card *equip, first->getEquips())
        equips1.append(equip->getId());
    foreach (const Card *equip, second->getEquips())
        equips2.append(equip->getId());

    QList<CardsMoveStruct> exchangeMove;
    CardsMoveStruct move1;
    move1.card_ids = equips1;
    move1.to = second;
    move1.to_place = Player::PlaceEquip;
    CardsMoveStruct move2;
    move2.card_ids = equips2;
    move2.to = first;
    move2.to_place = Player::PlaceEquip;
    exchangeMove.push_back(move2);
    exchangeMove.push_back(move1);
    room->moveCards(exchangeMove, false);
}

bool GanluCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool GanluCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    switch (targets.length()) {
    case 0: return true;
    case 1: {
            int n1 = targets.first()->getEquips().length();
            int n2 = to_select->getEquips().length();
            return qAbs(n1 - n2) <= Self->getLostHp();
        }
    default:
        return false;
    }
}

void GanluCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    LogMessage log;
    log.type = "#GanluSwap";
    log.from = source;
    log.to = targets;
    room->sendLog(log);

    swapEquip(targets.first(), targets[1]);
}

class Ganlu: public ZeroCardViewAsSkill {
public:
    Ganlu(): ZeroCardViewAsSkill("ganlu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("GanluCard");
    }

    virtual const Card *viewAs() const{
        return new GanluCard;
    }
};

class Buyi: public TriggerSkill {
public:
    Buyi(): TriggerSkill("buyi") {
        events << Dying;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *wuguotai, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        ServerPlayer *player = dying.who;
        if (player->isKongcheng()) return false;
        if (player->getHp() < 1 && wuguotai->askForSkillInvoke(objectName(), data)) {
            const Card *card = NULL;
            if (player == wuguotai)
                card = room->askForCardShow(player, wuguotai, objectName());
            else {
                int card_id = room->askForCardChosen(wuguotai, player, "h", "buyi");
                card = Sanguosha->getCard(card_id);
            }

            room->showCard(player, card->getEffectiveId());

            if (card->getTypeId() != Card::TypeBasic) {
                if (!player->isJilei(card))
                    room->throwCard(card, player);

                room->broadcastSkillInvoke(objectName());

                RecoverStruct recover;
                recover.who = wuguotai;
                room->recover(player, recover);
            }
        }
        return false;
    }
};

XinzhanCard::XinzhanCard() {
    target_fixed = true;
}

void XinzhanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(3), left;

    LogMessage log;
    log.type = "$ViewDrawPile";
    log.from = source;
    log.card_str = IntList2StringList(cards).join("+");
    room->doNotify(source, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

    left = cards;

    QList<int> hearts, non_hearts;
    foreach (int card_id, cards) {
        const Card *card = Sanguosha->getCard(card_id);
        if (card->getSuit() == Card::Heart)
            hearts << card_id;
        else
            non_hearts << card_id;
    }
    DummyCard *dummy = new DummyCard;

    if (!hearts.isEmpty()) {
        do {
            room->fillAG(left, source, non_hearts);
            int card_id = room->askForAG(source, hearts, true, "xinzhan");
            if (card_id == -1) {
                room->clearAG(source);
                break;
            }

            hearts.removeOne(card_id);
            left.removeOne(card_id);

            dummy->addSubcard(card_id);
            room->clearAG(source);
        } while (!hearts.isEmpty());

        if (dummy->subcardsLength() > 0) {
            room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, Json::Value(room->getDrawPile().length() + dummy->subcardsLength()));
            source->obtainCard(dummy);
            foreach (int id, dummy->getSubcards())
                room->showCard(source, id);
        }
        dummy->deleteLater();
    }

    if (!left.isEmpty())
        room->askForGuanxing(source, left, true);
 }

class Xinzhan: public ZeroCardViewAsSkill {
public:
    Xinzhan(): ZeroCardViewAsSkill("xinzhan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XinzhanCard") && player->getHandcardNum() > player->getMaxHp();
    }

    virtual const Card *viewAs() const{
        return new XinzhanCard;
    }
};

class Quanji: public MasochismSkill {
public:
    Quanji(): MasochismSkill("#quanji") {
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *zhonghui, const DamageStruct &damage) const{
        Room *room = zhonghui->getRoom();

        int x = damage.damage;
        for (int i = 0; i < x; i++) {
            if (zhonghui->askForSkillInvoke("quanji")) {
                room->broadcastSkillInvoke("quanji");
                room->drawCards(zhonghui, 1);
                if (!zhonghui->isKongcheng()) {
                    int card_id;
                    if (zhonghui->getHandcardNum() == 1) {
                        room->getThread()->delay();
                        card_id = zhonghui->handCards().first();
                    } else
                        card_id = room->askForExchange(zhonghui, "quanji", 1, false, "QuanjiPush")->getSubcards().first();
                    zhonghui->addToPile("power", card_id);
                }
            }
        }

    }
};

class QuanjiKeep: public MaxCardsSkill {
public:
    QuanjiKeep(): MaxCardsSkill("quanji") {
        frequency = Frequent;
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasSkill(objectName()))
            return target->getPile("power").length();
        else
            return 0;
    }
};

class Zili: public PhaseChangeSkill {
public:
    Zili(): PhaseChangeSkill("zili") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Start
               && target->getMark("zili") == 0
               && target->getPile("power").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *zhonghui) const{
        Room *room = zhonghui->getRoom();
        room->notifySkillInvoked(zhonghui, objectName());

        LogMessage log;
        log.type = "#ZiliWake";
        log.from = zhonghui;
        log.arg = QString::number(zhonghui->getPile("power").length());
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$ZiliAnimate", 3000);

        room->addPlayerMark(zhonghui, "zili");
        if (room->changeMaxHpForAwakenSkill(zhonghui)) {
            if (zhonghui->isWounded() && room->askForChoice(zhonghui, objectName(), "recover+draw") == "recover") {
                RecoverStruct recover;
                recover.who = zhonghui;
                room->recover(zhonghui, recover);
            } else {
                room->drawCards(zhonghui, 2);
            }
            room->acquireSkill(zhonghui, "paiyi");
        }

        return false;
    }
};

PaiyiCard::PaiyiCard() {
    mute = true;
}

bool PaiyiCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.isEmpty();
}

void PaiyiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *zhonghui = card_use.from;
    ServerPlayer *target = card_use.to.first();
    QList<int> powers = zhonghui->getPile("power");
    if (powers.isEmpty()) return;

    LogMessage log;
    log.from = zhonghui;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    int card_id;
    if (powers.length() == 1)
        card_id = powers.first();
    else {
        room->fillAG(powers, zhonghui);
        card_id = room->askForAG(zhonghui, powers, false, "paiyi");
        room->clearAG(zhonghui);

        if (card_id == -1)
            return;
    }

    room->broadcastSkillInvoke("paiyi", target == zhonghui ? 1 : 2);

    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(),
                          target->objectName(), "paiyi", QString());
    room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
    room->drawCards(target, 2, "paiyi");
    if (target->getHandcardNum() > zhonghui->getHandcardNum())
        room->damage(DamageStruct("paiyi", zhonghui, target));
}

class Paiyi: public ZeroCardViewAsSkill {
public:
    Paiyi(): ZeroCardViewAsSkill("paiyi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("power").isEmpty() && !player->hasUsed("PaiyiCard");
    }

    virtual const Card *viewAs() const{
        return new PaiyiCard;
    }

    virtual Location getLocation() const{
        return Right;
    }
};

class Jueqing: public TriggerSkill {
public:
    Jueqing(): TriggerSkill("jueqing") {
        frequency = Compulsory;
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhangchunhua, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from == zhangchunhua) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = zhangchunhua;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(zhangchunhua, objectName());
            room->broadcastSkillInvoke(objectName());
            room->loseHp(damage.to, damage.damage);

            return true;
        }
        return false;
    }
};

Shangshi::Shangshi(): TriggerSkill("shangshi") {
    events << HpChanged << MaxHpChanged << CardsMoveOneTime << EventPhaseChanging;
    frequency = Frequent;
}

int Shangshi::getMaxLostHp(ServerPlayer *zhangchunhua) const{
    int losthp = zhangchunhua->getLostHp();
    if (losthp > 2)
        losthp = 2;
    return qMin(losthp, zhangchunhua->getMaxHp());
}

bool Shangshi::trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhangchunhua, QVariant &data) const{
    int losthp = getMaxLostHp(zhangchunhua);
    if (triggerEvent == CardsMoveOneTime) {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (zhangchunhua->getPhase() == Player::Discard) {
            bool changed = false;
            if (move.from == zhangchunhua && move.from_places.contains(Player::PlaceHand))
                changed = true;
            if (move.to == zhangchunhua && move.to_place == Player::PlaceHand)
                changed = true;
            if (changed)
                zhangchunhua->addMark("shangshi");
            return false;
        } else {
            bool can_invoke = false;
            if (move.from == zhangchunhua && move.from_places.contains(Player::PlaceHand))
                can_invoke = true;
            if (move.to == zhangchunhua && move.to_place == Player::PlaceHand)
                can_invoke = true;
            if (!can_invoke)
                return false;
        }
    } else if (triggerEvent == HpChanged || triggerEvent == MaxHpChanged) {
        if (zhangchunhua->getPhase() == Player::Discard) {
            zhangchunhua->addMark("shangshi");
            return false;
        }
    } else if (triggerEvent == EventPhaseChanging) {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.from != Player::Discard)
            return false;
        if (zhangchunhua->getMark("shangshi") <= 0)
            return false;
        zhangchunhua->setMark("shangshi", 0);
    }

    if (zhangchunhua->getHandcardNum()<losthp && zhangchunhua->getPhase() != Player::Discard
        && zhangchunhua->askForSkillInvoke(objectName())) {
        zhangchunhua->drawCards(losthp - zhangchunhua->getHandcardNum());
        room->broadcastSkillInvoke("shangshi");
    }

    return false;
}

YJCMPackage::YJCMPackage()
    : Package("YJCM")
{
    General *caozhi = new General(this, "caozhi", "wei", 3); // YJ 001
    caozhi->addSkill(new Luoying);
    caozhi->addSkill(new Jiushi);
    caozhi->addSkill(new JiushiFlip);
    related_skills.insertMulti("jiushi", "#jiushi-flip");

    General *chengong = new General(this, "chengong", "qun", 3); // YJ 002
    chengong->addSkill(new Zhichi);
    chengong->addSkill(new ZhichiProtect);
    chengong->addSkill(new ZhichiClear);
    chengong->addSkill(new Mingce);
    related_skills.insertMulti("zhichi", "#zhichi-protect");
    related_skills.insertMulti("zhichi", "#zhichi-clear");

    General *fazheng = new General(this, "fazheng", "shu", 3); // YJ 003
    fazheng->addSkill(new Enyuan);
    fazheng->addSkill(new Xuanhuo);
    fazheng->addSkill(new FakeMoveSkill("xuanhuo"));
    related_skills.insertMulti("xuanhuo", "#xuanhuo-fake-move");

    General *gaoshun = new General(this, "gaoshun", "qun"); // YJ 004
    gaoshun->addSkill(new Xianzhen);
    gaoshun->addSkill(new Jinjiu);

    General *lingtong = new General(this, "lingtong", "wu"); // YJ 005
    lingtong->addSkill(new Xuanfeng);

    General *masu = new General(this, "masu", "shu", 3); // YJ 006
    masu->addSkill(new Xinzhan);
    masu->addSkill(new Huilei);

    General *wuguotai = new General(this, "wuguotai", "wu", 3, false); // YJ 007
    wuguotai->addSkill(new Ganlu);
    wuguotai->addSkill(new Buyi);

    General *xusheng = new General(this, "xusheng", "wu"); // YJ 008
    xusheng->addSkill(new Pojun);

    General *xushu = new General(this, "xushu", "shu", 3); // YJ 009
    xushu->addSkill(new Wuyan);
    xushu->addSkill(new Jujian);

    General *yujin = new General(this, "yujin", "wei"); // YJ 010
    yujin->addSkill(new Yizhong);

    General *zhangchunhua = new General(this, "zhangchunhua", "wei", 3, false); // YJ 011
    zhangchunhua->addSkill(new Jueqing);
    zhangchunhua->addSkill(new Shangshi);

    General *zhonghui = new General(this, "zhonghui", "wei"); // YJ 012
    zhonghui->addSkill(new QuanjiKeep);
    zhonghui->addSkill(new Quanji);
    zhonghui->addSkill(new DetachEffectSkill("quanji", "power"));
    zhonghui->addSkill(new Zili);
    zhonghui->addRelateSkill("paiyi");
    related_skills.insertMulti("quanji", "#quanji");
    related_skills.insertMulti("quanji", "#quanji-clear");

    addMetaObject<MingceCard>();
    addMetaObject<GanluCard>();
    addMetaObject<XianzhenCard>();
    addMetaObject<XianzhenSlashCard>();
    addMetaObject<XinzhanCard>();
    addMetaObject<XuanfengCard>();
    addMetaObject<JujianCard>();
    addMetaObject<PaiyiCard>();

    skills << new Paiyi;
}

ADD_PACKAGE(YJCM)

