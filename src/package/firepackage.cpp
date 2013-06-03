#include "firepackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "engine.h"
#include "maneuvering.h"

QuhuCard::QuhuCard() {
    mute = true;
}

bool QuhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getHp() > Self->getHp() && !to_select->isKongcheng();
}

void QuhuCard::use(Room *room, ServerPlayer *xunyu, QList<ServerPlayer *> &targets) const{
    ServerPlayer *tiger = targets.first();

    room->broadcastSkillInvoke("quhu", 1);

    bool success = xunyu->pindian(tiger, "quhu", NULL);
    if (success) {
        room->broadcastSkillInvoke("quhu", 2);

        QList<ServerPlayer *> players = room->getOtherPlayers(tiger), wolves;
        foreach (ServerPlayer *player, players) {
            if (tiger->inMyAttackRange(player))
                wolves << player;
        }

        if (wolves.isEmpty()) {
            LogMessage log;
            log.type = "#QuhuNoWolf";
            log.from = xunyu;
            log.to << tiger;
            room->sendLog(log);

            return;
        }

        room->broadcastSkillInvoke("#tunlang");
        ServerPlayer *wolf = room->askForPlayerChosen(xunyu, wolves, "quhu", QString("@quhu-damage:%1").arg(tiger->objectName()));
        room->damage(DamageStruct("quhu", tiger, wolf));
    } else {
        room->damage(DamageStruct("quhu", tiger, xunyu));
    }
}

class Jieming: public MasochismSkill {
public:
    Jieming(): MasochismSkill("jieming") {
    }

    virtual void onDamaged(ServerPlayer *xunyu, const DamageStruct &damage) const{
        Room *room = xunyu->getRoom();
        for (int i = 0; i < damage.damage; i++) {
            ServerPlayer *to = room->askForPlayerChosen(xunyu, room->getAlivePlayers(), objectName(), "jieming-invoke", true, true);
            if (!to) break;

            int upper = qMin(5, to->getMaxHp());
            int x = upper - to->getHandcardNum();
            if (x <= 0) continue;

            room->broadcastSkillInvoke(objectName());
            to->drawCards(x);
        }
    }
};

class Quhu: public ZeroCardViewAsSkill {
public:
    Quhu(): ZeroCardViewAsSkill("quhu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("QuhuCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new QuhuCard;
    }
};

QiangxiCard::QiangxiCard() {
}

bool QiangxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    int rangefix = 0;
    if (!subcards.isEmpty() && Self->getWeapon() && Self->getWeapon()->getId() == subcards.first()) {
        const Weapon *card = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        rangefix += card->getRange() - 1;
    }

    return Self->distanceTo(to_select, rangefix) <= Self->getAttackRange();
}

void QiangxiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if (subcards.isEmpty())
        room->loseHp(effect.from);

    room->damage(DamageStruct("qiangxi", effect.from, effect.to));
}

class Qiangxi: public ViewAsSkill {
public:
    Qiangxi(): ViewAsSkill("qiangxi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("QiangxiCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty() && to_select->isKindOf("Weapon") && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return new QiangxiCard;
        else if (cards.length() == 1) {
            QiangxiCard *card = new QiangxiCard;
            card->addSubcards(cards);

            return card;
        } else
            return NULL;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        return 2 - card->subcardsLength();
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

class Xueyi: public MaxCardsSkill {
public:
    Xueyi(): MaxCardsSkill("xueyi$") {
    }

    virtual int getExtra(const Player *target) const{
        int extra = 0;
        QList<const Player *> players = target->getSiblings();
        foreach (const Player *player, players) {
            if (player->isAlive() && player->getKingdom() == "qun")
                extra += 2;
        }
        if (target->hasLordSkill(objectName()))
            return extra;
        else
            return 0;
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
        events << EventPhaseStart << FinishJudge;
        view_as_skill = new ShuangxiongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *shuangxiong, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (shuangxiong->getPhase() == Player::Start) {
                room->setPlayerMark(shuangxiong, "shuangxiong", 0);
            } else if (shuangxiong->getPhase() == Player::Draw && TriggerSkill::triggerable(shuangxiong)) {
                if (shuangxiong->askForSkillInvoke(objectName())) {
                    room->setPlayerFlag(shuangxiong, "shuangxiong");

                    room->broadcastSkillInvoke("shuangxiong");
                    JudgeStruct judge;
                    judge.good = true;
                    judge.play_animation = false;
                    judge.reason = objectName();
                    judge.who = shuangxiong;

                    room->judge(judge);
                    room->setPlayerMark(shuangxiong, "shuangxiong", judge.card->isRed() ? 1 : 2);

                    return true;
                }
            }
        } else if (triggerEvent == FinishJudge) {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason == "shuangxiong")
                shuangxiong->obtainCard(judge->card);
        }

        return false;
    }
};

class Mengjin: public TriggerSkill {
public:
    Mengjin():TriggerSkill("mengjin") {
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *pangde, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.to->isAlive() && pangde->canDiscard(effect.to, "he")) {
            if (pangde->askForSkillInvoke(objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                int to_throw = room->askForCardChosen(pangde, effect.to, "he", objectName(), false, Card::MethodDiscard);
                room->throwCard(Sanguosha->getCard(to_throw), effect.to, pangde);
            }
        }

        return false;
    }
};

