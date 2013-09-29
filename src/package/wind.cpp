#include "settings.h"
#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "ai.h"
#include "general.h"

class Guidao: public TriggerSkill {
public:
    Guidao(): TriggerSkill("guidao") {
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
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

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                    << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, ".|black", prompt, data, Card::MethodResponse, judge->who, true);

        if (card != NULL) {
            room->broadcastSkillInvoke(objectName());
            room->retrial(card, player, judge, objectName(), true);
        }
        return false;
    }
};

class Leiji: public TriggerSkill {
public:
    Leiji(): TriggerSkill("leiji") {
        events << CardResponded;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardResponseStruct>().m_card;
        if (card_star->isKindOf("Jink")) {
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao, room->getAlivePlayers(), objectName(), "leiji-invoke", true, true);
            if (target) {
                room->broadcastSkillInvoke(objectName());

                JudgeStruct judge;
                judge.pattern = ".|black";
                judge.good = false;
                judge.negative = true;
                judge.reason = objectName();
                judge.who = target;

                room->judge(judge);

                if (judge.isBad()) {
                    room->damage(DamageStruct(objectName(), zhangjiao, target, 1, DamageStruct::Thunder));
                    if (zhangjiao->isAlive()) {
                        RecoverStruct recover;
                        recover.who = zhangjiao;
                        room->recover(zhangjiao, recover);
                    }
                }
            }
        }
        return false;
    }
};

HuangtianCard::HuangtianCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
    m_skillName = "huangtianv";
    mute = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if (zhangjiao->hasLordSkill("huangtian")) {
        room->setPlayerFlag(zhangjiao, "HuangtianInvoked");

        if (!zhangjiao->isLord() && zhangjiao->hasSkill("weidi"))
            room->broadcastSkillInvoke("weidi");
        else
            room->broadcastSkillInvoke("huangtian");

        room->notifySkillInvoked(zhangjiao, "huangtian");
        zhangjiao->obtainCard(this);
        QList<ServerPlayer *> zhangjiaos;
        QList<ServerPlayer *> players = room->getOtherPlayers(source);
        foreach (ServerPlayer *p, players) {
            if (p->hasLordSkill("huangtian") && !p->hasFlag("HuangtianInvoked"))
                zhangjiaos << p;
        }
        if (zhangjiaos.empty())
            room->setPlayerFlag(source, "ForbidHuangtian");
    }
}

bool HuangtianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian")
           && to_select != Self && !to_select->hasFlag("HuangtianInvoked");
}

class HuangtianViewAsSkill: public OneCardViewAsSkill {
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv") {
        attached_lord_skill = true;
        filter_pattern = "Jink,Lightning";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getKingdom() == "qun" && !player->hasFlag("ForbidHuangtian");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(originalCard);

        return card;
    }
};

class Huangtian: public TriggerSkill {
public:
    Huangtian(): TriggerSkill("huangtian$") {
        events << GameStart << EventAcquireSkill << EventLoseSkill << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if ((triggerEvent == GameStart && player->isLord())
            || (triggerEvent == EventAcquireSkill && data.toString() == "huangtian")) {
            QList<ServerPlayer *> lords;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasLordSkill(objectName()))
                    lords << p;
            }
            if (lords.isEmpty()) return false;

            QList<ServerPlayer *> players;
            if (lords.length() > 1)
                players = room->getAlivePlayers();
            else
                players = room->getOtherPlayers(lords.first());
            foreach (ServerPlayer *p, players) {
                if (!p->hasSkill("huangtianv"))
                    room->attachSkillToPlayer(p, "huangtianv");
            }
        } else if (triggerEvent == EventLoseSkill && data.toString() == "huangtian") {
            QList<ServerPlayer *> lords;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasLordSkill(objectName()))
                    lords << p;
            }
            if (lords.length() > 2) return false;

            QList<ServerPlayer *> players;
            if (lords.isEmpty())
                players = room->getAlivePlayers();
            else
                players << lords.first();
            foreach (ServerPlayer *p, players) {
                if (p->hasSkill("huangtianv"))
                    room->detachSkillFromPlayer(p, "huangtianv", true);
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();
            if (phase_change.from != Player::Play)
                  return false;
            if (player->hasFlag("ForbidHuangtian"))
                room->setPlayerFlag(player, "-ForbidHuangtian");
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach (ServerPlayer *p, players) {
                if (p->hasFlag("HuangtianInvoked"))
                    room->setPlayerFlag(p, "-HuangtianInvoked");
            }
        }
        return false;
    }
};

