/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "standard-equips.h"
#include "standard-package.h"
#include "skill.h"
#include "standard-basics.h"
#include "client.h"
#include "engine.h"

Crossbow::Crossbow(Suit suit, int number)
    : Weapon(suit, number, 1)
{
    setObjectName("Crossbow");
}

class DoubleSwordSkill : public WeaponSkill {
public:
    DoubleSwordSkill() : WeaponSkill("DoubleSword") {
        events << TargetChosen;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return WeaponSkill::triggerable(target);
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from != player)
            return false;

        foreach(ServerPlayer *to, use.to) {
            if (((use.from->isMale() && to->isFemale()) || (use.from->isFemale() && to->isMale()))
                && use.card->isKindOf("Slash")) {
                if (use.from->askForSkillInvoke(objectName())) {
                    room->setEmotion(use.from, "weapon/double_sword");

                    bool draw_card = false;
                    if (!to->canDiscard(to, "h"))
                        draw_card = true;
                    else {
                        QString prompt = "double-sword-card:" + use.from->objectName();
                        const Card *card = room->askForCard(to, ".", prompt, data);
                        if (!card) draw_card = true;
                    }
                    if (draw_card)
                        use.from->drawCards(1);
                }
            }
        }

        return false;
    }
};

DoubleSword::DoubleSword(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("DoubleSword");
}

class QinggangSwordSkill : public WeaponSkill {
public:
    QinggangSwordSkill() : WeaponSkill("QinggangSword") {
        events << TargetChosen;
        frequency = Compulsory;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (WeaponSkill::triggerable(use.from) && use.from == player && use.card->isKindOf("Slash")) {
            bool do_anim = false;
            foreach(ServerPlayer *p, use.to.toSet()) {
                if (p->getMark("Equips_of_Others_Nullified_to_You") == 0) {
                    do_anim = (p->getArmor() && p->hasArmorEffect(p->getArmor()->objectName())) || p->hasArmorEffect("bazhen");
                    p->addQinggangTag(use.card);
                }
            }
            if (do_anim)
                room->setEmotion(use.from, "weapon/qinggang_sword");
        }
        return false;
    }
};

QinggangSword::QinggangSword(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("QinggangSword");
}

class SpearSkill : public ViewAsSkill {
public:
    SpearSkill() : ViewAsSkill("Spear") {
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player)
            && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash" && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        Slash *slash = new Slash(Card::SuitToBeDecided, 0);
        slash->setSkillName(objectName());
        slash->addSubcards(cards);

        return slash;
    }
};

Spear::Spear(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("Spear");
}

class AxeViewAsSkill : public ViewAsSkill {
public:
    AxeViewAsSkill() : ViewAsSkill("Axe") {
        response_pattern = "@Axe";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && to_select != Self->getWeapon() && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class AxeSkill : public WeaponSkill {
public:
    AxeSkill() : WeaponSkill("Axe") {
        events << SlashMissed;
        view_as_skill = new AxeViewAsSkill;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if (!effect.to->isAlive() || effect.to->getMark("Equips_of_Others_Nullified_to_You") > 0)
            return false;

        const Card *card = NULL;
        if (player->getCardCount(true) >= 3) // Need 2 more cards except from the weapon itself
            card = room->askForCard(player, "@Axe", "@Axe:" + effect.to->objectName(), data, objectName());
        if (card) {
            room->setEmotion(player, "weapon/axe");
            room->slashResult(effect, NULL);
        }

        return false;
    }
};

Axe::Axe(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("Axe");
}

class KylinBowSkill : public WeaponSkill {
public:
    KylinBowSkill() : WeaponSkill("KylinBow") {
        events << DamageCaused;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();

        QStringList horses;
        if (damage.card && damage.card->isKindOf("Slash") && damage.by_user && !damage.chain && !damage.transfer
            && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0) {
            if (damage.to->getDefensiveHorse() && damage.from->canDiscard(damage.to, damage.to->getDefensiveHorse()->getEffectiveId()))
                horses << "dhorse";
            if (damage.to->getOffensiveHorse() && damage.from->canDiscard(damage.to, damage.to->getOffensiveHorse()->getEffectiveId()))
                horses << "ohorse";

            if (horses.isEmpty())
                return false;

            if (player == NULL) return false;
            if (!player->askForSkillInvoke(objectName(), data))
                return false;

            room->setEmotion(player, "weapon/kylin_bow");

            QString horse_type = room->askForChoice(player, objectName(), horses.join("+"));

            if (horse_type == "dhorse")
                room->throwCard(damage.to->getDefensiveHorse(), damage.to, damage.from);
            else if (horse_type == "ohorse")
                room->throwCard(damage.to->getOffensiveHorse(), damage.to, damage.from);
        }

        return false;
    }
};

KylinBow::KylinBow(Suit suit, int number)
    : Weapon(suit, number, 5)
{
    setObjectName("KylinBow");
}

class EightDiagramSkill : public ArmorSkill {
public:
    EightDiagramSkill() : ArmorSkill("EightDiagram") {
        events << CardAsked;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!ArmorSkill::triggerable(player)) return QStringList();
        QString asked = data.toStringList().first();
        if (asked == "jink") return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (room->askForSkillInvoke(player, objectName())) {
            if (player->hasArmorEffect("bazhen")) {
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                player->showGeneral(player->inHeadSkills("bazhen"));
            }
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        int armor_id = -1;
        if (player->getArmor()) {
            armor_id = player->getArmor()->getId();
            room->setCardFlag(armor_id, "using");
        }
        room->setEmotion(player, "armor/eight_diagram");
        JudgeStruct judge;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = objectName();
        judge.who = player;

        room->judge(judge);
        if (armor_id != -1)
            room->setCardFlag(armor_id, "-using");

        if (judge.isGood()) {
            Jink *jink = new Jink(Card::NoSuit, 0);
            jink->setSkillName(objectName());
            room->provide(jink);

            return true;
        }


        return false;
    }

    int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

EightDiagram::EightDiagram(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("EightDiagram");
}

class IceSwordSkill : public WeaponSkill {
public:
    IceSwordSkill() : WeaponSkill("IceSword") {
        events << DamageCaused;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();

        if (damage.card && damage.card->isKindOf("Slash")
            && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0
            && !damage.to->isNude() && damage.by_user
            && !damage.chain && !damage.transfer && player->askForSkillInvoke("IceSword", data)) {
            room->setEmotion(player, "weapon/ice_sword");
            if (damage.from->canDiscard(damage.to, "he")) {
                int card_id = room->askForCardChosen(player, damage.to, "he", "IceSword", false, Card::MethodDiscard);
                room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);

                if (damage.from->isAlive() && damage.to->isAlive() && damage.from->canDiscard(damage.to, "he")) {
                    card_id = room->askForCardChosen(player, damage.to, "he", "IceSword", false, Card::MethodDiscard);
                    room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);
                }
            }
            return true;
        }
        return false;
    }
};

IceSword::IceSword(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("IceSword");
}

class RenwangShieldSkill : public ArmorSkill {
public:
    RenwangShieldSkill() : ArmorSkill("RenwangShield") {
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!ArmorSkill::triggerable(player)) return QStringList();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.slash->isBlack()) return QStringList(objectName());

        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        LogMessage log;
        log.type = "#ArmorNullify";
        log.from = player;
        log.arg = objectName();
        log.arg2 = effect.slash->objectName();
        player->getRoom()->sendLog(log);

