#include "special3v3-package.h"
#include "skill.h"
#include "standard.h"
#include "server.h"
#include "engine.h"
#include "ai.h"
#include "maneuvering.h"
#include "clientplayer.h"

HongyuanCard::HongyuanCard() {
    mute = true;
}

bool HongyuanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 2 && !targets.contains(Self);
}

bool HongyuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self && targets.length() < 2;
}

void HongyuanCard::onEffect(const CardEffectStruct &effect) const{
   effect.to->setFlags("HongyuanTarget");
}

class HongyuanViewAsSkill: public ZeroCardViewAsSkill {
public:
    HongyuanViewAsSkill(): ZeroCardViewAsSkill("hongyuan") {
        response_pattern = "@@hongyuan";
    }

    virtual const Card *viewAs() const{
        return new HongyuanCard;
    }
};

class Hongyuan: public DrawCardsSkill {
public:
    Hongyuan(): DrawCardsSkill("hongyuan") {
        frequency = NotFrequent;
        view_as_skill = new HongyuanViewAsSkill;
    }

    virtual int getDrawNum(ServerPlayer *zhugejin, int n) const{
        Room *room = zhugejin->getRoom();
        bool invoke = false;
        if (room->getMode().startsWith("06_"))
            invoke = room->askForSkillInvoke(zhugejin, objectName());
        else
            invoke = room->askForUseCard(zhugejin, "@@hongyuan", "@hongyuan");
        if (invoke) {
            room->broadcastSkillInvoke(objectName());
            zhugejin->setFlags("hongyuan");
            return n - 1;
        } else
            return n;
    }
};

class HongyuanDraw: public TriggerSkill {
public:
    HongyuanDraw(): TriggerSkill("#hongyuan") {
        events << AfterDrawNCards;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (!player->hasFlag("hongyuan"))
            return false;
        player->setFlags("-hongyuan");

        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (room->getMode().startsWith("06_")) {
                if (AI::GetRelation3v3(player, p) == AI::Friend)
                    targets << p;
            } else if (p->hasFlag("HongyuanTarget")) {
                p->setFlags("-HongyuanTarget");
                targets << p;
            }
        }

        if (targets.isEmpty()) return false;
        room->drawCards(targets, 1, "hongyuan");
        return false;
    }
};

class Huanshi: public TriggerSkill {
public:
    Huanshi(): TriggerSkill("huanshi") {
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isNude();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        const Card *card = NULL;
        if (room->getMode().startsWith("06_")) {
            if (AI::GetRelation3v3(player, judge->who) != AI::Friend) return false;
            QStringList prompt_list;
            prompt_list << "@huanshi-card" << judge->who->objectName()
                        << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
            QString prompt = prompt_list.join(":");

            card = room->askForCard(player, "..", prompt, data, Card::MethodResponse, judge->who, true);
        } else if (!player->isNude()) {
            QList<int> ids, disabled_ids;
            foreach (const Card *card, player->getCards("he")) {
                if (player->isCardLimited(card, Card::MethodResponse))
                    disabled_ids << card->getEffectiveId();
                else
                    ids << card->getEffectiveId();
            }
            if (!ids.isEmpty() && room->askForSkillInvoke(player, objectName(), data)) {
                if (judge->who != player && !player->isKongcheng()) {
                    LogMessage log;
                    log.type = "$ViewAllCards";
                    log.from = judge->who;
                    log.to << player;
                    log.card_str = IntList2StringList(player->handCards()).join("+");
                    room->doNotify(judge->who, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
                }
                judge->who->tag["HuanshiJudge"] = data;
                room->fillAG(ids + disabled_ids, judge->who, disabled_ids);
                int card_id = room->askForAG(judge->who, ids, false, objectName());
                room->clearAG(judge->who);
                judge->who->tag.remove("HuanshiJudge");
                card = Sanguosha->getCard(card_id);
            }
        }
        if (card != NULL) {
            room->broadcastSkillInvoke(objectName());
            room->retrial(card, player, judge, objectName());
        }

        return false;
    }
};

class Mingzhe: public TriggerSkill {
public:
    Mingzhe(): TriggerSkill("mingzhe") {
        events << BeforeCardsMove << CardsMoveOneTime << CardUsed << CardResponded;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->getPhase() != Player::NotActive) return false;
        if (triggerEvent == BeforeCardsMove || triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != player) return false;