ShensuCard::ShensuCard() {
    mute = true;
}

bool ShensuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("shensu");
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

void ShensuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *target, targets) {
        if (!source->canSlash(target, NULL, false))
            targets.removeOne(target);
    }

    if (targets.length() > 0) {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_shensu");
        room->useCard(CardUseStruct(slash, source, targets));
    }
}

class ShensuViewAsSkill: public ViewAsSkill {
public:
    ShensuViewAsSkill(): ViewAsSkill("shensu") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern.startsWith("@@shensu");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern().endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->isKindOf("EquipCard") && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern().endsWith("1")) {
            return cards.isEmpty() ? new ShensuCard : NULL;
        } else {
            if (cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public TriggerSkill {
public:
    Shensu(): TriggerSkill("shensu") {
        events << EventPhaseChanging;
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *xiahouyuan, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Judge && !xiahouyuan->isSkipped(Player::Judge)
            && !xiahouyuan->isSkipped(Player::Draw)) {
            if (Slash::IsAvailable(xiahouyuan) && room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1", 1)) {
                xiahouyuan->skip(Player::Judge);
                xiahouyuan->skip(Player::Draw);
            }
        } else if (Slash::IsAvailable(xiahouyuan) && change.to == Player::Play && !xiahouyuan->isSkipped(Player::Play)) {
            if (xiahouyuan->canDiscard(xiahouyuan, "he") && room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2", 2, Card::MethodDiscard))
                xiahouyuan->skip(Player::Play);
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        int index = qrand() % 2 + 1;
        if (!player->hasInnateSkill(objectName()) && player->hasSkill("baobian"))
            index += 2;
        return index;
    }
};

Jushou::Jushou(): PhaseChangeSkill("jushou") {
}

int Jushou::getJushouDrawNum(ServerPlayer *) const{
    return 1;
}

bool Jushou::onPhaseChange(ServerPlayer *target) const{
    if (target->getPhase() == Player::Finish) {
        Room *room = target->getRoom();
        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName() == "neojushou" ? "neojushou" : "jushou");
            target->drawCards(getJushouDrawNum(target));
            target->turnOver();
        }
    }

    return false;
}

class Jiewei: public TriggerSkill {
public:
    Jiewei(): TriggerSkill("jiewei") {
        events << TurnedOver;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (!room->askForSkillInvoke(player, objectName())) return false;
        player->drawCards(1);

        const Card *card = room->askForUseCard(player, "TrickCard+^Nullification,EquipCard|.|.|hand", "@jiewei");
        if (!card) return false;

        QList<ServerPlayer *> targets;
        if (card->getTypeId() == Card::TypeTrick) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                bool can_discard = false;
                foreach (const Card *judge, p->getJudgingArea()) {
                    if (judge->getTypeId() == Card::TypeTrick && player->canDiscard(p, judge->getEffectiveId())) {
                        can_discard = true;
                        break;
                    } else if (judge->getTypeId() == Card::TypeSkill) {
                        const Card *real_card = Sanguosha->getEngineCard(judge->getEffectiveId());
                        if (real_card->getTypeId() == Card::TypeTrick && player->canDiscard(p, real_card->getEffectiveId())) {
                            can_discard = true;
                            break;
                        }
                    }
                 }
                 if (can_discard) targets << p;
            }
        } else if (card->getTypeId() == Card::TypeEquip) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (!p->getEquips().isEmpty() && player->canDiscard(p, "e"))
                    targets << p;
                else {
                    foreach (const Card *judge, p->getJudgingArea()) {
                        if (judge->getTypeId() == Card::TypeSkill) {
                            const Card *real_card = Sanguosha->getEngineCard(judge->getEffectiveId());
                            if (real_card->getTypeId() == Card::TypeEquip && player->canDiscard(p, real_card->getEffectiveId())) {
                                targets << p;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (targets.isEmpty()) return false;
        ServerPlayer *to_discard = room->askForPlayerChosen(player, targets, objectName(), "@jiewei-discard", true);
        if (to_discard) {
            QList<int> disabled_ids;
            foreach (const Card *c, to_discard->getCards("ej")) {
                const Card *pcard = c;
                if (pcard->getTypeId() == Card::TypeSkill)
                    pcard = Sanguosha->getEngineCard(c->getEffectiveId());
                if (pcard->getTypeId() != card->getTypeId())
                    disabled_ids << pcard->getEffectiveId();
            }
            int id = room->askForCardChosen(player, to_discard, "ej", objectName(), false, Card::MethodDiscard, disabled_ids);
            room->throwCard(id, to_discard, player);
        }
        return false;
    }
};

class Liegong: public TriggerSkill {
public:
    Liegong(): TriggerSkill("liegong") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (player != use.from || player->getPhase() != Player::Play || !use.card->isKindOf("Slash"))
            return false;
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        int index = 0;
        foreach (ServerPlayer *p, use.to) {
            int handcardnum = p->getHandcardNum();
            if ((player->getHp() <= handcardnum || player->getAttackRange() >= handcardnum)
                && player->askForSkillInvoke(objectName(), QVariant::fromValue(p))) {
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#NoJink";
                log.from = p;
                room->sendLog(log);
                jink_list.replace(index, QVariant(0));
            }
            index++;
        }
        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }
};

class Kuanggu: public TriggerSkill {
public:
    Kuanggu(): TriggerSkill("kuanggu") {
        frequency = Compulsory;
        events << Damage << PreDamageDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (triggerEvent == PreDamageDone && damage.from && damage.from->hasSkill("kuanggu") && damage.from->isAlive()) {
            ServerPlayer *weiyan = damage.from;
            weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;
        } else if (triggerEvent == Damage && player->hasSkill("kuanggu") && player->isAlive()) {
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            player->tag["InvokeKuanggu"] = false;
            if (invoke && player->isWounded()) {
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(player, objectName());

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class Buqu: public TriggerSkill {
public:
    Buqu(): TriggerSkill("buqu") {
        events << AskForPeaches;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if (dying_data.who != zhoutai)
            return false;

        if (zhoutai->getHp() > 0) return false;
        room->broadcastSkillInvoke(objectName());
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = zhoutai;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(zhoutai, objectName());

        int id = room->drawCard();
        int num = Sanguosha->getCard(id)->getNumber();
        bool duplicate = false;
        foreach (int card_id, zhoutai->getPile("buqu")) {
            if (Sanguosha->getCard(card_id)->getNumber() == num) {
                duplicate = true;
                break;
            }
        }
        zhoutai->addToPile("buqu", id);
        if (duplicate) {
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
            room->throwCard(Sanguosha->getCard(id), reason, NULL);
        } else {
            RecoverStruct recover;
            recover.who = zhoutai;
            recover.recover = 1 - zhoutai->getHp();
            room->recover(zhoutai, recover);
        }
        return false;
    }
};

class BuquMaxCards: public MaxCardsSkill {
public:
    BuquMaxCards(): MaxCardsSkill("#buqu") {
    }

    virtual int getFixed(const Player *target) const{
        int len = target->getPile("buqu").length();
        if (len > 0)
            return len;
        else
            return -1;
    }
};

class Fenji: public TriggerSkill {
public:
    Fenji(): TriggerSkill("fenji") {
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from && move.from->isAlive() && move.from_places.contains(Player::PlaceHand)
            && ((move.reason.m_reason == CardMoveReason::S_REASON_DISMANTLE
                 && move.reason.m_playerId != move.reason.m_targetId)
                || ((move.to_place == Player::PlaceTable && move.origin_to && move.origin_to != move.from && move.origin_to_place == Player::PlaceHand)
                    || (move.to && move.to != move.from && move.to_place == Player::PlaceHand)))) {
            if (room->askForSkillInvoke(player, objectName(), data)) {
                room->loseHp(player);
                if (move.from->isAlive())
                    room->drawCards((ServerPlayer *)move.from, 2);
            }
        }
        return false;
    }
};

class Hongyan: public FilterSkill {
public:
    Hongyan(): FilterSkill("hongyan") {
    }

    static WrappedCard *changeToHeart(int cardId) {
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("hongyan");
        new_card->setSuit(Card::Heart);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToHeart(originalCard->getEffectiveId());
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

TianxiangCard::TianxiangCard() {
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    effect.to->addMark("TianxiangTarget");
    DamageStruct damage = effect.from->tag.value("TianxiangDamage").value<DamageStruct>();

    if (damage.card && damage.card->isKindOf("Slash"))
        effect.from->removeQinggangTag(damage.card);

    damage.to = effect.to;
    damage.transfer = true;
    try {
        room->damage(damage);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange)
            effect.to->removeMark("TianxiangTarget");
        throw triggerEvent;
    }
}

class TianxiangViewAsSkill: public OneCardViewAsSkill {
public:
    TianxiangViewAsSkill(): OneCardViewAsSkill("tianxiang") {
        filter_pattern = ".|heart|.|hand!";
        response_pattern = "@@tianxiang";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TianxiangCard *tianxiangCard = new TianxiangCard;
        tianxiangCard->addSubcard(originalCard);
        return tianxiangCard;
    }
};

class Tianxiang: public TriggerSkill {
public:
    Tianxiang(): TriggerSkill("tianxiang") {
        events << DamageInflicted;
        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *xiaoqiao, QVariant &data) const{
        if (xiaoqiao->canDiscard(xiaoqiao, "h")) {
            DamageStruct damage = data.value<DamageStruct>();
            xiaoqiao->tag["TianxiangDamage"] = QVariant::fromValue(damage);
            return room->askForUseCard(xiaoqiao, "@@tianxiang", "@tianxiang-card", -1, Card::MethodDiscard);
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

class TianxiangDraw: public TriggerSkill {
public:
    TianxiangDraw(): TriggerSkill("#tianxiang") {
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (player->isAlive() && player->getMark("TianxiangTarget") > 0 && damage.transfer) {
            player->drawCards(player->getLostHp());
            player->removeMark("TianxiangTarget");
        }

        return false;
    }
};

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

GuhuoDialog *GuhuoDialog::getInstance(const QString &object, bool left, bool right) {
    static GuhuoDialog *instance;
    if (instance == NULL || instance->objectName() != object)
        instance = new GuhuoDialog(object, left, right);

    return instance;
}

GuhuoDialog::GuhuoDialog(const QString &object, bool left, bool right): object_name(object) {
    setObjectName(object);
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if (left) layout->addWidget(createLeft());
    if (right) layout->addWidget(createRight());
    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(selectCard(QAbstractButton *)));
}

void GuhuoDialog::popup() {
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
        return;

    foreach (QAbstractButton *button, group->buttons()) {
        const Card *card = map[button->objectName()];
        bool enabled = !Self->isCardLimited(card, Card::MethodUse, true) && card->isAvailable(Self);
        button->setEnabled(enabled);
    }

    Self->tag.remove(object_name);
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button) {
    CardStar card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    if (button->objectName().contains("slash")) {
        if (objectName() == "guhuo")
            Self->tag["GuhuoSlash"] = button->objectName();
        else if (objectName() == "nosguhuo")
            Self->tag["NosGuhuoSlash"] = button->objectName();
    }
    emit onButtonClick();
    accept();
}

QGroupBox *GuhuoDialog::createLeft() {
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->getTypeId() == Card::TypeBasic && !map.contains(card->objectName())
            && !Config.BanPackages.contains(card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setParent(this);
            layout->addWidget(createButton(c));

            if (card->objectName() == "slash" && !Config.BanPackages.contains("maneuvering") && objectName() != "gudan") {
                Card *c2 = Sanguosha->cloneCard(card->objectName());
                c2->setParent(this);
                layout->addWidget(createButton(c2));
            }
        }
    }

    layout->addStretch();
    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight() {
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target_trick"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_target_trick"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->isNDTrick() && !map.contains(card->objectName())
            && !Config.BanPackages.contains(card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->isKindOf("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *GuhuoDialog::createButton(const Card *card) {
    if (card->objectName() == "slash" && map.contains(card->objectName()) 
        && !map.contains("normal_slash") && objectName() != "gudan") {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate("normal_slash"));
        button->setObjectName("normal_slash");
        button->setToolTip(card->getDescription());

        map.insert("normal_slash", card);
        group->addButton(button);

        return button;
    } else {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
        button->setObjectName(card->objectName());
        button->setToolTip(card->getDescription());

        map.insert(card->objectName(), card);
        group->addButton(button);

        return button;
    }
}

GuhuoCard::GuhuoCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool GuhuoCard::guhuo(ServerPlayer *yuji) const{
    Room *room = yuji->getRoom();
    QList<ServerPlayer *> players = room->getOtherPlayers(yuji);

    QList<int> used_cards;
    QList<CardsMoveStruct> moves;
    foreach (int card_id, getSubcards())
        used_cards << card_id;
    room->setTag("GuhuoType", user_string);

    ServerPlayer *questioned = NULL;
    foreach (ServerPlayer *player, players) {
        if (player->hasSkill("chanyuan")) {
            LogMessage log;
            log.type = "#Chanyuan";
            log.from = player;
            log.arg = "chanyuan";
            room->sendLog(log);

            room->notifySkillInvoked(player, "chanyuan");
            room->setEmotion(player, "no-question");
            continue;
        }

        QString choice = room->askForChoice(player, "guhuo", "noquestion+question");
        if (choice == "question")
            room->setEmotion(player, "question");
        else
            room->setEmotion(player, "no-question");

        LogMessage log;
        log.type = "#GuhuoQuery";
        log.from = player;
        log.arg = choice;
        room->sendLog(log);
        if (choice == "question") {
            questioned = player;
            break;
        }
    }

    LogMessage log;
    log.type = "$GuhuoResult";
    log.from = yuji;
    log.card_str = QString::number(subcards.first());
    room->sendLog(log);

    bool success = false;
    if (!questioned) {
        success = true;
        foreach (ServerPlayer *player, players)
            room->setEmotion(player, ".");

        CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "guhuo");
        CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceUnknown, Player::PlaceTable, reason);
        moves.append(move);
        room->moveCardsAtomic(moves, true);
    } else {
        const Card *card = Sanguosha->getCard(subcards.first());
        if (user_string == "peach+analeptic")
            success = card->objectName() == yuji->tag["GuhuoSaveSelf"].toString();
        else if (user_string == "slash")
            success = card->objectName().contains("slash");
        else if (user_string == "normal_slash")
            success = card->objectName() == "slash";
        else
            success = card->match(user_string);

        if (success) {
            CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "guhuo");
            CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceUnknown, Player::PlaceTable, reason);
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        } else {
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                             CardMoveReason(CardMoveReason::S_REASON_PUT, yuji->objectName(), QString(), "guhuo"), true);
        }
        foreach (ServerPlayer *player, players) {
            room->setEmotion(player, ".");
            if (success && questioned == player)
                room->acquireSkill(questioned, "chanyuan");
        }
    }
    yuji->tag.remove("GuhuoSaveSelf");
    yuji->tag.remove("GuhuoSlash");
    room->setPlayerFlag(yuji, "GuhuoUsed");
    return success;
}

bool GuhuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return false;
    }

    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool GuhuoCard::targetFixed() const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFixed();
}

bool GuhuoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *GuhuoCard::validate(CardUseStruct &card_use) const{
    ServerPlayer *yuji = card_use.from;
    Room *room = yuji->getRoom();

    QString to_guhuo = user_string;
    if (user_string == "slash"
        && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(yuji, "guhuo_slash", guhuo_list.join("+"));
        yuji->tag["GuhuoSlash"] = QVariant(to_guhuo);
    }
    room->broadcastSkillInvoke("guhuo");

    LogMessage log;
    log.type = card_use.to.isEmpty() ? "#GuhuoNoTarget" : "#Guhuo";
    log.from = yuji;
    log.to = card_use.to;
    log.arg = to_guhuo;
    log.arg2 = "guhuo";

    room->sendLog(log);

    if (guhuo(card_use.from)) {
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_guhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_guhuo == "normal_slash")
            user_str = "slash";
        else
            user_str = to_guhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->addSubcard(subcards.first());
        use_card->deleteLater();

        QList<ServerPlayer *> tos = card_use.to;
        foreach (ServerPlayer *to, tos) {
            const ProhibitSkill *skill = room->isProhibited(card_use.from, to, use_card);
            if (skill) {
                LogMessage log;
                log.type = "#SkillAvoid";
                log.from = to;
                log.arg = skill->objectName();
                log.arg2 = use_card->objectName();
                room->sendLog(log);

                room->broadcastSkillInvoke(skill->objectName());
                card_use.to.removeOne(to);
            }
        }
        return use_card;
    } else
        return NULL;
}

const Card *GuhuoCard::validateInResponse(ServerPlayer *yuji) const{
    Room *room = yuji->getRoom();
    room->broadcastSkillInvoke("guhuo");

    QString to_guhuo;
    if (user_string == "peach+analeptic") {
        QStringList guhuo_list;
        guhuo_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "analeptic";
        to_guhuo = room->askForChoice(yuji, "guhuo_saveself", guhuo_list.join("+"));
        yuji->tag["GuhuoSaveSelf"] = QVariant(to_guhuo);
    } else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(yuji, "guhuo_slash", guhuo_list.join("+"));
        yuji->tag["GuhuoSlash"] = QVariant(to_guhuo);
    }
    else
        to_guhuo = user_string;

    LogMessage log;
    log.type = "#GuhuoNoTarget";
    log.from = yuji;
    log.arg = to_guhuo;
    log.arg2 = "guhuo";
    room->sendLog(log);

    if (guhuo(yuji)) {
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_guhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_guhuo == "normal_slash")
            user_str = "slash";
        else
            user_str = to_guhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->addSubcard(subcards.first());
        use_card->deleteLater();
        return use_card;
    } else
        return NULL;
}

