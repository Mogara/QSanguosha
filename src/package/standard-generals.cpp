#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-skillcards.h"
#include "ai.h"
#include "settings.h"

class Jianxiong: public MasochismSkill {
public:
    Jianxiong(): MasochismSkill("jianxiong") {
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if (card && room->getCardPlace(card->getEffectiveId()) == Player::PlaceTable) {
            QVariant data = QVariant::fromValue(card);
            if (room->askForSkillInvoke(caocao, "jianxiong", data)) {
                room->broadcastSkillInvoke(objectName());
                caocao->obtainCard(card);
            }
        }
    }
};

class Hujia: public TriggerSkill {
public:
    Hujia(): TriggerSkill("hujia$") {
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("hujia");
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *caocao, QVariant &data) const{
		QString pattern = data.toStringList().first();
        QString prompt = data.toStringList().at(1);
        if (pattern != "jink" || prompt.startsWith("@hujia-jink"))
            return false;

        QList<ServerPlayer *> lieges = room->getLieges("wei", caocao);
        if (lieges.isEmpty())
            return false;

        if (!room->askForSkillInvoke(caocao, objectName(), data))
            return false;

        room->broadcastSkillInvoke(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)caocao);
        foreach (ServerPlayer *liege, lieges) {
            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink:" + caocao->objectName(),
                                                tohelp, Card::MethodResponse, caocao);
            if (jink) {
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class TuxiViewAsSkill: public ZeroCardViewAsSkill {
public:
    TuxiViewAsSkill(): ZeroCardViewAsSkill("tuxi") {
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@tuxi";
    }
};

class Tuxi: public PhaseChangeSkill {
public:
    Tuxi(): PhaseChangeSkill("tuxi") {
        view_as_skill = new TuxiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if (zhangliao->getPhase() == Player::Draw) {
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach (ServerPlayer *player, other_players) {
                if (!player->isKongcheng()) {
                    can_invoke = true;
                    break;
                }
            }

            if (can_invoke && room->askForUseCard(zhangliao, "@@tuxi", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

class Tiandu: public TriggerSkill {
public:
    Tiandu(): TriggerSkill("tiandu") {
        frequency = Frequent;
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *guojia, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        if (guojia->askForSkillInvoke(objectName(), data_card)) {
            guojia->obtainCard(judge->card);
            room->broadcastSkillInvoke(objectName());
            return false;
        }

        return false;
    }
};

Yiji::Yiji(): MasochismSkill("yiji") {
    frequency = Frequent;
    n = 2;
}

void Yiji::onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
    Room *room = guojia->getRoom();
    int x = damage.damage, i;
    for (i = 0; i < x; i++) {
        if (!guojia->isAlive() || !room->askForSkillInvoke(guojia, objectName()))
            return;
        room->broadcastSkillInvoke("yiji");

        QList<ServerPlayer *> _guojia;
        _guojia.append(guojia);
        QList<int> yiji_cards = room->getNCards(n, false);

        CardsMoveStruct move;
        move.card_ids = yiji_cards;
        move.from = NULL;
        move.from_place = Player::PlaceTable;
        move.to = guojia;
        move.to_player_name = guojia->objectName();
        move.to_place = Player::PlaceHand;
        move.reason = CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString());
        QList<CardsMoveStruct> moves;
        moves.append(move);
        room->notifyMoveCards(true, moves, false, _guojia);
        room->notifyMoveCards(false, moves, false, _guojia);

        QList<int> origin_yiji = yiji_cards;
        while (room->askForYiji(guojia, yiji_cards, objectName(), true, false, true, -1, room->getAlivePlayers())) {
            CardsMoveStruct move;
            move.from = guojia;
            move.from_player_name = guojia->objectName();
            move.from_place = Player::PlaceHand;
            move.to = NULL;
            move.to_place = Player::PlaceTable;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString());
            foreach (int id, origin_yiji) {
                if (room->getCardPlace(id) != Player::DrawPile) {
                    move.card_ids << id;
                    yiji_cards.removeOne(id);
                }
            }
            origin_yiji = yiji_cards;
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->notifyMoveCards(true, moves, false, _guojia);
            room->notifyMoveCards(false, moves, false, _guojia);
            if (!guojia->isAlive())
                return;
        }

        if (!yiji_cards.isEmpty()) {
            CardsMoveStruct move;
            move.from = guojia;
            move.from_player_name = guojia->objectName();
            move.from_place = Player::PlaceHand;
            move.to = NULL;
            move.to_place = Player::PlaceTable;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString());
            move.card_ids = yiji_cards;
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->notifyMoveCards(true, moves, false, _guojia);
            room->notifyMoveCards(false, moves, false, _guojia);

            DummyCard *dummy = new DummyCard;
            foreach (int id, yiji_cards)
                dummy->addSubcard(id);
            guojia->obtainCard(dummy, false);
            delete dummy;
        } else {
            continue;
        }
    }
}

class Ganglie: public MasochismSkill {
public:
    Ganglie(): MasochismSkill("ganglie") {
    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant data = QVariant::fromValue(damage);

        if (room->askForSkillInvoke(xiahou, "ganglie", data)) {
            room->broadcastSkillInvoke(objectName());

            JudgeStruct judge;
            judge.pattern = ".|heart";
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if (!from || from->isDead()) return;
            if (judge.isGood()) {
                if (from->getHandcardNum() < 2 || !room->askForDiscard(from, objectName(), 2, 2, true))
                    room->damage(DamageStruct(objectName(), xiahou, from));
            }
        }
    }
};

class Fankui: public MasochismSkill {
public:
    Fankui(): MasochismSkill("fankui") {
    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if (from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)) {
            room->broadcastSkillInvoke(objectName());
            int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, simayi->objectName());
            room->obtainCard(simayi, Sanguosha->getCard(card_id),
                             reason, room->getCardPlace(card_id) != Player::PlaceHand);
        }
    }
};

class Guicai: public TriggerSkill {
public:
    Guicai(): TriggerSkill("guicai") {
        events << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->isKongcheng())
            return false;

        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guicai-card" << judge->who->objectName()
                    << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");
        bool forced = false;
        if (player->getMark("JilveEvent") == int(AskForRetrial))
            forced = true;
        const Card *card = room->askForCard(player, forced ? ".!" : "." , prompt, data, Card::MethodResponse, judge->who, true);
        if (forced && card == NULL)
            card = player->getRandomHandCard();
        if (card) {
            if (player->hasInnateSkill("guicai") || !player->hasSkill("jilve"))
                room->broadcastSkillInvoke(objectName());
            else
                room->broadcastSkillInvoke("jilve", 1);
            room->retrial(card, player, judge, objectName());
        }

