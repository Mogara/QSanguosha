#include "hegemony.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"
#include "standard-skillcards.h"

class Xiaoguo: public TriggerSkill {
public:
    Xiaoguo(): TriggerSkill("xiaoguo") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() != Player::Finish)
            return false;
        ServerPlayer *yuejin = room->findPlayerBySkillName(objectName());
        if (!yuejin || yuejin == player)
            return false;
        if (!yuejin->isKongcheng() && room->askForCard(yuejin, ".Basic", "@xiaoguo", QVariant(), objectName())) {
            room->broadcastSkillInvoke(objectName(), 1);
            if (!room->askForCard(player, ".Equip", "@xiaoguo-discard", QVariant())) {
                room->broadcastSkillInvoke(objectName(), 2);
                DamageStruct damage;
                damage.card = NULL;
                damage.from = yuejin;
                damage.to = player;
                room->damage(damage);
            } else
                room->broadcastSkillInvoke(objectName(), 3);
        }
        return false;
    }
};

ShushenCard::ShushenCard() {
}

bool ShushenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void ShushenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->broadcastSkillInvoke("shushen", effect.to->getGeneralName().contains("liubei") ? 2 : 1);
    effect.to->drawCards(1);
}

class ShushenViewAsSkill: public ZeroCardViewAsSkill {
public:
    ShushenViewAsSkill(): ZeroCardViewAsSkill("shushen") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@shushen";
    }

    virtual const Card *viewAs() const{
        return new ShushenCard;
    }
};

class Shushen: public TriggerSkill {
public:
    Shushen(): TriggerSkill("shushen") {
        events << HpRecover;
        view_as_skill = new ShushenViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover_struct = data.value<RecoverStruct>();
        int recover = recover_struct.recover;
        for (int i = 0; i < recover; i++) {
            if (!room->askForUseCard(player, "@@shushen", "@shushen-draw"))
                break;
        }
        return false;
    }
};

class Shenzhi: public PhaseChangeSkill {
public:
    Shenzhi(): PhaseChangeSkill("shenzhi") {
    }

    virtual bool onPhaseChange(ServerPlayer *ganfuren) const{
        Room *room = ganfuren->getRoom();
        if (ganfuren->getPhase() != Player::Start || ganfuren->isKongcheng())
            return false;
        // As the cost, if one of her handcards cannot be throwed, the skill is unable to invoke
        foreach (const Card *card, ganfuren->getHandcards()) {
            if (ganfuren->isJilei(card))
                return false;
        }
        //==================================
        if (room->askForSkillInvoke(ganfuren,objectName())) {
            int handcard_num = ganfuren->getHandcardNum();
            room->broadcastSkillInvoke(objectName());
            ganfuren->throwAllHandCards();
            if (handcard_num >= ganfuren->getHp()) {
                RecoverStruct recover;
                recover.who = ganfuren;
                room->recover(ganfuren, recover);
            }
        }
        return false;
    }
};

DuoshiCard::DuoshiCard() {
    mute = true;
}

bool DuoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

bool DuoshiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return true;
}

void DuoshiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (!use.to.contains(use.from))
        use.to << use.from;
    if (use.to.length() == 1)
        use.from->getRoom()->broadcastSkillInvoke("duoshi", 1);
    else
        use.from->getRoom()->broadcastSkillInvoke("duoshi", 2);
    SkillCard::onUse(room, use);
}

void DuoshiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(2);
    room->askForDiscard(effect.to, "duoshi", 2, 2, false, true);
}

class Duoshi: public OneCardViewAsSkill {
public:
    Duoshi(): OneCardViewAsSkill("duoshi") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isRed() && !to_select->isEquipped() && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        DuoshiCard *await = new DuoshiCard;
        await->addSubcard(originalcard->getId());
        await->setSkillName(objectName());
        return await;
    }
};

FenxunCard::FenxunCard() {
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FenxunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->tag["FenxunTarget"] = QVariant::fromValue(effect.to);
    room->setFixedDistance(effect.from, effect.to, 1);
}

class FenxunViewAsSkill: public OneCardViewAsSkill {
public:
    FenxunViewAsSkill(): OneCardViewAsSkill("fenxun") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FenxunCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        FenxunCard *first = new FenxunCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Fenxun: public TriggerSkill {
public:
    Fenxun(): TriggerSkill("fenxun") {
        events << EventPhaseChanging << Death << EventLoseSkill;
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->tag["FenxunTarget"].value<PlayerStar>() != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *dingfeng, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        }
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != dingfeng)
                return false;
        }
        ServerPlayer *target = dingfeng->tag["FenxunTarget"].value<PlayerStar>();

        if (target) {
            room->setFixedDistance(dingfeng, target, -1);
            dingfeng->tag.remove("FenxunTarget");
        }
        return false;
    }
};