class Guhuo: public OneCardViewAsSkill {
public:
    Guhuo(): OneCardViewAsSkill("guhuo") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        bool current = false;
        QList<const Player *> players = player->getAliveSiblings();
        players.append(player);
        foreach (const Player *p, players) {
            if (p->getPhase() != Player::NotActive) {
                current = true;
                break;
            }
        }
        if (!current) return false;

        if (player->isKongcheng() || player->hasFlag("GuhuoUsed")
            || pattern.startsWith(".") || pattern.startsWith("@"))
            return false;
        if (pattern == "peach" && player->hasFlag("Global_PreventPeach")) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern[i];
            if (ch.isUpper() || ch.isDigit()) return false; // This is an extremely dirty hack!! For we need to prevent patterns like 'BasicCard'
        }
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        bool current = false;
        QList<const Player *> players = player->getAliveSiblings();
        players.append(player);
        foreach (const Player *p, players) {
            if (p->getPhase() != Player::NotActive) {
                current = true;
                break;
            }
        }
        if (!current) return false;
        return !player->isKongcheng() && !player->hasFlag("GuhuoUsed");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
            || Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(Sanguosha->currentRoomState()->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        CardStar c = Self->tag.value("guhuo").value<CardStar>();
        if (c) {
            GuhuoCard *card = new GuhuoCard;
            if (!c->objectName().contains("slash"))
                card->setUserString(c->objectName());
            else
                card->setUserString(Self->tag["GuhuoSlash"].toString());
            card->addSubcard(originalCard);
            return card;
        } else
            return NULL;
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("guhuo");
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (!card->isKindOf("GuhuoCard"))
            return -2;
        else
            return -1;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        ServerPlayer *current = player->getRoom()->getCurrent();
        if (!current || current->isDead() || current->getPhase() == Player::NotActive) return false;
        return !player->isKongcheng() && !player->hasFlag("GuhuoUsed");
    }
};

