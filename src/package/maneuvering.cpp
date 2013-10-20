#include "maneuvering.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"

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

ManeuveringPackage::ManeuveringPackage()
    : Package("maneuvering", Package::CardPack)
{
    QList<Card *> cards;

    // spade
    cards << new GudingBlade(Card::Spade, 1);

    DefensiveHorse *hualiu = new DefensiveHorse(Card::Diamond, 13);
    hualiu->setObjectName("HuaLiu");

    cards << hualiu;

    foreach (Card *card, cards)
        card->setParent(this);

    skills << new GudingBladeSkill;
}

ADD_PACKAGE(Maneuvering)

