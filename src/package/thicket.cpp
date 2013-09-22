#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "general.h"

class Xingshang: public TriggerSkill {
public:
    Xingshang(): TriggerSkill("xingshang") {
        events << Death;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *caopi, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        ServerPlayer *player = death.who;
        if (player->isNude() || caopi == player)
            return false;
        if (caopi->isAlive() && room->askForSkillInvoke(caopi, objectName(), data)) {
            bool isCaoCao = player->getGeneralName().contains("caocao");
            room->broadcastSkillInvoke(objectName(), (isCaoCao ? 3 : (player->isMale() ? 1 : 2)));

            DummyCard *dummy = new DummyCard(player->handCards());
            QList <const Card *> equips = player->getEquips();
            foreach (const Card *card, equips)
                dummy->addSubcard(card);

            if (dummy->subcardsLength() > 0) {
                CardMoveReason reason(CardMoveReason::S_REASON_RECYCLE, caopi->objectName());
                room->obtainCard(caopi, dummy, reason, false);
            }
            delete dummy;
        }

        return false;
    }
};

class Fangzhu: public MasochismSkill {
public:
    Fangzhu(): MasochismSkill("fangzhu") {
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &) const{
        Room *room = caopi->getRoom();
        ServerPlayer *to = room->askForPlayerChosen(caopi, room->getOtherPlayers(caopi), objectName(),
                                                    "fangzhu-invoke", caopi->getMark("JilveEvent") != int(Damaged), true);
        if (to) {           
            if (caopi->hasInnateSkill("fangzhu") || !caopi->hasSkill("jilve"))
                room->broadcastSkillInvoke("fangzhu", to->faceUp() ? 1 : 2); // @todo_P: audios
            else
                room->broadcastSkillInvoke("jilve", 2);

            to->drawCards(caopi->getLostHp());
            to->turnOver();
        }
    }
};

class Songwei: public TriggerSkill {
public:
    Songwei(): TriggerSkill("songwei$") {
        events << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getKingdom() == "wei";
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        if (card->isBlack()) {
            QList<ServerPlayer *> caopis;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->hasLordSkill(objectName()))
                    caopis << p;
            }
            
            while (!caopis.isEmpty()) {
                ServerPlayer *caopi = room->askForPlayerChosen(player, caopis, objectName(), "@songwei-to", true);
                if (caopi) {
                    room->broadcastSkillInvoke(objectName());
                    room->notifySkillInvoked(caopi, objectName());
                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = player;
                    log.to << caopi;
                    log.arg = objectName();
                    room->sendLog(log);

                    caopi->drawCards(1);
                    caopis.removeOne(caopi);
                } else
                    break;
            }
        }

        return false;
    }
};

class Duanliang: public OneCardViewAsSkill {
public:
    Duanliang(): OneCardViewAsSkill("duanliang") {
        filter_pattern = "BasicCard,EquipCard|black";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        SupplyShortage *shortage = new SupplyShortage(originalCard->getSuit(), originalCard->getNumber());
        shortage->setSkillName(objectName());
        shortage->addSubcard(originalCard);

        return shortage;
    }
};

class DuanliangTargetMod: public TargetModSkill {
public:
    DuanliangTargetMod(): TargetModSkill("#duanliang-target") {
        frequency = NotFrequent;
        pattern = "SupplyShortage";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill("duanliang"))
            return 1;
        else
            return 0;
    }
};

class SavageAssaultAvoid: public TriggerSkill {
public:
    SavageAssaultAvoid(const QString &avoid_skill)
        : TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill)
    {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
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

private:
    QString avoid_skill;
};

class Huoshou: public TriggerSkill {
public:
    Huoshou(): TriggerSkill("huoshou") {
        events << TargetConfirmed << ConfirmDamage << CardFinished;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhurong, QVariant &data) const{
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
                else
                    room->broadcastSkillInvoke(objectName(), 3);

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

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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

class Yinghun: public PhaseChangeSkill {
public:
    Yinghun(): PhaseChangeSkill("yinghun") {
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
               && target->getPhase() == Player::Start
               && target->isWounded();
    }

    virtual bool onPhaseChange(ServerPlayer *sunjian) const{
        Room *room = sunjian->getRoom();
        ServerPlayer *to = room->askForPlayerChosen(sunjian, room->getOtherPlayers(sunjian), objectName(), "yinghun-invoke", true, true);
        if (to) {
            int x = sunjian->getLostHp();

            int index = 1;
            if (!sunjian->hasInnateSkill("yinghun") && sunjian->hasSkill("hunzi"))
                index += 2;

            if (x == 1) {
                room->broadcastSkillInvoke(objectName(), index);

                to->drawCards(1);
                room->askForDiscard(to, objectName(), 1, 1, false, true);
            } else {
                to->setFlags("YinghunTarget");
                QString choice = room->askForChoice(sunjian, objectName(), "d1tx+dxt1");
                to->setFlags("-YinghunTarget");
                if (choice == "d1tx") {
                    room->broadcastSkillInvoke(objectName(), index + 1);

                    to->drawCards(1);
                    room->askForDiscard(to, objectName(), x, x, false, true);
                } else {
                    room->broadcastSkillInvoke(objectName(), index);

                    to->drawCards(x);
                    room->askForDiscard(to, objectName(), 1, 1, false, true);
                }
            }
        }
        return false;
    }
};

HaoshiCard::HaoshiCard() {
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
    m_skillName = "_haoshi";
}

bool HaoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    return to_select->getHandcardNum() == Self->getMark("haoshi");
}

void HaoshiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(),
                          targets.first()->objectName(), "haoshi", QString());
    room->moveCardTo(this, targets.first(), Player::PlaceHand, reason);
}