class Mingshi: public TriggerSkill {
public:
    Mingshi(): TriggerSkill("mingshi") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from) {
            room->broadcastSkillInvoke(objectName());
            QString choice;
            if (!damage.from->isKongcheng())
                choice = room->askForChoice(damage.from, objectName(), "yes+no", data);
            else
                choice = "yes";
            if (choice == "no") {
                LogMessage log;
                log.type = "#Mingshi";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(--damage.damage);
                room->sendLog(log);

                if (damage.damage < 1)
                    return true;
                data = QVariant::fromValue(damage);
            } else {
                room->showAllCards(damage.from);
            }
        }
        return false;
    }
};

class Lirang: public TriggerSkill {
public:
    Lirang(): TriggerSkill("lirang") {
        events << CardsMoving;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *kongrong, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if (move->from != kongrong)
            return false;
        if (move->to_place == Player::DiscardPile
            && ((move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)) {

            int i = 0;
            QList<int> lirang_card;
            foreach (int card_id, move->card_ids) {
                if (room->getCardPlace(card_id) == Player::DiscardPile
                    && (move->from_places[i] == Player::PlaceHand || move->from_places[i] == Player::PlaceEquip)) {
                        lirang_card << card_id;
                }
                i++;
            }
            if (lirang_card.isEmpty())
                return false;
            if (!kongrong->askForSkillInvoke(objectName(), data))
                return false;

            room->broadcastSkillInvoke(objectName());
            room->setPlayerFlag(kongrong, "lirang_InTempMoving");

            CardsMoveStruct move2;
            move2.card_ids = lirang_card;
            move2.to_place = Player::PlaceHand;
            move2.to = kongrong;
            room->moveCardsAtomic(move2, true);

            while (room->askForYiji(kongrong, lirang_card, false, true)) {}

            CardsMoveStruct move3;
            move3.card_ids = lirang_card;
            move3.to_place = Player::DiscardPile;
            move3.reason = move->reason;
            room->moveCardsAtomic(move3, true);

            room->setPlayerFlag(kongrong, "-lirang_InTempMoving");
        }
        return false;
    }
};

SijianCard::SijianCard() {
    mute = true;
}

void SijianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->broadcastSkillInvoke("sijian", card_use.to.first()->isLord() ? 2 : 1);
    SkillCard::onUse(room, card_use);
}

bool SijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isNude() && to_select != Self;
}

void SijianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", objectName());
    room->throwCard(card_id, effect.to, effect.from);
}

class SijianViewAsSkill: public ZeroCardViewAsSkill {
public:
    SijianViewAsSkill(): ZeroCardViewAsSkill("sijian") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@sijian";
    }

    virtual const Card *viewAs() const{
        return new SijianCard;
    }
};

class Sijian: public TriggerSkill {
public:
    Sijian(): TriggerSkill("sijian") {
        events << BeforeCardsMove << CardsMoveOneTime;
        view_as_skill = new SijianViewAsSkill;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *tianfeng, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if (move->from == tianfeng && move->from_places.contains(Player::PlaceHand)) {
            if (triggerEvent == BeforeCardsMove) {
                foreach (int id, tianfeng->handCards()) {
                    if (!move->card_ids.contains(id))
                        return false;
                }
                tianfeng->addMark(objectName());
            } else {
                if (tianfeng->getMark(objectName()) == 0)
                    return false;
                tianfeng->removeMark(objectName());
                bool can_invoke = false;
                QList<ServerPlayer *> other_players = room->getOtherPlayers(tianfeng);
                foreach (ServerPlayer *p, other_players) {
                    if (!p->isNude()) {
                        can_invoke = true;
                        break;
                    }
                }
                if (!can_invoke) return false;
                room->askForUseCard(tianfeng, "@@sijian", "@sijian-discard");
            }
        }

        return false;
    }
};

class Suishi: public TriggerSkill {
public:
    Suishi(): TriggerSkill("suishi") {
        events << Dying << Death;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target = NULL;
        if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.damage && dying.damage->from)
                target = dying.damage->from;
            if (dying.who != player && target && room->askForChoice(target, "suishi1", "draw+no") == "draw") {
                room->broadcastSkillInvoke(objectName(), 1);
                LogMessage log;
                log.type = (target != player) ? "#InvokeOthersSkill" : "#InvokeSkill";
                log.from = target;
                log.to << player;
                log.arg = objectName();
                room->sendLog(log);

                player->drawCards(1);
            }
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.damage && death.damage->from)
                target = death.damage->from;
            if (target && room->askForChoice(target, "suishi2", "damage+no") == "damage") {
                room->broadcastSkillInvoke(objectName(), 2);
                LogMessage log;
                log.type = (target != player) ? "#InvokeOthersSkill" : "#InvokeSkill";
                log.from = target;
                log.to << player;
                log.arg = objectName();
                room->sendLog(log);

                room->loseHp(player);
            }
        }
        return false;
    }
};

