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

void EquipCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const EquipCard *equipped = source->getEquip(getSubtype());
    if(equipped){
        CardMoveStruct detach;
        detach.card_id = equipped->getId();
        detach.from = source;
        detach.to = NULL;
        detach.from_place = Player::Equip;
        detach.to_place = Player::DiscardedPile;
        detach.open = true;

        ActiveRecord *uninstall = new ActiveRecord;
        uninstall->method = "moveCard";
        uninstall->target = NULL;
        uninstall->data = QVariant::fromValue(detach);
        room->enqueueRecord(uninstall);
    }

    CardMoveStruct attach;
    attach.card_id = getId();
    attach.from = source;
    attach.to = source;
    attach.from_place = Player::Hand;
    attach.to_place = Player::Equip;
    attach.open = true;

    ActiveRecord *install = new ActiveRecord;
    install->method = "moveCard";
    install->target = NULL;
    install->data = QVariant::fromValue(attach);
    room->enqueueRecord(install);
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

    // states
    t["online"] = tr("online");
    t["offline"] = tr("offline");

    // normal phrases
    t["yes"] = tr("yes");
    t["no"] = tr("no");
    t["nothing"] = tr("nothing");

    addCards();
    addGenerals();
}
