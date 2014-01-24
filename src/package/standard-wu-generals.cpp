#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"

ZhihengCard::ZhihengCard() {
    target_fixed = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    if (source->isAlive())
        room->drawCards(source, subcards.length());
}

class Zhiheng: public ViewAsSkill {
public:
    Zhiheng(): ViewAsSkill("zhiheng") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !Self->isJilei(to_select) && selected.length() < Self->getMaxHp();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);
        zhiheng_card->setSkillName(objectName());
        zhiheng_card->setShowSkill(objectName());
        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("ZhihengCard");
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
        dismantlement->setShowSkill(objectName());
        return dismantlement;
    }
};

class KejiGlobal: public TriggerSkill{
public:
    KejiGlobal(): TriggerSkill("keji-global"){
        global = true;
        events << PreCardUsed << CardResponded;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        return player != NULL && player->isAlive();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return true;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if (triggerEvent == PreCardUsed)
            card = data.value<CardUseStruct>().card;
        else
            card = data.value<CardResponseStruct>().m_card;
        if (card->isKindOf("Slash") && player->getPhase() == Player::Play)
            player->setFlags("KejiSlashInPlayPhase");
    }
};

class Keji: public TriggerSkill {
public:
    Keji(): TriggerSkill("keji") {
        events << EventPhaseChanging;
        frequency = Frequent;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who)){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (player->hasFlag("KejiSlashInPlayPhase") && change.to == Player::Discard)
                return true;
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->askForSkillInvoke(objectName())){
            if (player->getHandcardNum() > player->getMaxCards()) {
                room->broadcastSkillInvoke(objectName());
            }
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data) const{
        lvmeng->skip(Player::Discard);
        return false;
    }
};

KurouCard::KurouCard() {
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if (source->isAlive())
        room->drawCards(source, 2);
}

class Kurou: public ZeroCardViewAsSkill {
public:
    Kurou(): ZeroCardViewAsSkill("kurou") {
    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Yingzi: public DrawCardsSkill {
public:
    Yingzi(): DrawCardsSkill("yingzi") {
        frequency = Frequent;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        return n + 1;
    }
};

FanjianCard::FanjianCard() {
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "fanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);
    room->showCard(target, card_id);

    if (card->getSuit() != suit)
        room->damage(DamageStruct("fanjian", zhouyu, target));
}

class Fanjian: public ZeroCardViewAsSkill {
public:
    Fanjian(): ZeroCardViewAsSkill("fanjian") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        FanjianCard *fj = new FanjianCard;
        fj->setShowSkill(objectName());
        return fj;
    }
};

class Guose: public OneCardViewAsSkill {
public:
    Guose(): OneCardViewAsSkill("guose") {
        filter_pattern = ".|diamond";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Indulgence *indulgence = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        indulgence->addSubcard(originalCard->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

LiuliCard::LiuliCard() {
    //mute = true;
}

bool LiuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->hasFlag("LiuliSlashSource") || to_select == Self)
        return false;

    const Player *from = NULL;
    foreach (const Player *p, Self->getAliveSiblings()) {
        if (p->hasFlag("LiuliSlashSource")) {
            from = p;
            break;
        }
    }

    const Card *slash = Card::Parse(Self->property("liuli").toString());
    if (from && !from->canSlash(to_select, slash, false))
        return false;

    int card_id = subcards.first();
    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getId() == card_id) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - Self->getAttackRange(false);
    } else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getId() == card_id) {
        range_fix += 1;
    }

    return Self->distanceTo(to_select, range_fix) <= Self->getAttackRange();
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("LiuliTarget");
}

class LiuliViewAsSkill: public OneCardViewAsSkill {
public:
    LiuliViewAsSkill(): OneCardViewAsSkill("liuli") {
        filter_pattern = ".!";
        response_pattern = "@@liuli";
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *daqiao, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isKindOf("Slash") && use.to.contains(daqiao) && daqiao->canDiscard(daqiao, "he")) {
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(use.from);

            bool can_invoke = false;
            foreach (ServerPlayer *p, players) {
                if (use.from->canSlash(p, use.card, false) && daqiao->inMyAttackRange(p)) {
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

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        if (player->hasSkill("luoyan"))
            return qrand() % 2 + 3;
        else
            return qrand() % 2 + 1;
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

class Lianying: public TriggerSkill {
public:
    Lianying(): TriggerSkill("lianying") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *luxun, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == luxun && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard) {
            if (room->askForSkillInvoke(luxun, objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                luxun->drawCards(1);
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
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

JieyinCard::JieyinCard() {
}

bool JieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    return to_select->isMale() && to_select->isWounded() && to_select != Self;
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &data) const{
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

void StandardPackage::addWuGenerals()
{
    General *sunquan = new General(this, "sunquan", "wu"); // WU 001
    sunquan->addCompanion("zhoutai");
    sunquan->addSkill(new Zhiheng);

    General *ganning = new General(this, "ganning", "wu"); // WU 002
    ganning->addSkill(new Qixi);

    General *lvmeng = new General(this, "lvmeng", "wu"); // WU 003
    lvmeng->addSkill(new Keji);

    General *huanggai = new General(this, "huanggai", "wu"); // WU 004
    huanggai->addSkill(new Kurou);

    General *zhouyu = new General(this, "zhouyu", "wu", 3); // WU 005
    zhouyu->addCompanion("huanggai");
    zhouyu->addCompanion("xiaoqiao");
    zhouyu->addSkill(new Yingzi);
    zhouyu->addSkill(new Fanjian);

    General *daqiao = new General(this, "daqiao", "wu", 3, false); // WU 006
    daqiao->addCompanion("xiaoqiao");
    daqiao->addSkill(new Guose);
    daqiao->addSkill(new Liuli);

    General *luxun = new General(this, "luxun", "wu", 3); // WU 007
    luxun->addSkill(new Qianxun);
    luxun->addSkill(new Lianying);
    luxun->addSkill(new LianyingForZeroMaxCards);
    related_skills.insertMulti("lianying", "#lianying-for-zero-maxcards");

    General *sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false); // WU 008
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    addMetaObject<ZhihengCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<KurouCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<LiuliCard>();
}