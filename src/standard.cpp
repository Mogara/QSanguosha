#include "standard.h"
#include "serverplayer.h"
#include "room.h"
#include "skill.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"

QString BasicCard::getType() const{
    return "basic";
}

Card::CardType BasicCard::getTypeId() const{
    return Basic;
}

TrickCard::TrickCard(Suit suit, int number, bool aggressive)
    :Card(suit, number), aggressive(aggressive),
    cancelable(true)
{
}

bool TrickCard::isAggressive() const{
    return aggressive;
}

void TrickCard::setCancelable(bool cancelable){
    this->cancelable = cancelable;
}

QString TrickCard::getType() const{
    return "trick";
}

Card::CardType TrickCard::getTypeId() const{
    return Trick;
}

bool TrickCard::isCancelable(const CardEffectStruct &effect) const{
    return cancelable;
}

TriggerSkill *EquipCard::getSkill() const{
    return skill;
}

QString EquipCard::getType() const{
    return "equip";
}

Card::CardType EquipCard::getTypeId() const{
    return Equip;
}

QString EquipCard::getEffectPath(bool is_male) const{
    return "audio/card/common/equip.ogg";
}

void EquipCard::onUse(Room *room, const CardUseStruct &card_use) const{
    if(card_use.to.isEmpty()){
        ServerPlayer *player = card_use.from;

        QVariant data = QVariant::fromValue(card_use);
        RoomThread *thread = room->getThread();
        thread->trigger(CardUsed, player, data);

        thread->trigger(CardFinished, player, data);
    }else
        Card::onUse(room, card_use);
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

    LogMessage log;
    log.from = source;
    log.type = "$Install";
    log.card_str = QString::number(getEffectiveId());
    room->sendLog(log);

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
    QList<ServerPlayer *> all_players = room->getAllPlayers();
    TrickCard::use(room, source, all_players);
}

QString AOE::getSubtype() const{
    return "aoe";
}

bool AOE::isAvailable(const Player *player) const{
    QList<const Player *> players = player->parent()->findChildren<const Player *>();
    foreach(const Player *p, players){
        if(p == player)
            continue;

        if(p->isDead())
            continue;

        if(player->isProhibited(p, this))
            continue;

        return true;
    }

    return false;
}

void AOE::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> targets;
    QList<ServerPlayer *> other_players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, other_players){
        const ProhibitSkill *skill = room->isProhibited(source, player, this);
        if(skill){
            LogMessage log;
            log.type = "#SkillAvoid";
            log.from = player;
            log.arg = skill->objectName();
            log.arg2 = objectName();
            room->sendLog(log);

            room->playSkillEffect(skill->objectName());
        }else
            targets << player;
    }

    TrickCard::use(room, source, targets);
}

QString SingleTargetTrick::getSubtype() const{
    return "single_target_trick";
}

bool SingleTargetTrick::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

DelayedTrick::DelayedTrick(Suit suit, int number, bool movable)
    :TrickCard(suit, number, true), movable(movable)
{
}

void DelayedTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(target_fixed)
        room->moveCardTo(this, source, Player::Judging, true);
    else{
        ServerPlayer *target = targets.first();
        room->moveCardTo(this, target, Player::Judging, true);
    }
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

    JudgeStruct judge_struct = judge;
    judge_struct.who = effect.to;
    room->judge(judge_struct);

    if(judge_struct.isBad()){
        room->throwCard(this);
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

            if(room->isProhibited(target, player, this))
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

int Weapon::getRange() const{
    return range;
}

QString Weapon::getSubtype() const{
    return "weapon";
}

EquipCard::Location Weapon::location() const{
    return WeaponLocation;
}

QString Weapon::label() const{
    return QString("%1(%2)").arg(getName()).arg(range);
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

QString Armor::label() const{
    return getName();
}

Horse::Horse(Suit suit, int number, int correct)
    :EquipCard(suit, number), correct(correct)
{
}

int Horse::getCorrect() const{
    return correct;
}

QString Horse::getEffectPath(bool) const{
    return "audio/card/common/horse.ogg";
}

void Horse::onInstall(ServerPlayer *) const{

}

void Horse::onUninstall(ServerPlayer *) const{

}

QString Horse::label() const{
    QString format;

    if(correct > 0)
        format = "%1(+%2)";
    else
        format = "%1(%2)";

    return format.arg(getName()).arg(correct);
}

OffensiveHorse::OffensiveHorse(Card::Suit suit, int number, int correct)
    :Horse(suit, number, correct)
{

}

QString OffensiveHorse::getSubtype() const{
    return "offensive_horse";
}

DefensiveHorse::DefensiveHorse(Card::Suit suit, int number, int correct)
    :Horse(suit, number, correct)
{

}

QString DefensiveHorse::getSubtype() const{
    return "defensive_horse";
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
    addCards();
    addGenerals();
}

ADD_PACKAGE(Standard)