            if (triggerEvent == BeforeCardsMove) {
                CardMoveReason reason = move.reason;
                if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                    const Card *card;
                    int i = 0;
                    foreach (int card_id, move.card_ids) {
                        card = Sanguosha->getCard(card_id);
                        if (room->getCardOwner(card_id) == player && card->isRed()
                            && (move.from_places[i] == Player::PlaceHand
                                || move.from_places[i] == Player::PlaceEquip)) {
                            player->addMark(objectName());
                        }
                        i++;
                    }
                }
            } else {
                int n = player->getMark(objectName());
                try {
                    for (int i = 0; i < n; i++) {
                        player->removeMark(objectName());
                        if (player->isAlive() && player->askForSkillInvoke(objectName(), data)) {
                            room->broadcastSkillInvoke(objectName());
                            player->drawCards(1);
                        } else {
                            break;
                        }
                    }
                    player->setMark(objectName(), 0);
                }
                catch (TriggerEvent triggerEvent) {
                    if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                        player->setMark(objectName(), 0);
                    throw triggerEvent;
                }
            }
        } else {
            CardStar card = NULL;
            if (triggerEvent == CardUsed) {
                CardUseStruct use = data.value<CardUseStruct>();
                card = use.card;
            } else if (triggerEvent == CardResponded) {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                card = resp.m_card;
            }
            if (card && card->isRed() && player->askForSkillInvoke(objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                player->drawCards(1);
            }
        }
        return false;
    }
};

class VsGanglie: public MasochismSkill {
public:
    VsGanglie(): MasochismSkill("vsganglie") {
    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &) const{
        Room *room = xiahou->getRoom();
        QString mode = room->getMode();
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(xiahou)) {
            if (!mode.startsWith("06_") || AI::GetRelation3v3(xiahou, p) == AI::Enemy)
                targets << p;
        }

        ServerPlayer *from = room->askForPlayerChosen(xiahou, targets, objectName(), "vsganglie-invoke", true, true);
        if (!from) return;

        room->broadcastSkillInvoke("ganglie");

        JudgeStruct judge;
        judge.pattern = ".|heart";
        judge.good = false;
        judge.reason = objectName();
        judge.who = xiahou;

        room->judge(judge);
        if (from->isDead()) return;
        if (judge.isGood()) {
            if (from->getHandcardNum() < 2 || !room->askForDiscard(from, objectName(), 2, 2, true))
                room->damage(DamageStruct(objectName(), xiahou, from));
        }
    }
};

ZhongyiCard::ZhongyiCard() {
    mute = true;
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
}

void ZhongyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->broadcastSkillInvoke("zhongyi");
    room->doLightbox("$ZhongyiAnimate");
    room->removePlayerMark(source, "@loyal");
    source->addToPile("loyal", this);
}

class Zhongyi: public OneCardViewAsSkill {
public:
    Zhongyi(): OneCardViewAsSkill("zhongyi") {
        frequency = Limited;
        limit_mark = "@loyal";
        filter_pattern = ".|red|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && player->getMark("@loyal") > 0;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ZhongyiCard *card = new ZhongyiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class ZhongyiAction: public TriggerSkill {
public:
    ZhongyiAction(): TriggerSkill("#zhongyi-action") {
        events << DamageCaused << EventPhaseStart << ActionedReset;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QString mode = room->getMode();
        if (triggerEvent == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.chain || damage.transfer || !damage.by_user) return false;
            if (damage.card && damage.card->isKindOf("Slash")) {
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->getPile("loyal").isEmpty()) continue;
                    bool on_effect = false;
                    if (room->getMode().startsWith("06_"))
                        on_effect = (AI::GetRelation3v3(player, p) == AI::Friend);
                    else
                        on_effect = (room->askForSkillInvoke(p, "zhongyi", data));
                    if (on_effect) {
                        LogMessage log;
                        log.type = "#ZhongyiBuff";
                        log.from = p;
                        log.to << damage.to;
                        log.arg = QString::number(damage.damage);
                        log.arg2 = QString::number(++damage.damage);
                        room->sendLog(log);
                    }
                }
            }
            data = QVariant::fromValue(damage);
        } else if ((mode == "06_3v3" && triggerEvent == ActionedReset) || (mode != "06_3v3" && triggerEvent == EventPhaseStart)) {
            if (triggerEvent == EventPhaseStart && player->getPhase() != Player::RoundStart)
                return false;
            if (player->getPile("loyal").length() > 0)
                player->clearOnePrivatePile("loyal");
        }
        return false;
    }
};

