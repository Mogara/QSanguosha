#include "standard.h"
#include "serverplayer.h"
#include "room.h"
#include "skill.h"
#include "maneuvering.h"

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

void EquipCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const EquipCard *equipped = NULL;
    switch(location()){
    case WeaponLocation: equipped = source->getWeapon(); break;
    case ArmorLocation: equipped = source->getArmor(); break;
    case DefensiveHorseLocation: equipped = source->getDefensiveHorse(); break;
    case OffensiveHorseLocation: equipped = source->getOffensiveHorse(); break;
    }

    if(equipped)
        room->throwCard(equipped);

    CardMoveStruct attach;
    attach.card_id = getId();
    attach.from = source;
    attach.to = source;
    attach.from_place = Player::Hand;
    attach.to_place = Player::Equip;
    attach.open = true;

    room->moveCard(attach);
}

void EquipCard::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    if(set_flag)
        room->setPlayerFlag(player, objectName());

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void EquipCard::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    if(set_flag)
        room->setPlayerFlag(player, "-" + objectName());

    if(skill)
        room->getThread()->removeTriggerSkill(skill);
}

QString GlobalEffect::getSubtype() const{
    return "global_effect";
}

void GlobalEffect::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    QList<ServerPlayer *> all_players = room->getAllPlayers();
    foreach(ServerPlayer *player, all_players){
        CardEffectStruct effect;
        effect.card = this;
        effect.from = source;
        effect.to = player;

        room->cardEffect(effect);
    }
}

QString AOE::getSubtype() const{
    return "aoe";
}

void AOE::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    QList<ServerPlayer *> other_players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, other_players){
        if(player->hasFlag("vine"))
            continue;

        CardEffectStruct effect;
        effect.card = this;
        effect.from = source;
        effect.to = player;

        room->cardEffect(effect);
    }
}

QString SingleTargetTrick::getSubtype() const{
    return "single_target_trick";
}

DelayedTrick::DelayedTrick(Suit suit, int number, bool movable)
    :TrickCard(suit, number), movable(movable)
{
}

void DelayedTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Judging, true);
}

QString DelayedTrick::getSubtype() const{
    return "delayed_trick";
}

#include "engine.h"

void DelayedTrick::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    int card_id = room->getJudgeCard(effect.to);
    const Card *card = Sanguosha->getCard(card_id);
    if(judge(card)){
        takeEffect(effect.to);
        room->throwCard(this);
    }else{
        if(movable){
            QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
            players << effect.to;

            foreach(ServerPlayer *player, players){
                if(!player->containsTrick(objectName())){
                    room->moveCardTo(this, player, Player::Judging, true);
                    break;
                }
            }
        }else
            room->throwCard(this);
    }
}

const DelayedTrick *DelayedTrick::CastFrom(const Card *card){
    DelayedTrick *trick = NULL;
    Card::Suit suit = card->getSuit();
    int number = card->getNumber();
    if(card->getSuit() == Card::Diamond){
        trick = new Indulgence(suit, number);
        trick->addSubcard(card->getId());
    }else if(card->inherits("DelayedTrick"))
        return qobject_cast<const DelayedTrick *>(card);
    else if(card->isBlack() && (card->inherits("BasicCard") || card->inherits("EquipCard"))){
        trick = new SupplyShortage(suit, number);
        trick->addSubcard(card->getId());
    }

    return trick;
}

QString Weapon::getSubtype() const{
    return "weapon";
}

EquipCard::Location Weapon::location() const{
    return WeaponLocation;
}

void Weapon::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    Room *room = player->getRoom();
    if(range != 1)
        room->setPlayerProperty(player, "attack_range", range);
}

void Weapon::onUninstall(ServerPlayer *player) const{
    EquipCard::onUninstall(player);
    Room *room = player->getRoom();
    if(range != 1)
        room->setPlayerProperty(player, "attack_range", 1);
}

QString Armor::getSubtype() const{
    return "armor";
}

EquipCard::Location Armor::location() const{
    return ArmorLocation;
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

void Horse::onInstall(ServerPlayer *player) const{
    QString field;
    if(correct > 0)
        field = "equip_dest";
    else
        field = "equip_src";

    player->getRoom()->setPlayerCorrect(player, field, correct);
}

void Horse::onUninstall(ServerPlayer *player) const{
    QString field;
    if(correct > 0)
        field = "equip_dest";
    else
        field = "equip_src";

    player->getRoom()->setPlayerCorrect(player, field, 0);
}

EquipCard::Location Horse::location() const{
    if(correct > 0)
        return DefensiveHorseLocation;
    else
        return OffensiveHorseLocation;
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

    addCards();
    addGenerals();
}

ADD_PACKAGE(Standard)
