#include "standard.h"
#include "serverplayer.h"
#include "room.h"

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

bool EquipCard::targetFixed() const{
    return true;
}

void EquipCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const Card *equip = this;
    const Card *uninstalled = source->replaceEquip(equip);
    if(uninstalled){
        room->broadcast(QString("! moveCard %1:%2@equip->_@_").arg(uninstalled->getID()).arg(source->objectName()));
        room->appendToDiscard(uninstalled->getID());
        Player::MoveCard(source, Player::Equip, NULL, Player::DiscardedPile, uninstalled->getID());
    }
    room->broadcast(QString("! moveCard %1:%2@hand->%2@equip").arg(equip->getID()).arg(source->objectName()));
    Player::MoveCard(source, Player::Hand, source, Player::Equip, getID());

    room->broadcast(QString("! activate %1:Nop:").arg(source->objectName()));
}

QString GlobalEffect::getSubtype() const{
    return "global_effect";
}

bool GlobalEffect::targetFixed() const{
    return true;
}

QString AOE::getSubtype() const{
    return "aoe";
}

bool AOE::targetFixed() const{
    return true;
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
    // package name
    t["standard"] = tr("standard");

    // role names
    t["lord"] = tr("lord");
    t["loyalist"] = tr("loyalist");
    t["rebel"] = tr("rebel");
    t["renegade"] = tr("renegade");

    // phase names
    t["start"] = tr("start");
    t["judge"] = tr("judge");
    t["draw"] = tr("draw");
    t["play"] = tr("play");
    t["discard"] = tr("discard");
    t["finish"] = tr("finish");

    addCards();
    addGenerals();
}