        room->setEmotion(player, "armor/renwang_shield");
        effect.to->setFlags("Global_NonSkillNullify");
        return true;
    }
};

RenwangShield::RenwangShield(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("RenwangShield");
}

class FanSkill : public OneCardViewAsSkill {
public:
    FanSkill() : OneCardViewAsSkill("Fan") {
        filter_pattern = "%slash";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player) && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
            && pattern == "slash" && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *acard = new FireSlash(originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        return acard;
    }
};

Fan::Fan(Suit suit, int number)
    : Weapon(suit, number, 4)
{
    setObjectName("Fan");
}

SixSwords::SixSwords(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("SixSwords");
}

class SixSwordsSkill : public AttackRangeSkill{
public:
    SixSwordsSkill() : AttackRangeSkill("SixSwords"){
    }

    virtual int getExtra(const Player *target, bool) const{
        foreach(const Player *p, target->getAliveSiblings()){
            if (p->hasWeapon("SixSwords") && p->isFriendWith(target) && p->getMark("Equips_Nullified_to_Yourself") == 0)
                return 1;
        }

        return 0;
    }
};

Triblade::Triblade(Card::Suit suit, int number) : Weapon(suit, number, 3){
    setObjectName("Triblade");
}

TribladeSkillCard::TribladeSkillCard() : SkillCard(){
    setObjectName("Triblade");
}

bool TribladeSkillCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.length() == 0 && to_select->hasFlag("TribladeCanBeSelected");
}

void TribladeSkillCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->damage(DamageStruct("Triblade", source, targets[0]));
}

class TribladeSkillVS : public OneCardViewAsSkill{
public:
    TribladeSkillVS() : OneCardViewAsSkill("Triblade"){
        response_pattern = "@@Triblade";
        filter_pattern = ".|.|.|hand!";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TribladeSkillCard *c = new TribladeSkillCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class TribladeSkill : public WeaponSkill{
public:
    TribladeSkill() : WeaponSkill("Triblade"){
        events << Damage;
        view_as_skill = new TribladeSkillVS;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to && damage.to->isAlive() && damage.card && damage.card->isKindOf("Slash")
            && damage.by_user && !damage.chain && !damage.transfer){
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if (damage.to->distanceTo(p) == 1){
                    players << p;
                    room->setPlayerFlag(p, "TribladeCanBeSelected");
                }
            }
            if (players.isEmpty())
                return false;
            room->askForUseCard(player, "@@Triblade", "@Triblade");
        }

        foreach(ServerPlayer *p, room->getAllPlayers())
            if (p->hasFlag("TribladeCanBeSelected"))
                room->setPlayerFlag(p, "TribladeCanBeSelected");

        return false;
    }
};