class HaoshiViewAsSkill: public ViewAsSkill {
public:
    HaoshiViewAsSkill(): ViewAsSkill("haoshi") {
        response_pattern = "@@haoshi!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        HaoshiCard *card = new HaoshiCard;
        card->addSubcards(cards);
        return card;
    }
};

class HaoshiGive: public TriggerSkill {
public:
    HaoshiGive(): TriggerSkill("#haoshi-give") {
        events << AfterDrawNCards;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *lusu, QVariant &) const{
        if (lusu->hasFlag("haoshi")) {
            lusu->setFlags("-haoshi");

            if (lusu->getHandcardNum() <= 5)
                return false;            

            QList<ServerPlayer *> other_players = room->getOtherPlayers(lusu);
            int least = 1000;
            foreach (ServerPlayer *player, other_players)
                least = qMin(player->getHandcardNum(), least);
            room->setPlayerMark(lusu, "haoshi", least);
            bool used = room->askForUseCard(lusu, "@@haoshi!", "@haoshi", -1, Card::MethodNone);

            if (!used) {
                // force lusu to give his half cards
                ServerPlayer *beggar = NULL;
                foreach (ServerPlayer *player, other_players) {
                    if (player->getHandcardNum() == least) {
                        beggar = player;
                        break;
                    }
                }

                int n = lusu->getHandcardNum() / 2;
                QList<int> to_give = lusu->handCards().mid(0, n);
                HaoshiCard *haoshi_card = new HaoshiCard;
                haoshi_card->addSubcards(to_give);
                QList<ServerPlayer *> targets;
                targets << beggar;
                haoshi_card->use(room, lusu, targets);
                delete haoshi_card;
            }
        }

        return false;
    }
};

class Haoshi: public DrawCardsSkill {
public:
    Haoshi(): DrawCardsSkill("#haoshi") {
    }

    virtual int getDrawNum(ServerPlayer *lusu, int n) const{
        Room *room = lusu->getRoom();
        if (room->askForSkillInvoke(lusu, "haoshi")) {
            room->broadcastSkillInvoke("haoshi");
            lusu->setFlags("haoshi");
            return n + 2;
        } else
            return n;
    }
};

DimengCard::DimengCard() {
}

bool DimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self)
        return false;

    if (targets.isEmpty())
        return true;

    if (targets.length() == 1) {
        return qAbs(to_select->getHandcardNum() - targets.first()->getHandcardNum()) == subcardsLength();
    }

    return false;
}

bool DimengCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

#include "jsonutils.h"
void DimengCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);
    a->setFlags("DimengTarget");
    b->setFlags("DimengTarget");

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    try {
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p != a && p != b)
                room->doNotify(p, QSanProtocol::S_COMMAND_EXCHANGE_KNOWN_CARDS,
                               QSanProtocol::Utils::toJsonArray(a->objectName(), b->objectName()));
        }
        QList<CardsMoveStruct> exchangeMove;
        CardsMoveStruct move1(a->handCards(), b, Player::PlaceHand,
                              CardMoveReason(CardMoveReason::S_REASON_SWAP, a->objectName(), b->objectName(), "dimeng", QString()));
        CardsMoveStruct move2(b->handCards(), a, Player::PlaceHand,
                              CardMoveReason(CardMoveReason::S_REASON_SWAP, b->objectName(), a->objectName(), "dimeng", QString()));
        exchangeMove.push_back(move1);
        exchangeMove.push_back(move2);
        room->moveCards(exchangeMove, false);

        LogMessage log;
        log.type = "#Dimeng";
        log.from = a;
        log.to << b;
        log.arg = QString::number(n1);
        log.arg2 = QString::number(n2);
        room->sendLog(log);
        room->getThread()->delay();

        a->setFlags("-DimengTarget");
        b->setFlags("-DimengTarget");
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
            a->setFlags("-DimengTarget");
            b->setFlags("-DimengTarget");
        }
        throw triggerEvent;
    }
}

