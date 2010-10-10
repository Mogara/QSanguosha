#include "standard.h"
#include "serverplayer.h"
#include "room.h"
#include "skill.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "engine.h"

QString BasicCard::getType() const{
    return "basic";
}

int BasicCard::getTypeId() const{
    return 0;
}

bool TrickCard::isAggressive() const{
    return aggressive;
}

QString TrickCard::getType() const{
    return "trick";
}

int TrickCard::getTypeId() const{
    return 1;
}

TriggerSkill *EquipCard::getSkill() const{
    return skill;
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

    room->moveCardTo(this, source, Player::Equip, true);
}

void EquipCard::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void EquipCard::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

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
        if(isBlack() && player->hasSkill("weimu"))
            continue;

        if(player->isDead())
            continue;

        room->cardEffect(this, source, player);
    }
}

QString SingleTargetTrick::getSubtype() const{
    return "single_target_trick";
}

bool SingleTargetTrick::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(isBlack() && to_select->hasSkill("weimu"))
        return false;
    else
        return true;
}

DelayedTrick::DelayedTrick(Suit suit, int number, bool movable)
    :TrickCard(suit, number, true), movable(movable)
{
}

void DelayedTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Judging, true);
}

QString DelayedTrick::getSubtype() const{
    return "delayed_trick";
}

void DelayedTrick::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(!movable)
        room->throwCard(this);

    LogMessage log;
    log.from = effect.to;
    log.type = "#DelayedTrick";
    log.arg = effect.card->objectName();
    room->sendLog(log);

    const Card *card = room->getJudgeCard(effect.to);
    if(judge(card)){
        takeEffect(effect.to);
    }else if(movable){
        onNullified(effect.to);
    }
}

void DelayedTrick::onNullified(ServerPlayer *target) const{
    Room *room = target->getRoom();
    if(movable){
        QList<ServerPlayer *> players = room->getOtherPlayers(target);
        players << target;

        foreach(ServerPlayer *player, players){            
            if(player->containsTrick(objectName()))
                continue;

            if(player->hasSkill("weimu") && isBlack())
                continue;

            room->moveCardTo(this, player, Player::Judging, true);
            break;
        }
    }else
        room->throwCard(this);
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

Weapon::Weapon(Suit suit, int number, int range)
    :EquipCard(suit, number), range(range), attach_skill(false)
{
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
    room->setPlayerProperty(player, "atk", range);

    if(attach_skill)
        room->attachSkillToPlayer(player, objectName());
}

void Weapon::onUninstall(ServerPlayer *player) const{
    EquipCard::onUninstall(player);
    Room *room = player->getRoom();
    room->setPlayerProperty(player, "atk", 1);

    if(attach_skill)
        room->detachSkillFromPlayer(player, objectName());
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
    Room *room = player->getRoom();
    if(correct > 0)
        room->setPlayerCorrect(player, "P");
    else
        room->setPlayerCorrect(player, "S");
}

void Horse::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    if(correct > 0)
        room->setPlayerCorrect(player, "-P");
    else
        room->setPlayerCorrect(player, "-S");
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

    // card suit
    t["spade"] = tr("spade");
    t["club"] = tr("club");
    t["heart"] = tr("heart");
    t["diamond"] = tr("diamond");

    t["spade_char"] = tr("spade_char");
    t["club_char"] = tr("club_char");
    t["heart_char"] = tr("heart_char");
    t["diamond_char"] = tr("diamond_char");
    t["no_suit_char"] = tr("no_suit_char");

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
    t["trust"] = tr("trust");

    // normal phrases
    t["yes"] = tr("yes");
    t["no"] = tr("no");

    // damage nature
    t["normal_nature"] = tr("normal_nature");
    t["fire_nature"] = tr("fire_nature");
    t["thunder_nature"] = tr("thunder_nature");

    // log translation
    t["#ChooseSuit"] = tr("#ChooseSuit");
    t["#Death"] = tr("#Death");
    t["#InvokeSkill"] = tr("#InvokeSkill");
    t["#Pindian"] = tr("#Pindian");
    t["#PindianSuccess"] = tr("#PindianSuccess");
    t["#PindianFailure"] = tr("#PindianFailure");
    t["#Damage"] = tr("#Damage");
    t["#DamageNoSource"] = tr("#DamageNoSource");
    t["#DelayedTrick"] = tr("#DelayedTrick");
    t["#SkillNullify"] = tr("#SkillNullify");
    t["#ArmorNullify"] = tr("#ArmorNullify");
    t["#DrawNCards"] = tr("#DrawNCards");
    t["#MoveNCards"] = tr("#MoveNCards");    

    t["$JudgeResult"] = tr("$JudgeResult");
    t["$InitialJudge"] = tr("$InitialJudge");
    t["$ChangedJudge"] = tr("$ChangedJudge");

    t["$TakeAG"] = tr("$TakeAG");
    t["$Uninstall"] = tr("$Uninstall");
    t["$PindianResult"] = tr("$PindianResult");
    t["$MoveCard"] = tr("$MoveCard");
    t["$PasteCard"] = tr("$PasteCard");
    t["$LightningMove"] = tr("$LightningMove");
    t["$DiscardCard"] = tr("$DiscardCard");
    t["$RecycleCard"] = tr("$RecycleCard");
    t["$ShowCard"] = tr("$ShowCard");

    addCards();
    addGenerals();
    addAIs();
}

ADD_PACKAGE(Standard)