        return false;
    }
};

class LuoyiBuff: public TriggerSkill {
public:
    LuoyiBuff(): TriggerSkill("#luoyi") {
        events << DamageCaused;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasFlag("luoyi") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer || !damage.by_user) return false;
        const Card *reason = damage.card;
        if (reason && (reason->isKindOf("Slash") || reason->isKindOf("Duel"))) {
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Luoyi: public DrawCardsSkill {
public:
    Luoyi(): DrawCardsSkill("luoyi") {
    }

    virtual int getDrawNum(ServerPlayer *xuchu, int n) const{
        Room *room = xuchu->getRoom();
        if (room->askForSkillInvoke(xuchu, objectName())) {
            room->broadcastSkillInvoke(objectName());
            xuchu->setFlags(objectName());
            return n - 1;
        } else
            return n;
    }
};

class Luoshen: public TriggerSkill {
public:
    Luoshen(): TriggerSkill("luoshen") {
        events << EventPhaseStart << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhenji, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && zhenji->getPhase() == Player::Start) {
            while (zhenji->askForSkillInvoke("luoshen")) {
                room->broadcastSkillInvoke(objectName());

                JudgeStruct judge;
                judge.pattern = ".|black";
                judge.good = true;
                judge.reason = objectName();
                judge.play_animation = false;
                judge.who = zhenji;
                judge.time_consuming = true;

                room->judge(judge);
                if (judge.isBad())
                    break;
            }
        } else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason == objectName()) {
                bool isHegVer = zhenji->getGeneralName() != "zhenji"
                                && (zhenji->getGeneralName() == "heg_zhenji" || zhenji->getGeneral2Name() == "heg_zhenji");
                if (judge->card->isBlack()) {
                    if (isHegVer && zhenji->hasSkills("guicai|guidao|huanshi")) {
                        CardMoveReason reason(CardMoveReason::S_REASON_JUDGEDONE, zhenji->objectName(), QString(), judge->reason);
                        room->moveCardTo(judge->card, zhenji, NULL, Player::PlaceTable, reason, true);
                        QVariantList luoshen_list = zhenji->tag[objectName()].toList();
                        luoshen_list << judge->card->getEffectiveId();
                        zhenji->tag[objectName()] = luoshen_list;
                    } else {
                        zhenji->obtainCard(judge->card);
                    }
                } else {
                    if (isHegVer && zhenji->hasSkills("guicai|guidao|huanshi")) {
                        DummyCard *dummy = new DummyCard;
                        foreach (QVariant id, zhenji->tag[objectName()].toList())
                            dummy->addSubcard(id.toInt());
                        zhenji->obtainCard(dummy);
                        zhenji->tag.remove(objectName());
                        delete dummy;
                    }
                }
            }
        }

        return false;
    }
};

class Qingguo: public OneCardViewAsSkill {
public:
    Qingguo(): OneCardViewAsSkill("qingguo") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(originalCard->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "jink";
    }
};