ShuangrenCard::ShuangrenCard() {
    will_throw = false;
    handling_method = Card::MethodPindian;
}

bool ShuangrenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void ShuangrenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool success = effect.from->pindian(effect.to, "shuangren", this);
    if (success) {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *target, room->getAlivePlayers()) {
            if (effect.from->canSlash(target, NULL, false))
                targets << target;
        }
        if (targets.isEmpty())
            return;

        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "shuangren-slash");

        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("shuangren");

        CardUseStruct card_use;
        card_use.card = slash;
        card_use.from = effect.from;
        card_use.to << target;
        room->useCard(card_use, false);
    } else {
        room->broadcastSkillInvoke("shuangren", 2);
        room->setPlayerFlag(effect.from, "SkipPlay");
    }
}

class ShuangrenViewAsSkill: public OneCardViewAsSkill {
public:
    ShuangrenViewAsSkill(): OneCardViewAsSkill("shuangren") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@shuangren";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        Card *card = new ShuangrenCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class Shuangren: public PhaseChangeSkill {
public:
    Shuangren(): PhaseChangeSkill("shuangren") {
        view_as_skill = new ShuangrenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *jiling) const{
        if (jiling->getPhase() == Player::Play) {
            Room *room = jiling->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(jiling);
            foreach (ServerPlayer *player, other_players) {
                if (!player->isKongcheng()) {
                    can_invoke = true;
                    break;
                }
            }

            if (can_invoke)
                room->askForUseCard(jiling, "@@shuangren", "@shuangren-card", -1, Card::MethodPindian);
            if (jiling->hasFlag("SkipPlay"))
                return true;
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const {
        if (card->isKindOf("Slash"))
            return -2;
        else
            return 1;
    }
};

XiongyiCard::XiongyiCard() {
    mute = true;
}

bool XiongyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

bool XiongyiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return true;
}

void XiongyiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    if (!use.to.contains(use.from))
        use.to << use.from;
    use.from->loseMark("@arise");
    room->broadcastSkillInvoke("xiongyi");
    room->broadcastInvoke("animate", "lightbox:$XiongyiAnimate");
    room->getThread()->delay(2000);
    SkillCard::onUse(room, use);
}

void XiongyiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(3);
    effect.from->addMark("xiongyi");
}

class Xiongyi: public ZeroCardViewAsSkill {
public:
    Xiongyi(): ZeroCardViewAsSkill("xiongyi") {
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@arise") >= 1;
    }

    virtual const Card *viewAs() const{
        return new XiongyiCard;
    }
};

class XiongyiRecover: public TriggerSkill {
public:
    XiongyiRecover(): TriggerSkill("#xiongyi") {
        events << CardFinished;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getMark("@arise") < 1 && player->getMark("xiongyi") > 0) {
            if (player->getMark("xiongyi") <= (room->getAlivePlayers().length()) / 2) {
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
                player->setMark("xiongyi", 0);
            }
        }
        return false;
    }
};

class Kuangfu: public TriggerSkill {
public:
    Kuangfu(): TriggerSkill("kuangfu") {
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *panfeng, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if (damage.card && damage.card->isKindOf("Slash") && target->hasEquip() && !damage.chain && !damage.transfer) {
            if (!panfeng->askForSkillInvoke(objectName(), data))
                return false;
            int card_id = room->askForCardChosen(panfeng, target , "e", "kuangfu");
            const Card *card = Sanguosha->getCard(card_id);

            int equip_index = -1;
            const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
            equip_index = static_cast<int>(equip->location());

            QStringList choicelist;
            choicelist << "throw";
            if (equip_index > -1 && panfeng->getEquip(equip_index) == NULL)
                choicelist << "move";

            QString choice = room->askForChoice(panfeng, "kuangfu", choicelist.join("+"));

            if (choice == "move") {
                room->broadcastSkillInvoke(objectName(), 1);
                room->moveCardTo(card, panfeng, Player::PlaceEquip);
            } else {
                room->broadcastSkillInvoke(objectName(), 2);
                room->throwCard(card, target, panfeng);
            }
        }

        return false;
    }
};