JiuzhuCard::JiuzhuCard() {
    target_fixed = true;
}

void JiuzhuCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &) const{
    ServerPlayer *who = room->getCurrentDyingPlayer();
    if (!who) return;

    room->loseHp(player);
    RecoverStruct recover;
    recover.who = player;
    room->recover(who, recover);
}

class Jiuzhu: public OneCardViewAsSkill {
public:
    Jiuzhu(): OneCardViewAsSkill("jiuzhu") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (pattern != "peach" || !player->canDiscard(player, "he") || player->getHp() <= 1) return false;
        QString dyingobj = player->property("currentdying").toString();
        const Player *who = NULL;
        foreach (const Player *p, player->getAliveSiblings()) {
            if (p->objectName() == dyingobj) {
                who = p;
                break;
            }
        }
        if (!who) return false;
        if (ServerInfo.GameMode.startsWith("06_"))
            return player->getRole().at(0) == who->getRole().at(0);
        else
            return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        JiuzhuCard *card = new JiuzhuCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Zhanshen: public TriggerSkill {
public:
    Zhanshen(): TriggerSkill("zhanshen") {
        events << Death << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
            foreach (ServerPlayer *lvbu, room->findPlayersBySkillName(objectName())) {
                if (room->getMode().startsWith("06_")) {
                    if (lvbu->getMark(objectName()) == 0 && lvbu->getMark("zhanshen_fight") == 0
                        && AI::GetRelation3v3(lvbu, player) == AI::Friend)
                        lvbu->addMark("zhanshen_fight");
                } else {
                    if (lvbu->getMark(objectName()) == 0 && lvbu->getMark("@fight") == 0
                        && room->askForSkillInvoke(player, objectName(), "mark:" + lvbu->objectName()))
                        room->addPlayerMark(lvbu, "@fight");
                }
            }
        } else if (TriggerSkill::triggerable(player)
                   && player->getPhase() == Player::Start
                   && player->getMark(objectName()) == 0
                   && player->isWounded()
                   && (player->getMark("zhanshen_fight") > 0 || player->getMark("@fight") > 0)) {
            room->notifySkillInvoked(player, objectName());

            LogMessage log;
            log.type = "#ZhanshenWake";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$ZhanshenAnimate");

            if (player->getMark("@fight") > 0)
                room->setPlayerMark(player, "@fight", 0);
            player->setMark("zhanshen_fight", 0);
            room->addPlayerMark(player, objectName());
            if (room->changeMaxHpForAwakenSkill(player)) {
                if (player->getWeapon())
                    room->throwCard(player->getWeapon(), player);
                room->handleAcquireDetachSkills(player, "mashu|shenji");
            }
        }
        return false;
    }
};

class ZhenweiDistance: public DistanceSkill {
public:
    ZhenweiDistance(): DistanceSkill("#zhenwei") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (to->hasSkill("zhenwei")) {
            return 0;
        } else {
            bool hasWenpin = false;
            foreach (const Player *p, to->getAliveSiblings()) {
                if (p->hasSkill("zhenwei")) {
                    hasWenpin = true;
                    break;
                }
            }
            if (!hasWenpin) return 0;
        }
        if (ServerInfo.GameMode.startsWith("06_")) {
            if (from->getRole().at(0) != to->getRole().at(0)) {
                foreach (const Player *p, to->getAliveSiblings()) {
                    if (p->hasSkill(objectName()) && p->getRole().at(0) == to->getRole().at(0))
                        return 1;
                }
            }
        } else if (to->getMark("@defense") > 0 && from->getMark("@defense") == 0 && !from->hasSkill("zhenwei")) {
            return 1;
        }
        return 0;
    }
};