class RendeViewAsSkill: public ViewAsSkill {
public:
    RendeViewAsSkill(): ViewAsSkill("rende") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (ServerInfo.GameMode == "04_1v3" && selected.length() + Self->getMark("rende") >= 2)
           return false;
        else {
            if (to_select->isEquipped()) return false;
            if (Sanguosha->currentRoomState()->getCurrentCardUsePattern() == "@@rende") {
                QList<int> rende_list = StringList2IntList(Self->property("rende").toString().split("+"));
                return rende_list.contains(to_select->getEffectiveId());
            } else
                return true;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (ServerInfo.GameMode == "04_1v3" && player->getMark("rende") >= 2)
           return false;
        return !player->hasUsed("RendeCard") && !player->isKongcheng();
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@rende";
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        RendeCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public TriggerSkill {
public:
    Rende(): TriggerSkill("rende") {
        events << EventPhaseChanging;
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getMark("rende") > 0;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        room->setPlayerMark(player, "rende", 0);
        room->setPlayerProperty(player, "rende", QString());
        return false;
    }
};

JijiangViewAsSkill::JijiangViewAsSkill(): ZeroCardViewAsSkill("jijiang$") {
}

bool JijiangViewAsSkill::isEnabledAtPlay(const Player *player) const{
    return hasShuGenerals(player) && player->hasLordSkill("jijiang") && !player->hasFlag("Global_JijiangFailed")
           && Slash::IsAvailable(player);
}

bool JijiangViewAsSkill::isEnabledAtResponse(const Player *player, const QString &pattern) const{
    return hasShuGenerals(player)
           && (pattern == "slash" || pattern == "@jijiang")
           && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
           && !player->hasFlag("Global_JijiangFailed");
}

const Card *JijiangViewAsSkill::viewAs() const{
    return new JijiangCard;
}

bool JijiangViewAsSkill::hasShuGenerals(const Player *player) {
    foreach (const Player *p, player->getSiblings())
        if (p->isAlive() && p->getKingdom() == "shu")
            return true;
    return false;
}

class Jijiang: public TriggerSkill {
public:
    Jijiang(): TriggerSkill("jijiang$") {
        events << CardAsked;
        view_as_skill = new JijiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("jijiang");
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *liubei, QVariant &data) const{
        QString pattern = data.toStringList().first();
        QString prompt = data.toStringList().at(1);
        if (pattern != "slash" || prompt.startsWith("@jijiang-slash"))
            return false;

        QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
        if (lieges.isEmpty())
            return false;

        if (!room->askForSkillInvoke(liubei, objectName(), data))
            return false;

        room->broadcastSkillInvoke(objectName(), getEffectIndex(liubei, NULL));

        foreach (ServerPlayer *liege, lieges) {
            const Card *slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(), QVariant(), Card::MethodResponse, liubei);
            if (slash) {
                room->provide(slash);
                return true;
            }
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        int r = 1 + qrand() % 2;
        if (!player->hasInnateSkill("jijiang") && player->hasSkill("ruoyu"))
            r += 2;
        return r;
    }
};

class Wusheng: public OneCardViewAsSkill {
public:
    Wusheng(): OneCardViewAsSkill("wusheng") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *card) const{
        if (!card->isRed())
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY
            && Self->getWeapon() && card->getEffectiveId() == Self->getWeapon()->getId() && card->isKindOf("Crossbow"))
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Paoxiao: public TargetModSkill {
public:
    Paoxiao(): TargetModSkill("paoxiao") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class Longdan: public OneCardViewAsSkill {
public:
    Longdan(): OneCardViewAsSkill("longdan") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        const Card *card = to_select;

        switch (Sanguosha->currentRoomState()->getCurrentCardUseReason()) {
        case CardUseStruct::CARD_USE_REASON_PLAY: {
                return card->isKindOf("Jink");
            }
        case CardUseStruct::CARD_USE_REASON_RESPONSE:
        case CardUseStruct::CARD_USE_REASON_RESPONSE_USE: {
                QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
                if (pattern == "slash")
                    return card->isKindOf("Jink");
                else if (pattern == "jink")
                    return card->isKindOf("Slash");
            }
        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        if (originalCard->isKindOf("Slash")) {
            Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
            jink->addSubcard(originalCard);
            jink->setSkillName(objectName());
            return jink;
        } else if (originalCard->isKindOf("Jink")) {
            Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
            slash->addSubcard(originalCard);
            slash->setSkillName(objectName());
            return slash;
        } else
            return NULL;
    }
};

class Tieji: public TriggerSkill {
public:
    Tieji(): TriggerSkill("tieji") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (player != use.from || !use.card->isKindOf("Slash"))
            return false;
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        int index = 0;
        foreach (ServerPlayer *p, use.to) {
            if (player->askForSkillInvoke(objectName(), QVariant::fromValue(p))) {
                room->broadcastSkillInvoke(objectName());

                p->setFlags("TiejiTarget"); // For AI

                JudgeStruct judge;
                judge.pattern = ".|red";
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                try {
                    room->judge(judge);
                }
                catch (TriggerEvent triggerEvent) {
                    if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                        p->setFlags("-TiejiTarget");
                    throw triggerEvent;
                }

                if (judge.isGood()) {
                    LogMessage log;
                    log.type = "#NoJink";
                    log.from = p;
                    room->sendLog(log);
                    jink_list.replace(index, QVariant(0));
                }

                p->setFlags("-TiejiTarget");
            }
            index++;
        }
        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }
};

class Guanxing: public PhaseChangeSkill {
public:
    Guanxing(): PhaseChangeSkill("guanxing") {
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if (zhuge->getPhase() == Player::Start && zhuge->askForSkillInvoke(objectName())) {
            Room *room = zhuge->getRoom();
            int index = qrand() % 2 + 1;
            if (objectName() == "guanxing" && !zhuge->hasInnateSkill(objectName()) && zhuge->hasSkill("zhiji"))
                index += 2;
            room->broadcastSkillInvoke(objectName(), index);
            QList<int> guanxing = room->getNCards(getGuanxingNum(room));

            LogMessage log;
            log.type = "$ViewDrawPile";
            log.from = zhuge;
            log.card_str = IntList2StringList(guanxing).join("+");
            room->doNotify(zhuge, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

            room->askForGuanxing(zhuge, guanxing, false);
        }

        return false;
    }

    virtual int getGuanxingNum(Room *room) const{
        return qMin(5, room->alivePlayerCount());
    }
};

class Kongcheng: public ProhibitSkill {
public:
    Kongcheng(): ProhibitSkill("kongcheng") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && (card->isKindOf("Slash") || card->isKindOf("Duel")) && to->isKongcheng();
    }
};

class KongchengEffect: public TriggerSkill {
public:
    KongchengEffect() :TriggerSkill("#kongcheng-effect") {
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->isKongcheng()) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && move.from_places.contains(Player::PlaceHand))
                room->broadcastSkillInvoke("kongcheng");
        }