class Huoshui: public TriggerSkill {
public:
    Huoshui(): TriggerSkill("huoshui") {
        events << EventPhaseStart << EventPhaseChanging << Death
               << EventLoseSkill << EventAcquireSkill
               << PostHpReduced << HpRecover << MaxHpChanged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 4;
    }

    void setHuoshuiFlag(Room *room, ServerPlayer *player, bool is_lose) const{
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            room->setPlayerFlag(p, is_lose ? "-huoshui" : "huoshui");
            room->filterCards(p, p->getCards("he"), !is_lose);
        }
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::RoundStart) {
                setHuoshuiFlag(room, player, false);
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (TriggerSkill::triggerable(player) && change.to == Player::NotActive)
                setHuoshuiFlag(room, player, true);
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
            if (player->hasSkill(objectName()))
                setHuoshuiFlag(room, player, true);
        } else if (triggerEvent == EventLoseSkill) {
            if (data.toString() == objectName()
                && room->getCurrent() && room->getCurrent() == player)
                setHuoshuiFlag(room, player, true);
        } else if (triggerEvent == EventAcquireSkill) {
            if (data.toString() == objectName()
                && room->getCurrent() && room->getCurrent() == player)
                setHuoshuiFlag(room, player, false);
        } else if (triggerEvent == PostHpReduced) {
            if (player->hasFlag(objectName())) {
                int reduce = 0;
                if (data.canConvert<DamageStruct>()) {
                    DamageStruct damage = data.value<DamageStruct>();
                    reduce = damage.damage;
                } else
                    reduce = data.toInt();
                int hp = player->getHp();
                int maxhp_2 = (player->getMaxHp() + 1) / 2;
                if (hp < maxhp_2 && hp + reduce >= maxhp_2)
                    room->filterCards(player, player->getCards("he"), false);
            }
        } else if (triggerEvent == MaxHpChanged) {
            if (player->hasFlag(objectName())) {
                room->filterCards(player, player->getCards("he"), true);
            }
        } else if (triggerEvent == HpRecover) {
            RecoverStruct recover_struct = data.value<RecoverStruct>();
            int recover = recover_struct.recover;
            if (player->hasFlag(objectName())) {
                int hp = player->getHp();
                int maxhp_2 = (player->getMaxHp() + 1) / 2;
                if (hp >= maxhp_2 && hp - recover < maxhp_2)
                    room->filterCards(player, player->getCards("he"), true);
            }
        }
        return false;
    }
};

QingchengCard::QingchengCard() {
    will_throw = false;
    handling_method = Card::MethodDiscard;
}

bool QingchengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void QingchengCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QStringList skill_list;
    foreach (const Skill *skill, effect.to->getVisibleSkillList()) {
        if (!skill_list.contains(skill->objectName()) && !skill->inherits("SPConvertSkill") && !skill->isAttachedLordSkill()) {
            skill_list << skill->objectName();
        }
    }
    QString skill_qc;
    if (!skill_list.isEmpty()) {
        QVariant data_for_ai = QVariant::fromValue((PlayerStar)effect.to);
        skill_qc = room->askForChoice(effect.from, "qingcheng", skill_list.join("+"), data_for_ai);
    }
    room->throwCard(this, effect.from);

    if (!skill_qc.isEmpty()) {
        LogMessage log;
        log.type = "$QingchengNullify";
        log.from = effect.from;
        log.to << effect.to;
        log.arg = skill_qc;
        room->sendLog(log);

        QStringList Qingchenglist = effect.to->tag["Qingcheng"].toStringList();
        Qingchenglist << skill_qc;
        effect.to->tag["Qingcheng"] = QVariant::fromValue(Qingchenglist);
        room->setPlayerMark(effect.to, "Qingcheng" + skill_qc, 1);
        room->filterCards(effect.to, effect.to->getCards("he"), true);
    }
}

class QingchengViewAsSkill: public OneCardViewAsSkill {
public:
    QingchengViewAsSkill(): OneCardViewAsSkill("qingcheng") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isNude();
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("EquipCard") && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        QingchengCard *first = new QingchengCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Qingcheng: public TriggerSkill {
public:
    Qingcheng(): TriggerSkill("qingcheng") {
        events << EventPhaseStart;
        view_as_skill = new QingchengViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 4;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() == Player::RoundStart) {
            QStringList Qingchenglist = player->tag["Qingcheng"].toStringList();
            foreach (QString skill_name, Qingchenglist) {
                room->setPlayerMark(player, "Qingcheng" + skill_name, 0);
                if (player->hasSkill(skill_name)) {
                    LogMessage log;
                    log.type = "$QingchengReset";
                    log.from = player;
                    log.arg = skill_name;
                    room->sendLog(log);
                }
            }
            player->tag.remove("Qingcheng");
            room->filterCards(player, player->getCards("he"), false);
        }
        return false;
    }
};