class Lianhuan: public OneCardViewAsSkill {
public:
    Lianhuan(): OneCardViewAsSkill("lianhuan") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped() && to_select->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        IronChain *chain = new IronChain(originalCard->getSuit(), originalCard->getNumber());
        chain->addSubcard(originalCard);
        chain->setSkillName(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill {
public:
    Niepan(): TriggerSkill("niepan") {
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *pangtong, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if (dying_data.who != pangtong)
            return false;

        if (pangtong->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$NiepanAnimate");

            room->removePlayerMark(pangtong, "@nirvana");

            pangtong->throwAllHandCardsAndEquips();
            QList<const Card *> tricks = pangtong->getJudgingArea();
            foreach (const Card *trick, tricks) {
                CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, pangtong->objectName());
                room->throwCard(trick, reason, NULL);
            }

            RecoverStruct recover;
            recover.recover = qMin(3, pangtong->getMaxHp()) - pangtong->getHp();
            room->recover(pangtong, recover);

            pangtong->drawCards(3);

            if (pangtong->isChained())
                room->setPlayerProperty(pangtong, "chained", false);

            if (!pangtong->faceUp())
                pangtong->turnOver();
        }

        return false;
    }
};

class Huoji: public OneCardViewAsSkill {
public:
    Huoji(): OneCardViewAsSkill("huoji") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped() && to_select->isRed();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        FireAttack *fire_attack = new FireAttack(originalCard->getSuit(), originalCard->getNumber());
        fire_attack->addSubcard(originalCard->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Bazhen: public TriggerSkill {
public:
    Bazhen(): TriggerSkill("bazhen") {
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasArmorEffect(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *wolong, QVariant &data) const{
        QString pattern = data.toStringList().first();

        if (pattern != "jink")
            return false;

        if (wolong->askForSkillInvoke(objectName())) {
            JudgeStruct judge;
            judge.pattern = ".|red";
            judge.good = true;
            judge.reason = objectName();
            judge.who = wolong;

            room->setEmotion(wolong, "armor/eight_diagram");
            room->judge(judge);

            if (judge.isGood()) {
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class Kanpo: public OneCardViewAsSkill {
public:
    Kanpo(): OneCardViewAsSkill("kanpo") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack() && !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return  pattern == "nullification";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        const Card *first = originalCard;
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName(objectName());
        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        foreach (const Card *card, player->getHandcards()) {
            if (card->isBlack()) return true;
        }
        return false;
    }
};

TianyiCard::TianyiCard() {
}

bool TianyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TianyiCard::use(Room *room, ServerPlayer *taishici, QList<ServerPlayer *> &targets) const{
    bool success = taishici->pindian(targets.first(), "tianyi", NULL);
    if (success)
        room->setPlayerFlag(taishici, "TianyiSuccess");
    else
        room->setPlayerCardLimitation(taishici, "use", "Slash", true);
}

class TianyiViewAsSkill: public ZeroCardViewAsSkill {
public:
    TianyiViewAsSkill(): ZeroCardViewAsSkill("tianyi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TianyiCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new TianyiCard;
    }
};

class Tianyi: public TriggerSkill {
public:
    Tianyi(): TriggerSkill("tianyi") {
        events << EventLoseSkill;
        view_as_skill = new TianyiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasFlag("TianyiSuccess");
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *taishici, QVariant &data) const{
        if (data.toString() == objectName())
            room->setPlayerFlag(taishici, "-TianyiSuccess");

        return false;
    }
};

class TianyiTargetMod: public TargetModSkill {
public:
    TianyiTargetMod(): TargetModSkill("#tianyi-target") {
        frequency = NotFrequent;
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill("tianyi") && from->hasFlag("TianyiSuccess"))
            return 1;
        else
            return 0;
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill("tianyi") && from->hasFlag("TianyiSuccess"))
            return 1000;
        else
            return 0;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *) const{
        if (from->hasSkill("tianyi") && from->hasFlag("TianyiSuccess"))
            return 1;
        else
            return 0;
    }
};

FirePackage::FirePackage()
    : Package("fire")
{
    General *dianwei = new General(this, "dianwei", "wei"); // WEI 012
    dianwei->addSkill(new Qiangxi);

    General *xunyu = new General(this, "xunyu", "wei", 3); // WEI 013
    xunyu->addSkill(new Quhu);
    xunyu->addSkill(new Jieming);

    General *pangtong = new General(this, "pangtong", "shu", 3); // SHU 010
    pangtong->addSkill(new Lianhuan);
    pangtong->addSkill(new MarkAssignSkill("@nirvana", 1));
    pangtong->addSkill(new Niepan);
    related_skills.insertMulti("niepan", "#@nirvana-1");

    General *wolong = new General(this, "wolong", "shu", 3); // SHU 011
    wolong->addSkill(new Huoji);
    wolong->addSkill(new Kanpo);
    wolong->addSkill(new Bazhen);

    General *taishici = new General(this, "taishici", "wu"); // WU 012
    taishici->addSkill(new Tianyi);
    taishici->addSkill(new TianyiTargetMod);
    related_skills.insertMulti("tianyi", "#tianyi-target");

    General *yuanshao = new General(this, "yuanshao$", "qun"); // QUN 004
    yuanshao->addSkill(new Luanji);
    yuanshao->addSkill(new Xueyi);

    General *yanliangwenchou = new General(this, "yanliangwenchou", "qun"); // QUN 005
    yanliangwenchou->addSkill(new Shuangxiong);

    General *pangde = new General(this, "pangde", "qun"); // QUN 008
    pangde->addSkill("mashu");
    pangde->addSkill(new Mengjin);
    pangde->addSkill(new SPConvertSkill("pangde", "sp_pangde"));

    addMetaObject<QuhuCard>();
    addMetaObject<QiangxiCard>();
    addMetaObject<TianyiCard>();
}

ADD_PACKAGE(Fire)