        return false;
    }
};

class Jizhi: public TriggerSkill {
public:
    Jizhi(): TriggerSkill("jizhi") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *yueying, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->getTypeId() == Card::TypeTrick
            && (yueying->getMark("JilveEvent") > 0 || room->askForSkillInvoke(yueying, objectName()))) {
            if (yueying->getMark("JilveEvent") > 0)
                room->broadcastSkillInvoke("jilve", 5);
            else
                room->broadcastSkillInvoke(objectName());

            QList<int> ids = room->getNCards(1, false);
            CardsMoveStruct move;
            move.card_ids = ids;
            move.to = yueying;
            move.to_place = Player::PlaceTable;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, yueying->objectName(), "jizhi", QString());
            room->moveCardsAtomic(move, true);

            int id = ids.first();
            const Card *card = Sanguosha->getCard(id);
            if (!card->isKindOf("BasicCard")) {
                yueying->obtainCard(card);
            } else {
                const Card *card_ex = NULL;
                if (!yueying->isKongcheng())
                    card_ex = room->askForCard(yueying, ".", "@jizhi-exchange:::" + card->objectName(),
                                               QVariant::fromValue((CardStar)card), Card::MethodNone);
                if (card_ex) {
                    CardMoveReason reason1(CardMoveReason::S_REASON_PUT, yueying->objectName(), "jizhi", QString());
                    CardMoveReason reason2(CardMoveReason::S_REASON_OVERRIDE, yueying->objectName(), "jizhi", QString());
                    CardsMoveStruct move1(QList<int>() << card_ex->getEffectiveId(), yueying, NULL, Player::DrawPile, reason1);
                    CardsMoveStruct move2(ids, yueying, yueying, Player::PlaceHand, reason2);

                    QList<CardsMoveStruct> moves;
                    moves.append(move1);
                    moves.append(move2);
                    room->moveCardsAtomic(moves, false);
                } else {
                    CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, yueying->objectName(), "jizhi", QString());
                    room->throwCard(card, reason, NULL);
                }
            }
        }

        return false;
    }
};

class Qicai: public TargetModSkill {
public:
    Qicai(): TargetModSkill("qicai") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class Zhiheng: public ViewAsSkill {
public:
    Zhiheng(): ViewAsSkill("zhiheng") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (Self->getMark("ZhihengInLatestKOF") > 0 && selected.length() >= 2) return false;
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);
        zhiheng_card->setSkillName(objectName());
        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("ZhihengCard");
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@zhiheng";
    }
};

class ZhihengForKOF: public GameStartSkill {
public:
    ZhihengForKOF(): GameStartSkill("#zhiheng-for-kof") {
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if (room->getMode() == "02_1v1" && Config.value("1v1/Rule", "Classical").toString() == "2013")
            room->setPlayerMark(player, "ZhihengInLatestKOF", 1);
    }
};

class Jiuyuan: public TriggerSkill {
public:
    Jiuyuan(): TriggerSkill("jiuyuan$") {
        events << TargetConfirmed << PreHpRecover;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("jiuyuan");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *sunquan, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Peach") && use.from && use.from->getKingdom() == "wu"
                && sunquan != use.from && sunquan->hasFlag("Global_Dying")) {
                room->setCardFlag(use.card, "jiuyuan");
            }
        } else if (triggerEvent == PreHpRecover) {
            RecoverStruct rec = data.value<RecoverStruct>();
            if (rec.card && rec.card->hasFlag("jiuyuan")) {
                room->notifySkillInvoked(sunquan, "jiuyuan");
                room->broadcastSkillInvoke("jiuyuan", rec.who->isMale() ? 1 : 2);

                LogMessage log;
                log.type = "#JiuyuanExtraRecover";
                log.from = sunquan;
                log.to << rec.who;
                log.arg = objectName();
                room->sendLog(log);

                rec.recover++;
                data = QVariant::fromValue(rec);
            }
        }

        return false;
    }
};

class Yingzi: public DrawCardsSkill {
public:
    Yingzi(): DrawCardsSkill("yingzi") {
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        if (room->askForSkillInvoke(zhouyu, objectName())) {
            int index = qrand() % 2 + 1;
            if (!zhouyu->hasInnateSkill(objectName())) {
                if (zhouyu->hasSkill("mouduan"))
                    index += 2;
                else if (zhouyu->hasSkill("hunzi"))
                    index = 5;
            }

            room->broadcastSkillInvoke(objectName(), index);
            return n + 1;
        } else
            return n;
    }
};

class Fanjian: public ZeroCardViewAsSkill {
public:
    Fanjian(): ZeroCardViewAsSkill("fanjian") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public TriggerSkill {
public:
    Keji(): TriggerSkill("keji") {
        events << EventPhaseChanging;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Discard) {
            if (!lvmeng->hasFlag("Global_SlashInPlayPhase") && lvmeng->askForSkillInvoke(objectName())) {
                if (lvmeng->getHandcardNum() > lvmeng->getMaxCards()) {
                    int index = qrand() % 2 + 1;
                    if (!lvmeng->hasInnateSkill(objectName()) && lvmeng->hasSkill("mouduan"))
                        index += 2;
                    room->broadcastSkillInvoke(objectName(), index);
                }
                lvmeng->skip(Player::Discard);
            }
        }

        return false;
    }
};