class GuhuoClear: public TriggerSkill {
public:
    GuhuoClear(): TriggerSkill("#guhuo-clear") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::NotActive) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("GuhuoUsed"))
                    room->setPlayerFlag(p, "-GuhuoUsed");
            }
        }
        return false;
    }
};


class Chanyuan: public TriggerSkill {
public:
    Chanyuan(): TriggerSkill("chanyuan") {
        events << GameStart << HpChanged << MaxHpChanged << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 5;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventLoseSkill) {
            if (data.toString() == objectName()) return false;
            room->removePlayerMark(player, "@chanyuan");
        } else if (triggerEvent == EventAcquireSkill) {
            if (data.toString() != objectName()) return false;
            room->addPlayerMark(player, "@chanyuan");
        }
        if (triggerEvent != EventLoseSkill && !player->hasSkill(objectName())) return false;

        foreach (ServerPlayer *p, room->getOtherPlayers(player))
            room->filterCards(p, p->getCards("he"), true);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
        return false;
    }

    virtual Location getLocation() const{
        return Right;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    General *xiahouyuan = new General(this, "xiahouyuan", "wei"); // WEI 008
    xiahouyuan->addSkill(new Shensu);
    xiahouyuan->addSkill(new SlashNoDistanceLimitSkill("shensu"));
    related_skills.insertMulti("shensu", "#shensu-slash-ndl");

    General *caoren = new General(this, "caoren", "wei"); // WEI 011
    caoren->addSkill(new Jushou);
    caoren->addSkill(new Jiewei);

    General *huangzhong = new General(this, "huangzhong", "shu"); // SHU 008
    huangzhong->addSkill(new Liegong);

    General *weiyan = new General(this, "weiyan", "shu"); // SHU 009
    weiyan->addSkill(new Kuanggu);

    General *xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false); // WU 011
    xiaoqiao->addSkill(new Tianxiang);
    xiaoqiao->addSkill(new TianxiangDraw);
    xiaoqiao->addSkill(new Hongyan);
    related_skills.insertMulti("tianxiang", "#tianxiang");

    General *zhoutai = new General(this, "zhoutai", "wu"); // WU 013
    zhoutai->addSkill(new Buqu);
    zhoutai->addSkill(new BuquMaxCards);
    zhoutai->addSkill(new Fenji);
    related_skills.insertMulti("buqu", "#buqu");

    General *zhangjiao = new General(this, "zhangjiao$", "qun", 3); // QUN 010
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Huangtian);

    General *yuji = new General(this, "yuji", "qun", 3); // QUN 011
    yuji->addSkill(new Guhuo);
    yuji->addSkill(new GuhuoClear);
    related_skills.insertMulti("guhuo", "#guhuo-clear");

    addMetaObject<ShensuCard>();
    addMetaObject<TianxiangCard>();
    addMetaObject<HuangtianCard>();
    addMetaObject<GuhuoCard>();

    skills << new HuangtianViewAsSkill << new Chanyuan;
}

ADD_PACKAGE(Wind)