class Dimeng: public ViewAsSkill {
public:
    Dimeng(): ViewAsSkill("dimeng") {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        DimengCard *card = new DimengCard;
        foreach (const Card *c, cards)
            card->addSubcard(c);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DimengCard");
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

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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
};

class Luanwu: public ZeroCardViewAsSkill {
public:
    Luanwu(): ZeroCardViewAsSkill("luanwu") {
        frequency = Limited;
        limit_mark = "@chaos";
    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@chaos") >= 1;
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

class Weimu: public ProhibitSkill {
public:
    Weimu(): ProhibitSkill("weimu") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && (card->isKindOf("TrickCard") || card->isKindOf("QiceCard"))
               && card->isBlack() && card->getSkillName() != "nosguhuo"; // Be care!!!!!!
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

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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
            int index = (dongzhuo->isFemale()) ? 2 : 1; //@todo_P: audios
            room->broadcastSkillInvoke(objectName(), index);
            if (result == "hp")
                room->loseHp(dongzhuo);
            else
                room->loseMaxHp(dongzhuo);
        }

        return false;
    }
};

class Baonue: public TriggerSkill {
public:
    Baonue(): TriggerSkill("baonue$") {
        events << Damage << PreDamageDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (triggerEvent == PreDamageDone && damage.from)
            damage.from->tag["InvokeBaonue"] = damage.from->getKingdom() == "qun";
        else if (triggerEvent == Damage && player->tag.value("InvokeBaonue", false).toBool() && player->isAlive()) {
            QList<ServerPlayer *> dongzhuos;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->hasLordSkill(objectName()))
                    dongzhuos << p;
            }

            while (!dongzhuos.isEmpty()) {
                ServerPlayer *dongzhuo = room->askForPlayerChosen(player, dongzhuos, objectName(), "@baonue-to", true);
                if (dongzhuo) {
                    dongzhuos.removeOne(dongzhuo);

                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = player;
                    log.to << dongzhuo;
                    log.arg = objectName();
                    room->sendLog(log);
                    room->notifySkillInvoked(dongzhuo, objectName());

                    JudgeStruct judge;
                    judge.pattern = ".|spade";
                    judge.good = true;
                    judge.reason = objectName();
                    judge.who = player;

                    room->judge(judge);

                    if (judge.isGood()) {
                        room->broadcastSkillInvoke(objectName());

                        RecoverStruct recover;
                        recover.who = player;
                        room->recover(dongzhuo, recover);
                    }
                } else
                    break;
            }
        }
        return false;
    }
};

ThicketPackage::ThicketPackage()
    : Package("thicket")
{
    General *xuhuang = new General(this, "xuhuang", "wei"); // WEI 010
    xuhuang->addSkill(new Duanliang);
    xuhuang->addSkill(new DuanliangTargetMod);
    related_skills.insertMulti("duanliang", "#duanliang-target");

    General *caopi = new General(this, "caopi$", "wei", 3); // WEI 014
    caopi->addSkill(new Xingshang);
    caopi->addSkill(new Fangzhu);
    caopi->addSkill(new Songwei);

    General *menghuo = new General(this, "menghuo", "shu"); // SHU 014
    menghuo->addSkill(new SavageAssaultAvoid("huoshou"));
    menghuo->addSkill(new Huoshou);
    menghuo->addSkill(new Zaiqi);
    related_skills.insertMulti("huoshou", "#sa_avoid_huoshou");

    General *zhurong = new General(this, "zhurong", "shu", 4, false); // SHU 015
    zhurong->addSkill(new SavageAssaultAvoid("juxiang"));
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);
    related_skills.insertMulti("juxiang", "#sa_avoid_juxiang");

    General *sunjian = new General(this, "sunjian", "wu"); // WU 009
    sunjian->addSkill(new Yinghun);

    General *lusu = new General(this, "lusu", "wu", 3); // WU 014
    lusu->addSkill(new Haoshi);
    lusu->addSkill(new HaoshiViewAsSkill);
    lusu->addSkill(new HaoshiGive);
    lusu->addSkill(new Dimeng);
    related_skills.insertMulti("haoshi", "#haoshi");
    related_skills.insertMulti("haoshi", "#haoshi-give");

    General *dongzhuo = new General(this, "dongzhuo$", "qun", 8); // QUN 006
    dongzhuo->addSkill(new Jiuchi);
    dongzhuo->addSkill(new Roulin);
    dongzhuo->addSkill(new Benghuai);
    dongzhuo->addSkill(new Baonue);

    General *jiaxu = new General(this, "jiaxu", "qun", 3); // QUN 007
    jiaxu->addSkill(new Wansha);
    jiaxu->addSkill(new Luanwu);
    jiaxu->addSkill(new Weimu);

    addMetaObject<DimengCard>();
    addMetaObject<LuanwuCard>();
    addMetaObject<HaoshiCard>();
}

ADD_PACKAGE(Thicket)