class Lianying: public TriggerSkill {
public:
    Lianying(): TriggerSkill("lianying") {
        events << BeforeCardsMove << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *luxun, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == luxun && move.from_places.contains(Player::PlaceHand)) {
            if (triggerEvent == BeforeCardsMove) {
                if (luxun->isKongcheng()) return false;
                foreach (int id, luxun->handCards()) {
                    if (!move.card_ids.contains(id))
                        return false;
                }
                if (luxun->getMaxCards() == 0 && luxun->getPhase() == Player::Discard
                    && move.reason.m_reason == CardMoveReason::S_REASON_RULEDISCARD) {
                    luxun->setFlags("LianyingZeroMaxCards");
                    return false;
                }
                luxun->addMark(objectName());
            } else {
                if (luxun->getMark(objectName()) == 0)
                    return false;
                luxun->removeMark(objectName());
                if (room->askForSkillInvoke(luxun, objectName(), data)) {
                    room->broadcastSkillInvoke(objectName());
                    luxun->drawCards(1);
                }
            }
        }

        return false;
    }
};

class LianyingForZeroMaxCards: public TriggerSkill {
public:
    LianyingForZeroMaxCards(): TriggerSkill("#lianying-for-zero-maxcards") {
        events << EventPhaseChanging;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.from == Player::Discard && player->hasFlag("LianyingZeroMaxCards")) {
            player->setFlags("-LianyingZeroMaxCards");
            if (player->isKongcheng() && room->askForSkillInvoke(player, "lianying")) {
                room->broadcastSkillInvoke("lianying");
                player->drawCards(1);
            }
        }
        return false;
    }
};

