#include "standard.h"

QString BasicCard::getType() const{
    return "basic";
}

int BasicCard::getTypeId() const{
    return 0;
}

QString TrickCard::getType() const{
    return "trick";
}

int TrickCard::getTypeId() const{
    return 1;
}

QString EquipCard::getType() const{
    return "equip";
}

int EquipCard::getTypeId() const{
    return 2;
}

QString GlobalEffect::getSubtype() const{
    return "global_effect";
}

QString AOE::getSubtype() const{
    return "aoe";
}

QString SingleTargetTrick::getSubtype() const{
    return "single_target_trick";
}

QString DelayedTrick::getSubtype() const{
    return "delayed_trick";
}

QString Weapon::getSubtype() const{
    return "weapon";
}

QString Armor::getSubtype() const{
    return "armor";
}

Horse::Horse(const QString &name, Suit suit, int number, int correct)
    :EquipCard(suit, number), correct(correct)
{
    setObjectName(name);
}

QString Horse::getSubtype() const{
    if(correct > 0)
        return "defensive_horse";
    else
        return "offensive_horse";
}

StandardPackage::StandardPackage()
    :Package("standard")
{
    t["standard"] = tr("standard");
    t["lord"] = tr("lord");
    t["loyalist"] = tr("loyalist");
    t["rebel"] = tr("rebel");
    t["renegade"] = tr("renegade");

    addCards();
    addGenerals();
}