HegemonyPackage::HegemonyPackage()
    : Package("hegemony")
{
    General *yuejin = new General(this, "yuejin", "wei");
    yuejin->addSkill(new Xiaoguo);

    General *ganfuren = new General(this, "ganfuren", "shu", 3, false);
    ganfuren->addSkill(new Shushen);
    ganfuren->addSkill(new Shenzhi);

    General *heg_luxun = new General(this, "heg_luxun", "wu", 3);
    heg_luxun->addSkill("qianxun");
    heg_luxun->addSkill(new Duoshi);

    General *dingfeng = new General(this, "dingfeng", "wu");
    dingfeng->addSkill(new Skill("duanbing"));
    dingfeng->addSkill(new Fenxun);

    General *mateng = new General(this, "mateng", "qun");
    mateng->addSkill("mashu");
    mateng->addSkill(new MarkAssignSkill("@arise", 1));
    mateng->addSkill(new Xiongyi);
    mateng->addSkill(new XiongyiRecover);
    related_skills.insertMulti("xiongyi", "#xiongyi");
    related_skills.insertMulti("xiongyi", "#@arise-1");

    General *kongrong = new General(this, "kongrong", "qun", 3);
    kongrong->addSkill(new Mingshi);
    kongrong->addSkill(new Lirang);
    kongrong->addSkill(new FakeMoveSkill("lirang", FakeMoveSkill::SourceOnly));
    related_skills.insertMulti("lirang", "#lirang-fake-move");

    General *jiling = new General(this, "jiling", "qun", 4);
    jiling->addSkill(new Shuangren);
    jiling->addSkill(new SlashNoDistanceLimitSkill("shuangren"));
    related_skills.insertMulti("shuangren", "#shuangren-slash-ndl");

    General *tianfeng = new General(this, "tianfeng", "qun", 3);
    tianfeng->addSkill(new Sijian);
    tianfeng->addSkill(new Suishi);

    General *panfeng = new General(this, "panfeng", "qun");
    panfeng->addSkill(new Kuangfu);

    General *zoushi = new General(this, "zoushi", "qun", 3, false);
    zoushi->addSkill(new Huoshui);
    zoushi->addSkill(new Qingcheng);

    General *heg_caopi = new General(this, "heg_caopi$", "wei", 3, true, true);
    heg_caopi->addSkill("fangzhu");
    heg_caopi->addSkill("xingshang");
    heg_caopi->addSkill("songwei");

    General *heg_zhenji = new General(this, "heg_zhenji", "wei", 3, false, true);
    heg_zhenji->addSkill("qingguo");
    heg_zhenji->addSkill("luoshen");

    General *heg_zhugeliang = new General(this, "heg_zhugeliang", "shu", 3, true, true);
    heg_zhugeliang->addSkill("guanxing");
    heg_zhugeliang->addSkill("kongcheng");
    heg_zhugeliang->addSkill("#kongcheng-effect");

    General *heg_huangyueying = new General(this, "heg_huangyueying", "shu", 3, false, true);
    heg_huangyueying->addSkill("jizhi");
    heg_huangyueying->addSkill("qicai");

    General *heg_zhouyu = new General(this, "heg_zhouyu", "wu", 3, true, true);
    heg_zhouyu->addSkill("yingzi");
    heg_zhouyu->addSkill("fanjian");

    General *heg_xiaoqiao = new General(this, "heg_xiaoqiao", "wu", 3, false, true);
    heg_xiaoqiao->addSkill("tianxiang");
    heg_xiaoqiao->addSkill("#tianxiang");
    heg_xiaoqiao->addSkill("hongyan");

    General *heg_lvbu = new General(this, "heg_lvbu", "qun", 4, true, true);
    heg_lvbu->addSkill("wushuang");

    General *heg_diaochan = new General(this, "heg_diaochan", "qun", 3, false, true);
    heg_diaochan->addSkill("lijian");
    heg_diaochan->addSkill("biyue");

    addMetaObject<ShushenCard>();
    addMetaObject<DuoshiCard>();
    addMetaObject<FenxunCard>();
    addMetaObject<ShuangrenCard>();
    addMetaObject<XiongyiCard>();
    addMetaObject<SijianCard>();
    addMetaObject<QingchengCard>();
}

ADD_PACKAGE(Hegemony)