class Qixi: public OneCardViewAsSkill {
public:
    Qixi(): OneCardViewAsSkill("qixi") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class Kurou: public ZeroCardViewAsSkill {
public:
    Kurou(): ZeroCardViewAsSkill("kurou") {
    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Guose: public OneCardViewAsSkill {
public:
    Guose(): OneCardViewAsSkill("guose") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Indulgence *indulgence = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        indulgence->addSubcard(originalCard->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class LiuliViewAsSkill: public OneCardViewAsSkill {
public:
    LiuliViewAsSkill(): OneCardViewAsSkill("liuli") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@liuli";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->addSubcard(originalCard);
        return liuli_card;
    }
};

class Liuli: public TriggerSkill {
public:
    Liuli(): TriggerSkill("liuli") {
        events << TargetConfirming;
        view_as_skill = new LiuliViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card && use.card->isKindOf("Slash")
            && use.to.contains(daqiao) && daqiao->canDiscard(daqiao, "he") && room->alivePlayerCount() > 2) {
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(use.from);

            bool can_invoke = false;
            foreach (ServerPlayer *p, players) {
                if (use.from->canSlash(p, use.card) && daqiao->inMyAttackRange(p)) {
                    can_invoke = true;
                    break;
                }
            }

            if (can_invoke) {
                QString prompt = "@liuli:" + use.from->objectName();
                room->setPlayerFlag(use.from, "LiuliSlashSource");
                // a temp nasty trick
                daqiao->tag["liuli-card"] = QVariant::fromValue((CardStar)use.card); // for the server (AI)
                room->setPlayerProperty(daqiao, "liuli", use.card->toString()); // for the client (UI)
                if (room->askForUseCard(daqiao, "@@liuli", prompt, -1, Card::MethodDiscard)) {
                    daqiao->tag.remove("liuli-card");
                    room->setPlayerProperty(daqiao, "liuli", QString());
                    room->setPlayerFlag(use.from, "-LiuliSlashSource");
                    foreach (ServerPlayer *p, players) {
                        if (p->hasFlag("LiuliTarget")) {
                            p->setFlags("-LiuliTarget");
                            use.to.removeOne(daqiao);
                            use.to.append(p);
                            room->sortByActionOrder(use.to);
                            data = QVariant::fromValue(use);
                            room->getThread()->trigger(TargetConfirming, room, p, data);
                            return false;
                        }
                    }
                } else {
                    daqiao->tag.remove("liuli-card");
                    room->setPlayerProperty(daqiao, "liuli", QString());
                    room->setPlayerFlag(use.from, "-LiuliSlashSource");
                }
            }
        }

        return false;
    }
};

class Jieyin: public ViewAsSkill {
public:
    Jieyin(): ViewAsSkill("jieyin") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= 2 && !player->hasUsed("JieyinCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() > 1 || Self->isJilei(to_select))
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        JieyinCard *jieyin_card = new JieyinCard();
        jieyin_card->addSubcards(cards);
        return jieyin_card;
    }
};

class Xiaoji: public TriggerSkill {
public:
    Xiaoji(): TriggerSkill("xiaoji") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == sunshangxiang && move.from_places.contains(Player::PlaceEquip)) {
            for (int i = 0; i < move.card_ids.size(); i++) {
                if (!sunshangxiang->isAlive())
                    return false;
                if (move.from_places[i] == Player::PlaceEquip) {
                    if (room->askForSkillInvoke(sunshangxiang, objectName())) {
                        room->broadcastSkillInvoke(objectName());
                        sunshangxiang->drawCards(2);
                    } else {
                        break;
                    }
                }
            }
        }
        return false;
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

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            bool can_invoke = false;
            if (use.card->isKindOf("Slash") && TriggerSkill::triggerable(use.from) && use.from == player) {
                can_invoke = true;
                QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
                int index = 0;
                for (int i = 0; i < use.to.length(); i++) {
                    int n = jink_list.at(index).toInt();
                    if (n > 0 && n < 2)
                        jink_list.replace(index, QVariant(2));
                    index++;
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

class Lijian: public OneCardViewAsSkill {
public:
    Lijian(): OneCardViewAsSkill("lijian") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("LijianCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isJilei(to_select);
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

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        if (diaochan->getPhase() == Player::Finish) {
            Room *room = diaochan->getRoom();
            if (room->askForSkillInvoke(diaochan, objectName())) {
                room->broadcastSkillInvoke(objectName());
                diaochan->drawCards(1);
            }
        }

        return false;
    }
};

class Qingnang: public OneCardViewAsSkill {
public:
    Qingnang(): OneCardViewAsSkill("qingnang") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "h") && !player->hasUsed("QingnangCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped() && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(originalCard->getId());
        return qingnang_card;
    }
};

class Jijiu: public OneCardViewAsSkill {
public:
    Jijiu(): OneCardViewAsSkill("jijiu") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach") && !player->hasFlag("Global_PreventPeach")
                && player->getPhase() == Player::NotActive && player->canDiscard(player, "he");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isRed();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Peach *peach = new Peach(originalCard->getSuit(), originalCard->getNumber());
        peach->addSubcard(originalCard->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

class Qianxun: public ProhibitSkill {
public:
    Qianxun(): ProhibitSkill("qianxun") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && (card->isKindOf("Snatch") || card->isKindOf("Indulgence"));
    }
};

class Mashu: public DistanceSkill {
public:
    Mashu(): DistanceSkill("mashu") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

class Wangzun: public PhaseChangeSkill {
public:
    Wangzun(): PhaseChangeSkill("wangzun") {
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if (!isNormalGameMode(room->getMode()))
            return false;
        if (target->isLord() && target->getPhase() == Player::Start) {
            ServerPlayer *yuanshu = room->findPlayerBySkillName(objectName());
            if (yuanshu && room->askForSkillInvoke(yuanshu, objectName())) {
                room->broadcastSkillInvoke(objectName());
                yuanshu->drawCards(1);
                room->setPlayerFlag(target, "WangzunDecMaxCards");
            }
        }
        return false;
    }
};

class WangzunMaxCards: public MaxCardsSkill {
public:
    WangzunMaxCards(): MaxCardsSkill("#wangzun-maxcard") {
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasFlag("WangzunDecMaxCards"))
            return -1;
        else
            return 0;
    }
};

class Tongji: public ProhibitSkill {
public:
    Tongji(): ProhibitSkill("tongji") {
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const{
        if (card->isKindOf("Slash")) {
            if (to->hasSkill(objectName()))
                return false;
            // get rangefix
            int rangefix = 0;
            if (card->isVirtualCard()) {
                QList<int> subcards = card->getSubcards();
                if (from->getWeapon() && subcards.contains(from->getWeapon()->getId())) {
                    const Weapon *weapon = qobject_cast<const Weapon *>(from->getWeapon()->getRealCard());
                    rangefix += weapon->getRange() - 1;
                }

                if (from->getOffensiveHorse() && subcards.contains(from->getOffensiveHorse()->getId()))
                    rangefix += 1;
            }
            // find yuanshu
            foreach (const Player *p, from->getSiblings()) {
                if (p->isAlive() && p->hasSkill(objectName()) && p->getHandcardNum() > p->getHp()
                    && from->distanceTo(p, rangefix) <= from->getAttackRange()) {
                    return true;
                }
            }
        }
        return false;
    }
};

class Yaowu: public TriggerSkill {
public:
    Yaowu(): TriggerSkill("yaowu") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash") && damage.card->isRed()
            && damage.from && damage.from->isAlive()) {
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(damage.to, objectName());

            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = damage.to;
            log.arg = objectName();
            room->sendLog(log);

            if (damage.from->isWounded() && room->askForChoice(damage.from, objectName(), "recover+draw", data) == "recover") {
                RecoverStruct recover;
                recover.who = damage.to;
                room->recover(damage.from, recover);
            } else {
                damage.from->drawCards(1);
            }
        }
        return false;
    }
};

void StandardPackage::addGenerals() {
    // Wei
    General *caocao = new General(this, "caocao$", "wei"); // WEI 001
    caocao->addSkill(new Jianxiong);
    caocao->addSkill(new Hujia);

    General *simayi = new General(this, "simayi", "wei", 3); // WEI 002
    simayi->addSkill(new Fankui);
    simayi->addSkill(new Guicai);

    General *xiahoudun = new General(this, "xiahoudun", "wei"); // WEI 003
    xiahoudun->addSkill(new Ganglie);

    General *zhangliao = new General(this, "zhangliao", "wei"); // WEI 004
    zhangliao->addSkill(new Tuxi);
    zhangliao->addSkill(new SPConvertSkill("zhangliao", "tw_zhangliao"));

    General *xuchu = new General(this, "xuchu", "wei"); // WEI 005
    xuchu->addSkill(new Luoyi);
    xuchu->addSkill(new LuoyiBuff);
    related_skills.insertMulti("luoyi", "#luoyi");

    General *guojia = new General(this, "guojia", "wei", 3); // WEI 006
    guojia->addSkill(new Tiandu);
    guojia->addSkill(new Yiji);
    guojia->addSkill(new SPConvertSkill("guojia", "tw_guojia"));

    General *zhenji = new General(this, "zhenji", "wei", 3, false); // WEI 007
    zhenji->addSkill(new Qingguo);
    zhenji->addSkill(new Luoshen);
    zhenji->addSkill(new SPConvertSkill("zhenji", "sp_zhenji+heg_zhenji+tw_zhenji"));

    // Shu
    General *liubei = new General(this, "liubei$", "shu"); // SHU 001
    liubei->addSkill(new Rende);
    liubei->addSkill(new Jijiang);

    General *guanyu = new General(this, "guanyu", "shu"); // SHU 002
    guanyu->addSkill(new Wusheng);

    General *zhangfei = new General(this, "zhangfei", "shu"); // SHU 003
    zhangfei->addSkill(new Paoxiao);

    General *zhugeliang = new General(this, "zhugeliang", "shu", 3); // SHU 004
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);
    zhugeliang->addSkill(new KongchengEffect);
    related_skills.insertMulti("kongcheng", "#kongcheng-effect");
    zhugeliang->addSkill(new SPConvertSkill("zhugeliang", "heg_zhugeliang+tw_zhugeliang"));

    General *zhaoyun = new General(this, "zhaoyun", "shu"); // SHU 005
    zhaoyun->addSkill(new Longdan);
    zhaoyun->addSkill(new SPConvertSkill("zhaoyun", "tw_zhaoyun"));

    General *machao = new General(this, "machao", "shu"); // SHU 006
    machao->addSkill(new Tieji);
    machao->addSkill(new Mashu);
    machao->addSkill(new SPConvertSkill("machao", "sp_machao+tw_machao"));

    General *huangyueying = new General(this, "huangyueying", "shu", 3, false); // SHU 007
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill(new Qicai);

    // Wu
    General *sunquan = new General(this, "sunquan$", "wu"); // WU 001
    sunquan->addSkill(new Zhiheng);
    sunquan->addSkill(new ZhihengForKOF);
    sunquan->addSkill(new Jiuyuan);
    related_skills.insertMulti("zhiheng", "#zhiheng-for-kof");

    General *ganning = new General(this, "ganning", "wu"); // WU 002
    ganning->addSkill(new Qixi);
    ganning->addSkill(new SPConvertSkill("ganning", "tw_ganning"));

    General *lvmeng = new General(this, "lvmeng", "wu"); // WU 003
    lvmeng->addSkill(new Keji);

    General *huanggai = new General(this, "huanggai", "wu"); // WU 004
    huanggai->addSkill(new Kurou);
    huanggai->addSkill(new SPConvertSkill("huanggai", "tw_huanggai"));

    General *zhouyu = new General(this, "zhouyu", "wu", 3); // WU 005
    zhouyu->addSkill(new Yingzi);
    zhouyu->addSkill(new Fanjian);
    zhouyu->addSkill(new SPConvertSkill("zhouyu", "heg_zhouyu+sp_heg_zhouyu"));

    General *daqiao = new General(this, "daqiao", "wu", 3, false); // WU 006
    daqiao->addSkill(new Guose);
    daqiao->addSkill(new Liuli);
    daqiao->addSkill(new SPConvertSkill("daqiao", "wz_daqiao+tw_daqiao"));

    General *luxun = new General(this, "luxun", "wu", 3); // WU 007
    luxun->addSkill(new Qianxun);
    luxun->addSkill(new Lianying);
    luxun->addSkill(new LianyingForZeroMaxCards);
    related_skills.insertMulti("lianying", "#lianying-for-zero-maxcards");
    luxun->addSkill(new SPConvertSkill("luxun", "tw_luxun"));

    General *sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false); // WU 008
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);
    sunshangxiang->addSkill(new SPConvertSkill("sunshangxiang", "sp_sunshangxiang"));

    // Qun
    General *huatuo = new General(this, "huatuo", "qun", 3); // QUN 001
    huatuo->addSkill(new Qingnang);
    huatuo->addSkill(new Jijiu);

    General *lvbu = new General(this, "lvbu", "qun"); // QUN 002
    lvbu->addSkill(new Wushuang);
    lvbu->addSkill(new SPConvertSkill("lvbu", "heg_lvbu+tw_lvbu"));

    General *diaochan = new General(this, "diaochan", "qun", 3, false); // QUN 003
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);
    diaochan->addSkill(new SPConvertSkill("diaochan", "sp_diaochan+heg_diaochan+tw_diaochan"));

