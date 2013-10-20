#include "maneuvering.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"

class FanSkill: public OneCardViewAsSkill {
public:
    FanSkill(): OneCardViewAsSkill("Fan") {
        filter_pattern = "%slash";
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

class GudingBladeSkill: public WeaponSkill {
public:
    GudingBladeSkill(): WeaponSkill("GudingBlade") {
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash")
            && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0
            && damage.to->isKongcheng() && damage.by_user && !damage.chain && !damage.transfer) {
            room->setEmotion(player, "weapon/guding_blade");

            LogMessage log;
            log.type = "#GudingBladeEffect";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++ damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

GudingBlade::GudingBlade(Suit suit, int number)
    : Weapon(suit, number, 2)
{
    setObjectName("GudingBlade");
}

class VineSkill: public ArmorSkill {
public:
    VineSkill(): ArmorSkill("Vine") {
        events << DamageInflicted << SlashEffected << CardEffected;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.nature == DamageStruct::Normal) {
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
        } else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("SavageAssault") || effect.card->isKindOf("ArcheryAttack")) {
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
        } else if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.nature == DamageStruct::Fire) {
                room->setEmotion(player, "armor/vineburn");
                LogMessage log;
                log.type = "#VineDamage";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(++ damage.damage);
                room->sendLog(log);

                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

Vine::Vine(Suit suit, int number)
    : Armor(suit, number)
{
    setObjectName("Vine");
}

class SilverLionSkill: public ArmorSkill {
public:
    SilverLionSkill(): ArmorSkill("SilverLion") {
        events << DamageInflicted << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == DamageInflicted && ArmorSkill::triggerable(player)) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.damage > 1) {
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
        } else if (player->hasFlag("SilverLionRecover")) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != player || !move.from_places.contains(Player::PlaceEquip))
                return false;
            for (int i = 0; i < move.card_ids.size(); i++) {
                if (move.from_places[i] != Player::PlaceEquip) continue;
                const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
                if (card->objectName() == objectName()) {
                    player->setFlags("-SilverLionRecover");
                    if (player->isWounded()) {
                        room->setEmotion(player, "armor/silver_lion");
                        RecoverStruct recover;
                        recover.card = card;
                        room->recover(player, recover);
                    }
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

SupplyShortage::SupplyShortage(Card::Suit suit, int number)
    : DelayedTrick(suit, number)
{
    setObjectName("supply_shortage");

    judge.pattern = ".|club";
    judge.good = true;
    judge.reason = objectName();
}

bool SupplyShortage::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select->containsTrick(objectName()) || to_select == Self)
        return false;

    int distance_limit = 1 + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this);
    int rangefix = 0;
    if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
        rangefix += 1;

    if (Self->distanceTo(to_select, rangefix) > distance_limit)
        return false;

    return true;
}

void SupplyShortage::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Draw);
}

ManeuveringPackage::ManeuveringPackage()
    : Package("maneuvering", Package::CardPack)
{
    QList<Card *> cards;

    // spade
    cards << new GudingBlade(Card::Spade, 1)
          << new Vine(Card::Spade, 2)
          << new SupplyShortage(Card::Spade, 10);
   // club
    cards << new SilverLion(Card::Club, 1)
          << new Vine(Card::Club, 2)
          << new SupplyShortage(Card::Club, 4);

    // diamond
    cards << new Fan(Card::Diamond, 1);

    DefensiveHorse *hualiu = new DefensiveHorse(Card::Diamond, 13);
    hualiu->setObjectName("HuaLiu");

    cards << hualiu;

    foreach (Card *card, cards)
        card->setParent(this);

    skills << new GudingBladeSkill << new FanSkill
           << new VineSkill << new SilverLionSkill;
}

ADD_PACKAGE(Maneuvering)