class VineSkill : public ArmorSkill {
public:
    VineSkill() : ArmorSkill("Vine") {
        events << DamageInflicted << SlashEffected << CardEffected;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!ArmorSkill::triggerable(player)) return QStringList();
        if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.nature == DamageStruct::Normal)
                return QStringList(objectName());
        }
        else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("SavageAssault") || effect.card->isKindOf("ArcheryAttack"))
                return QStringList(objectName());
        }
        else if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.nature == DamageStruct::Fire)
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            room->setEmotion(player, "armor/vine");
            LogMessage log;
            log.from = player;
            log.type = "#ArmorNullify";
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            room->sendLog(log);

            effect.to->setFlags("Global_NonSkillNullify");
            return true;
        }
        else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            room->setEmotion(player, "armor/vine");
            LogMessage log;
            log.from = player;
            log.type = "#ArmorNullify";
            log.arg = objectName();
            log.arg2 = effect.card->objectName();
            room->sendLog(log);

            effect.to->setFlags("Global_NonSkillNullify");
            return true;
        }
        else if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            room->setEmotion(player, "armor/vineburn");
            LogMessage log;
            log.type = "#VineDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

Vine::Vine(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("Vine");
}

class SilverLionSkill : public ArmorSkill {
public:
    SilverLionSkill() : ArmorSkill("SilverLion") {
        events << DamageInflicted << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            if (ArmorSkill::triggerable(player) && damage.damage > 1)
                return QStringList(objectName());
        }
        else if (player->hasFlag("SilverLionRecover")) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != player || !move.from_places.contains(Player::PlaceEquip))
                return QStringList();
            for (int i = 0; i < move.card_ids.size(); i++) {
                if (move.from_places[i] != Player::PlaceEquip) continue;
                const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
                if (card->objectName() == objectName()) {
                    if (!player->isWounded()) {
                        player->setFlags("-SilverLionRecover");
                        return QStringList();
                    }
                    return QStringList(objectName());
                }
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            room->setEmotion(player, "armor/silver_lion");
            LogMessage log;
            log.type = "#SilverLion";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            room->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }
        else {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();

            for (int i = 0; i < move.card_ids.size(); i++) {
                if (move.from_places[i] != Player::PlaceEquip) continue;
                const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
                if (card->objectName() == objectName()) {
                    player->setFlags("-SilverLionRecover");

                    room->setEmotion(player, "armor/silver_lion");
                    RecoverStruct recover;
                    recover.card = card;
                    room->recover(player, recover);

                    return false;
                }
            }

        }
        return false;
    }
};

SilverLion::SilverLion(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("SilverLion");
}

void SilverLion::onUninstall(ServerPlayer *player) const{
    if (player->isAlive() && player->hasArmorEffect(objectName()))
        player->setFlags("SilverLionRecover");
}

class HorseSkill : public DistanceSkill {
public:
    HorseSkill() :DistanceSkill("Horse"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        const Horse *horse = NULL;
        if (from->getOffensiveHorse() && from->getMark("Equips_Nullified_to_Yourself") == 0) {
            horse = qobject_cast<const Horse *>(from->getOffensiveHorse()->getRealCard());
            if (horse) correct += horse->getCorrect();
        }
        if (to->getDefensiveHorse() && to->getMark("Equips_Nullified_to_Yourself") == 0) {
            horse = qobject_cast<const Horse *>(to->getDefensiveHorse()->getRealCard());
            if (horse) correct += horse->getCorrect();
        }

        return correct;
    }
};

QList<Card *> StandardCardPackage::equipCards(){

    QList<Card *> cards;

    cards
        << new Crossbow
        << new DoubleSword
        << new QinggangSword
        << new IceSword
        << new Spear
        << new Fan
        << new Axe
        << new KylinBow
        << new SixSwords
        << new Triblade

        << new EightDiagram
        << new RenwangShield
        << new Vine
        << new SilverLion;


    QList<Card *> horses;

    horses
        << new DefensiveHorse(Card::Spade, 5)
        << new DefensiveHorse(Card::Club, 5)
        << new DefensiveHorse(Card::Heart, 13)
        << new OffensiveHorse(Card::Heart, 5)
        << new OffensiveHorse(Card::Spade, 13)
        << new OffensiveHorse(Card::Diamond, 13);

    horses.at(0)->setObjectName("JueYing");
    horses.at(1)->setObjectName("DiLu");
    horses.at(2)->setObjectName("ZhuaHuangFeiDian");
    horses.at(3)->setObjectName("ChiTu");
    horses.at(4)->setObjectName("DaYuan");
    horses.at(5)->setObjectName("ZiXing");

    cards << horses;

    return cards;
}

void StandardCardPackage::addEquipSkills(){

    skills << new DoubleSwordSkill << new QinggangSwordSkill << new IceSwordSkill
        << new SpearSkill << new FanSkill << new AxeSkill << new KylinBowSkill
        << new TribladeSkill << new EightDiagramSkill << new RenwangShieldSkill
        << new VineSkill << new SilverLionSkill << new SixSwordsSkill;

    addMetaObject<TribladeSkillCard>();

    skills << new HorseSkill;
}