    General *st_yuanshu = new General(this, "st_yuanshu", "qun", 4);
    st_yuanshu->addSkill(new Wangzun);
    st_yuanshu->addSkill(new WangzunMaxCards);
    related_skills.insertMulti("wangzun", "#wangzun-maxcard");
    st_yuanshu->addSkill(new Tongji);

    General *st_huaxiong = new General(this, "st_huaxiong", "qun", 6);
    st_huaxiong->addSkill(new Yaowu);

    // for skill cards
    addMetaObject<ZhihengCard>();
    addMetaObject<RendeCard>();
    addMetaObject<TuxiCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<KurouCard>();
    addMetaObject<LijianCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<QingnangCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<JijiangCard>();
}

class SuperZhiheng: public Zhiheng {
public:
    SuperZhiheng():Zhiheng() {
        setObjectName("super_zhiheng");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && player->usedTimes("ZhihengCard") < (player->getLostHp() + 1);
    }
};

class SuperGuanxing: public Guanxing {
public:
    SuperGuanxing(): Guanxing() {
        setObjectName("super_guanxing");
    }

    virtual int getGuanxingNum(Room *room) const{
        return 5;
    }
};

class SuperMaxCards: public MaxCardsSkill {
public:
    SuperMaxCards(): MaxCardsSkill("super_max_cards") {
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasSkill(objectName()))
            return target->getMark("@max_cards_test");
        return 0;
    }
};