ZhenweiCard::ZhenweiCard() {
}

bool ZhenweiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total = Self->getSiblings().length() + 1;
    return targets.length() < total / 2 - 1 && to_select != Self;
}

void ZhenweiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@defense");
}

class ZhenweiViewAsSkill: public ZeroCardViewAsSkill {
public:
    ZhenweiViewAsSkill(): ZeroCardViewAsSkill("zhenwei") {
        response_pattern = "@@zhenwei";
    }

    virtual const Card *viewAs() const{
        return new ZhenweiCard;
    }
};

class Zhenwei: public TriggerSkill {
public:
    Zhenwei(): TriggerSkill("zhenwei") {
        events << EventPhaseChanging << Death << EventLoseSkill;
        view_as_skill = new ZhenweiViewAsSkill;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->getRoom()->getMode().startsWith("06_");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventLoseSkill) {
            if (data.toString() != objectName())
                return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player || !player->hasSkill(objectName()))
                return false;
        } else if (triggerEvent == EventPhaseChanging) {
            if (!TriggerSkill::triggerable(player))
                return false;
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        }
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
            room->setPlayerMark(p, "@defense", 0);
        if (triggerEvent == EventPhaseChanging && Sanguosha->getPlayerCount(room->getMode()) > 3)
            room->askForUseCard(player, "@@zhenwei", "@zhenwei");
        return false;
    }
};

VSCrossbow::VSCrossbow(Suit suit, int number)
    : Crossbow(suit, number)
{
    setObjectName("VSCrossbow");
}

bool VSCrossbow::match(const QString &pattern) const{
    QStringList patterns = pattern.split("+");
    if (patterns.contains("crossbow"))
        return true;
    else
        return Crossbow::match(pattern);
}

New3v3CardPackage::New3v3CardPackage()
    : Package("New3v3Card")
{
    QList<Card *> cards;
    cards << new SupplyShortage(Card::Spade, 1)
          << new SupplyShortage(Card::Club, 12)
          << new Nullification(Card::Heart, 12);

    foreach (Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(New3v3Card)

New3v3_2013CardPackage::New3v3_2013CardPackage()
    : Package("New3v3_2013Card")
{
    QList<Card *> cards;
    cards << new VSCrossbow(Card::Club)
          << new VSCrossbow(Card::Diamond);

    foreach (Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(New3v3_2013Card)

Special3v3Package::Special3v3Package()
    : Package("Special3v3")
{
    General *vs_xiahoudun = new General(this, "vs_xiahoudun", "wei");
    vs_xiahoudun->addSkill(new VsGanglie);

    General *vs_guanyu = new General(this, "vs_guanyu", "shu");
    vs_guanyu->addSkill("wusheng");
    vs_guanyu->addSkill(new Zhongyi);
    vs_guanyu->addSkill(new ZhongyiAction);
    related_skills.insertMulti("zhongyi", "#zhongyi-action");

    General *vs_zhaoyun = new General(this, "vs_zhaoyun", "shu");
    vs_zhaoyun->addSkill("longdan");
    vs_zhaoyun->addSkill(new Jiuzhu);

    General *vs_lvbu = new General(this, "vs_lvbu", "qun");
    vs_lvbu->addSkill("wushuang");
    vs_lvbu->addSkill(new Zhanshen);

    General *wenpin = new General(this, "wenpin", "wei"); // WEI 019
    wenpin->addSkill(new Zhenwei);
    wenpin->addSkill(new ZhenweiDistance);
    related_skills.insert("zhenwei", "#zhenwei");

    General *zhugejin = new General(this, "zhugejin", "wu", 3); // WU 018
    zhugejin->addSkill(new Hongyuan);
    zhugejin->addSkill(new HongyuanDraw);
    zhugejin->addSkill(new Huanshi);
    zhugejin->addSkill(new Mingzhe);
    related_skills.insertMulti("hongyuan", "#hongyuan");

    addMetaObject<ZhongyiCard>();
    addMetaObject<JiuzhuCard>();
    addMetaObject<ZhenweiCard>();
    addMetaObject<HongyuanCard>();
}

ADD_PACKAGE(Special3v3)