class SuperOffensiveDistance: public DistanceSkill {
public:
    SuperOffensiveDistance(): DistanceSkill("super_offensive_distance") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasSkill(objectName()))
            return -from->getMark("@offensive_distance_test");
        else
            return 0;
    }
};

class SuperDefensiveDistance: public DistanceSkill {
public:
    SuperDefensiveDistance(): DistanceSkill("super_defensive_distance") {
    }

    virtual int getCorrect(const Player *, const Player *to) const{
        if (to->hasSkill(objectName()))
            return to->getMark("@defensive_distance_test");
        else
            return 0;
    }
};

#include "sp-package.h"
class SuperYongsi: public Yongsi {
public:
    SuperYongsi(): Yongsi() {
        setObjectName("super_yongsi");
    }

    virtual int getKingdoms(ServerPlayer *yuanshu) const{
        return yuanshu->getMark("@yongsi_test");
    }
};

#include "wind.h"
class SuperJushou: public Jushou {
public:
    SuperJushou(): Jushou() {
        setObjectName("super_jushou");
    }

    virtual int getJushouDrawNum(ServerPlayer *caoren) const{
        return caoren->getMark("@jushou_test");
    }
};

#include "god.h"
#include "maneuvering.h"
class NosJuejing: public TriggerSkill {
public:
    NosJuejing(): TriggerSkill("nosjuejing") {
        events << CardsMoveOneTime << EventPhaseChanging;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *gaodayihao, QVariant &data) const{
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != gaodayihao && move.to != gaodayihao)
                return false;
            if ((move.to_place != Player::PlaceHand && !move.from_places.contains(Player::PlaceHand))
                || gaodayihao->getPhase() == Player::Discard) {
                return false;
            }
        }
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::Draw) {
                gaodayihao->skip(change.to);
                return false;
            } else if (change.to != Player::Finish)
                return false;
        }
        if (gaodayihao->getHandcardNum() == 4)
            return false;
        int diff = abs(gaodayihao->getHandcardNum() - 4);
        if (gaodayihao->getHandcardNum() < 4) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = gaodayihao;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(gaodayihao, objectName());
            gaodayihao->drawCards(diff);
        } else if (gaodayihao->getHandcardNum() > 4) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = gaodayihao;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(gaodayihao, objectName());
            room->askForDiscard(gaodayihao, objectName(), diff, diff);
        }

        return false;
    }
};

class NosLonghun: public Longhun {
public:
    NosLonghun(): Longhun() {
        setObjectName("noslonghun");
    }

    virtual int getEffHp(const Player *) const{
        return 1;
    }
};

class NosDuojian: public TriggerSkill {
public:
    NosDuojian(): TriggerSkill("#noslonghun_duojian") {
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *gaodayihao, QVariant &) const{
        if (gaodayihao->getPhase() == Player::Start) {
            foreach (ServerPlayer *p, room->getOtherPlayers(gaodayihao)) {
               if (p->getWeapon() && p->getWeapon()->isKindOf("QinggangSword")) {
                   if (room->askForSkillInvoke(gaodayihao, "noslonghun")) {
                       room->broadcastSkillInvoke("noslonghun", 5);
                       gaodayihao->obtainCard(p->getWeapon());
                    }
                    break;
                }
            }
        }

        return false;
    }
};

TestPackage::TestPackage()
    : Package("test")
{
    // for test only
    General *zhiba_sunquan = new General(this, "zhiba_sunquan$", "wu", 4, true, true);
    zhiba_sunquan->addSkill(new SuperZhiheng);
    zhiba_sunquan->addSkill("jiuyuan");

    General *wuxing_zhuge = new General(this, "wuxing_zhugeliang", "shu", 3, true, true);
    wuxing_zhuge->addSkill(new SuperGuanxing);
    wuxing_zhuge->addSkill("kongcheng");

    General *super_yuanshu = new General(this, "super_yuanshu", "qun", 4, true, true, true);
    super_yuanshu->addSkill(new SuperYongsi);
    super_yuanshu->addSkill(new MarkAssignSkill("@yongsi_test", 4));
    related_skills.insertMulti("super_yongsi", "#@yongsi_test-4");
    super_yuanshu->addSkill("weidi");

    General *super_caoren = new General(this, "super_caoren", "wei", 4, true, true, true);
    super_caoren->addSkill(new SuperJushou);
    super_caoren->addSkill(new MarkAssignSkill("@jushou_test", 5));
    related_skills.insertMulti("super_jushou", "#@jushou_test-5");

    General *gd_shenzhaoyun = new General(this, "gaodayihao", "god", 1, true, true);
    gd_shenzhaoyun->addSkill(new NosJuejing);
    gd_shenzhaoyun->addSkill(new NosLonghun);
    gd_shenzhaoyun->addSkill(new NosDuojian);
    related_skills.insertMulti("noslonghun", "#noslonghun_duojian");

    General *nobenghuai_dongzhuo = new General(this, "nobenghuai_dongzhuo$", "qun", 4, true, true, true);
    nobenghuai_dongzhuo->addSkill("jiuchi");
    nobenghuai_dongzhuo->addSkill("roulin");
    nobenghuai_dongzhuo->addSkill("baonue");

    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);

    new General(this, "anjiang", "god", 4, true, true, true);

    skills << new SuperMaxCards << new SuperOffensiveDistance << new SuperDefensiveDistance;
}

ADD_PACKAGE(Test)

