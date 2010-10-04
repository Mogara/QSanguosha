#include "standard.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "carditem.h"

Slash::Slash(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("slash");
    nature = DamageStruct::Normal;
}

DamageStruct::Nature Slash::getNature() const{
    return nature;
}

void Slash::setNature(DamageStruct::Nature nature){
    this->nature = nature;
}

bool Slash::IsAvailable(){
    if(Self->hasFlag("tianyi_failed"))
        return false;

    if(Self->hasSkill("paoxiao") || Self->hasWeapon("crossbow"))
        return true;
    else{
        int slash_count = ClientInstance->turn_tag.value("slash_count", 0).toInt();
        if(Self->hasFlag("tianyi_success"))
            return slash_count < 2;
        else
            return slash_count < 1;
    }
}

bool Slash::isAvailable() const{
    return IsAvailable();
}

QString Slash::getSubtype() const{
    return "attack_card";
}

void Slash::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->increaseSlashCount();
}

void Slash::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();

    SlashEffectStruct effect;
    effect.from = card_effect.from;
    effect.nature = nature;
    effect.slash = this;

    effect.to = card_effect.to;
    effect.drank = effect.from->hasFlag("drank");
    if(effect.drank)
        room->setPlayerFlag(effect.from, "-drank");

    room->slashEffect(effect);
}

bool Slash::targetsFeasible(const QList<const ClientPlayer *> &targets) const{   
    return !targets.isEmpty();
}

bool Slash::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    int slash_targets = 1;
    if(Self->hasWeapon("halberd") && Self->isLastHandCard(this)){
        slash_targets = 3;
    }

    if(Self->hasFlag("tianyi_success"))
        slash_targets ++;

    if(targets.length() >= slash_targets)
        return false;

    return Self->canSlash(to_select);
}

Jink::Jink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("jink");

    target_fixed = true;
}

QString Jink::getSubtype() const{
    return "defense_card";
}

bool Jink::isAvailable() const{
    return false;
}

Peach::Peach(Suit suit, int number):BasicCard(suit, number){    
    setObjectName("peach");
    target_fixed = true;
}

QString Peach::getSubtype() const{
    return "recover_card";
}

void Peach::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->recover(source, 1);
}

bool Peach::isAvailable() const{
    return Self->isWounded();
}

class Crossbow:public Weapon{
public:
    Crossbow(Suit suit, int number = 1):Weapon(suit, number, 1){
        setObjectName("crossbow");
    }
};

class DoubleSwordSkill: public WeaponSkill{
public:
    DoubleSwordSkill():WeaponSkill("double_sword"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();

        if(effect.from->getGeneral()->isMale() != effect.to->getGeneral()->isMale()){
            if(room->askForSkillInvoke(effect.from, objectName())){
                if(effect.to->isKongcheng() || !room->askForDiscard(effect.to, 1, true, false))
                    effect.from->drawCards(1);
            }
        }

        return false;
    }
};

class DoubleSword:public Weapon{
public:
    DoubleSword(Suit suit = Spade, int number = 2):Weapon(suit, number, 2){
        setObjectName("double_sword");
        skill = new DoubleSwordSkill;
    }
};

class QinggangSwordSkill: public WeaponSkill{
public:
    QinggangSwordSkill():WeaponSkill("qinggang_sword"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        room->setPlayerFlag(effect.to, "armor_nullified");

        return false;
    }
};

class QinggangSword:public Weapon{
public:
    QinggangSword(Suit suit = Spade, int number = 6):Weapon(suit, number, 2){
        setObjectName("qinggang_sword");

        skill = new QinggangSwordSkill;
    }
};

class BladeSkill : public WeaponSkill{
public:
    BladeSkill():WeaponSkill("blade"){
        events << SlashResult;
    }

    virtual int getPriority(ServerPlayer *target) const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashResultStruct result = data.value<SlashResultStruct>();
        if(!result.success){
            Room *room = player->getRoom();
            const Card *card = room->askForCard(player, "slash", "blade-slash");
            if(card){
                CardEffectStruct effect;
                effect.card = card;
                effect.from = player;
                effect.to = result.to;

                room->cardEffect(effect);
            }
        }

        return false;
    }
};

class Blade:public Weapon{
public:
    Blade(Suit suit = Spade, int number = 5):Weapon(suit, number, 3){
        setObjectName("blade");
        skill = new BladeSkill;
    }
};

class SpearSkill: public ViewAsSkill{
public:
    SpearSkill():ViewAsSkill("spear"){

    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "slash";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        const Card *first = cards.at(0)->getCard();
        const Card *second = cards.at(1)->getCard();

        Card::Suit suit = Card::NoSuit;
        if(first->isBlack() && second->isBlack())
            suit = Card::Spade;
        else if(first->isRed() && second->isRed())
            suit = Card::Heart;

        Slash *slash = new Slash(suit, 0);
        slash->setSkillName(objectName());
        slash->addSubcard(first->getId());
        slash->addSubcard(second->getId());

        return slash;
    }
};

class Spear:public Weapon{
public:
    Spear(Suit suit = Spade, int number = 12):Weapon(suit, number, 3){
        setObjectName("spear");
        attach_skill = true;
    }
};

class AxeViewAsSkill: public ViewAsSkill{
public:
    AxeViewAsSkill():ViewAsSkill("axe"){

    }

    bool isEnabledAtPlay() const{
        return false;
    }

    bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@axe-card";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;

        if(to_select->getCard() == Self->getWeapon())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        DummyCard *card = new DummyCard;
        card->addSubcards(cards);
        return card;
    }
};

class AxeSkill: public WeaponSkill{
public:
    AxeSkill():WeaponSkill("axe"){
        events << SlashResult;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashResultStruct result = data.value<SlashResultStruct>();
        if(!result.success){
            Room *room = player->getRoom();            
            if(room->askForCard(player, "@axe-card", "axe-card")){
                result.success = true;
                data = QVariant::fromValue(result);
            }
        }

        return false;
    }
};

class Axe:public Weapon{
public:
    Axe(Suit suit = Diamond, int number = 5):Weapon(suit, number, 3){
        setObjectName("axe");
        skill = new AxeSkill;
        attach_skill = true;
    }
};

class Halberd:public Weapon{
public:
    Halberd(Suit suit = Diamond, int number = 12):Weapon(suit, number, 4){
        setObjectName("halberd");
    }
};

class KylinBowSkill: public WeaponSkill{
public:
    KylinBowSkill():WeaponSkill("kylin_bow"){
        events << SlashResult;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashResultStruct result = data.value<SlashResultStruct>();
        if(result.success){
            QStringList horses;
            if(result.to->getDefensiveHorse())
                horses << "dhorse";
            if(result.to->getOffensiveHorse())
                horses << "ohorse";

            if(!horses.isEmpty()){
                Room *room = player->getRoom();
                QString horse_type = room->askForChoice(player, objectName(), horses.join("+") + "+no");
                if(horse_type == "dhorse")
                    room->throwCard(result.to->getDefensiveHorse());
                else if(horse_type == "ohorse")
                    room->throwCard(result.to->getOffensiveHorse());
            }
        }

        return false;
    }
};

class KylinBow:public Weapon{
public:
    KylinBow(Suit suit = Heart, int number = 5):Weapon(suit, number, 5){
        setObjectName("kylin_bow");
        skill = new KylinBowSkill;
    }
};

class EightDiagramSkill: public ArmorSkill{
public:
    EightDiagramSkill():ArmorSkill("eight_diagram"){
        events << CardAsked;
    }

    virtual int getPriority(ServerPlayer *) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if(asked == "jink"){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName())){
                const Card *card = room->getJudgeCard(player);
                if(card->isRed()){
                    Jink *jink = new Jink(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);
                    return true;
                }
            }
        }
        return false;
    }
};

class EightDiagram:public Armor{
public:
    EightDiagram(Suit suit, int number = 2):Armor(suit, number){
        setObjectName("eight_diagram");
        skill = new EightDiagramSkill;
    }
};

class AmazingGrace:public GlobalEffect{
public:
    AmazingGrace(Suit suit, int number):GlobalEffect(suit, number){
        setObjectName("amazing_grace");
    }

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
        room->throwCard(this);

        QList<ServerPlayer *> players = room->getAllPlayers();
        QList<int> card_ids = room->getNCards(players.length());
        QStringList card_str;
        foreach(int card_id, card_ids)
            card_str << QString::number(card_id);
        room->broadcastInvoke("fillAG", card_str.join("+"));

        foreach(ServerPlayer *player, players){
            bool can_take = room->cardEffect(this, source, player);
            if(can_take){
                int card_id = room->askForAG(player, card_ids);
                card_ids.removeOne(card_id);

                // these code is quick-and-dirty
                player->addCard(Sanguosha->getCard(card_id), Player::Hand);
                room->setCardMapping(card_id, player, Player::Hand);

                LogMessage log;
                log.type = "$TakeAG";
                log.from = player;
                log.card_str = QString::number(card_id);
                room->sendLog(log);

                room->broadcastInvoke("takeAG", QString("%1:%2").arg(player->objectName()).arg(card_id));
            }
        }

        room->broadcastInvoke("clearAG");
    }
};

GodSalvation::GodSalvation(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("god_salvation");
}

void GodSalvation::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    QList<ServerPlayer *> all_players = room->getAllPlayers();
    foreach(ServerPlayer *player, all_players){
        if(player->isWounded()){
            room->cardEffect(this, source, player);
        }
    }
}

void GodSalvation::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->recover(effect.to, 1);
}

SavageAssault::SavageAssault(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("savage_assault");
}

void SavageAssault::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *slash = room->askForCard(effect.to, "slash", "savage-assault-slash:" + effect.from->getGeneralName());
    if(slash == NULL){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        damage.from = effect.from;
        room->damage(damage);
    }
}

ArcheryAttack::ArcheryAttack(Card::Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("archery_attack");
}

void ArcheryAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *jink = room->askForCard(effect.to, "jink", "archery-attack-jink:" + effect.from->getGeneralName());
    if(jink == NULL){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }
}

void SingleTargetTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = targets.first();

    room->cardEffect(effect);
}


Collateral::Collateral(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("collateral");
}

bool Collateral::isAvailable() const{
    QList<const ClientPlayer*> players = ClientInstance->findChildren<const ClientPlayer*>();
    foreach(const ClientPlayer *player, players){
        if(player->getWeapon() != NULL && player != Self)
            return true;
    }

    return false;
}

bool Collateral::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

bool Collateral::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.isEmpty()){
        if(to_select->hasSkill("weimu") && isBlack())
            return false;

        return to_select->getWeapon() && to_select != Self;
    }else if(targets.length() == 1){
        const ClientPlayer *first = targets.first();
        return first != Self && first->canSlash(to_select);
    }else
        return false;
}

void Collateral::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *killer = targets.at(0);
    ServerPlayer *victim = targets.at(1);
    const Weapon *weapon = killer->getWeapon();

    if(weapon == NULL)
        return;

    bool on_effect = room->cardEffect(this, source, killer);
    if(on_effect){
        QString prompt = QString("collateral-slash:%1:%2").arg(source->getGeneralName()).arg(victim->getGeneralName());
        const Card *slash = room->askForCard(killer, "slash", prompt);
        if(slash){
            room->cardEffect(slash, killer, victim);
        }else{
            source->obtainCard(weapon);
        }
    }
}

Nullification::Nullification(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("nullification");
}

bool Nullification::isAvailable() const{
    return false;
}

ExNihilo::ExNihilo(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("ex_nihilo");
    target_fixed = true;
}

void ExNihilo::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    CardEffectStruct effect;
    effect.from = effect.to = source;
    effect.card = this;

    room->cardEffect(effect);
}

void ExNihilo::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

Duel::Duel(Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("duel");
}

bool Duel::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select->hasSkill("weimu") && isBlack())
        return false;

    if(to_select == Self)
        return false;

    return targets.isEmpty();
}

void Duel::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    forever{
        if(second->hasSkill("wushuang")){
            room->playSkillEffect("wushuang");
            const Card *slash = room->askForCard(first, "slash", "@wushuang-slash-1:" + second->getGeneralName());
            if(slash == NULL)
                break;

            slash = room->askForCard(first, "slash", "@wushuang-slash-2:" + second->getGeneralName());
            if(slash == NULL)
                break;

        }else{
            const Card *slash = room->askForCard(first, "slash", "duel-slash:" + second->getGeneralName());
            if(slash == NULL)
                break;
        }

        qSwap(first, second);
    }

    DamageStruct damage;
    damage.card = this;
    damage.damage = 1;
    damage.from = second;
    damage.to = first;

    room->damage(damage);
}

Snatch::Snatch(Suit suit, int number):SingleTargetTrick(suit, number, true) {
    setObjectName("snatch");
}

bool Snatch::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("qianxun"))
        return false;

    if(to_select->hasSkill("weimu") && isBlack())
        return false;

    if(to_select->isAllNude())
        return false;

    if(to_select == Self)
        return false;

    if(Self->distanceTo(to_select) > 1 && !Self->hasSkill("qicai"))
        return false;

    return true;
}

void Snatch::onEffect(const CardEffectStruct &effect) const{
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());

    if(room->getCardPlace(card_id) == Player::Hand)
        room->moveCardTo(Sanguosha->getCard(card_id), effect.from, Player::Hand, false);
    else
        room->obtainCard(effect.from, card_id);
}

Dismantlement::Dismantlement(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("dismantlement");
}

bool Dismantlement::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("weimu") && isBlack())
        return false;

    if(to_select->isAllNude())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Dismantlement::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    room->throwCard(card_id);
}

Indulgence::Indulgence(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("indulgence");
    target_fixed = false;
}

bool Indulgence::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("qianxun"))
        return false;

    if(isBlack() && to_select->hasSkill("weimu"))
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Indulgence::takeEffect(ServerPlayer *target) const{
    target->getRoom()->skip(Player::Play);
}

bool Indulgence::judge(const Card *card) const{
    return card->getSuit() != Card::Heart;
}


Lightning::Lightning(Suit suit, int number):DelayedTrick(suit, number, true){
    setObjectName("lightning");
    target_fixed = true;
}

void Lightning::takeEffect(ServerPlayer *target) const{
    DamageStruct damage;
    damage.card = this;
    damage.damage = 3;
    damage.from = NULL;
    damage.to = target;
    damage.nature = DamageStruct::Thunder;

    target->getRoom()->damage(damage);
}

bool Lightning::judge(const Card *card) const{
    return card->getSuit() == Card::Spade && card->getNumber() >= 2 && card->getNumber() <= 9;
}

bool Lightning::isAvailable() const{
    if(Self->containsTrick(objectName()))
        return false;

    if(Self->hasSkill("weimu") && isBlack())
        return false;

    return true;
}

void Lightning::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->moveCardTo(this, source, Player::Judging);
}

class IceSwordSkill: public TriggerSkill{
public:
    IceSwordSkill():TriggerSkill("ice_sword"){
        events << SlashResult;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasWeapon(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashResultStruct result = data.value<SlashResultStruct>();

        if(result.success){
            Room *room = player->getRoom();

            if(!result.to->isNude() && room->askForSkillInvoke(player, "ice_sword")){
                int card_id = room->askForCardChosen(player, result.to, "he", "ice_sword");
                room->throwCard(card_id);

                if(!result.to->isNude()){
                    card_id = room->askForCardChosen(player, result.to, "he", "ice_sword");
                    room->throwCard(card_id);
                }

                return true;
            }
        }

        return false;
    }
};

IceSword::IceSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("ice_sword");
    skill = new IceSwordSkill;
}

class RenwangShieldSkill: public ArmorSkill{
public:
    RenwangShieldSkill():ArmorSkill("renwang_shield"){
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isBlack()){
            LogMessage log;
            log.type = "#ArmorNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }
};

RenwangShield::RenwangShield(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("renwang_shield");
    skill = new RenwangShieldSkill;
}

void StandardPackage::addCards(){
    QList<Card*> cards;

    cards << new Slash(Card::Spade, 7)
          << new Slash(Card::Spade, 8)
          << new Slash(Card::Spade, 8)
          << new Slash(Card::Spade, 9)
          << new Slash(Card::Spade, 9)
          << new Slash(Card::Spade, 10)
          << new Slash(Card::Spade, 10)

          << new Slash(Card::Club, 2)
          << new Slash(Card::Club, 3)
          << new Slash(Card::Club, 4)
          << new Slash(Card::Club, 5)
          << new Slash(Card::Club, 6)
          << new Slash(Card::Club, 7)
          << new Slash(Card::Club, 8)
          << new Slash(Card::Club, 8)
          << new Slash(Card::Club, 9)
          << new Slash(Card::Club, 9)
          << new Slash(Card::Club, 10)
          << new Slash(Card::Club, 10)
          << new Slash(Card::Club, 11)
          << new Slash(Card::Club, 11)

          << new Slash(Card::Heart, 10)
          << new Slash(Card::Heart, 10)
          << new Slash(Card::Heart, 11)

          << new Slash(Card::Diamond, 6)
          << new Slash(Card::Diamond, 7)
          << new Slash(Card::Diamond, 8)
          << new Slash(Card::Diamond, 9)
          << new Slash(Card::Diamond, 10)
          << new Slash(Card::Diamond, 13)

          << new Jink(Card::Heart, 2)
          << new Jink(Card::Heart, 2)
          << new Jink(Card::Heart, 13)

          << new Jink(Card::Diamond, 2)
          << new Jink(Card::Diamond, 2)
          << new Jink(Card::Diamond, 3)
          << new Jink(Card::Diamond, 4)
          << new Jink(Card::Diamond, 5)
          << new Jink(Card::Diamond, 6)
          << new Jink(Card::Diamond, 7)
          << new Jink(Card::Diamond, 8)
          << new Jink(Card::Diamond, 9)
          << new Jink(Card::Diamond, 10)
          << new Jink(Card::Diamond, 11)
          << new Jink(Card::Diamond, 11)

          << new Peach(Card::Heart, 3)
          << new Peach(Card::Heart, 4)
          << new Peach(Card::Heart, 6)
          << new Peach(Card::Heart, 7)
          << new Peach(Card::Heart, 8)
          << new Peach(Card::Heart, 9)
          << new Peach(Card::Heart, 12)

          << new Peach(Card::Diamond, 12)

          << new Crossbow(Card::Club)
          << new Crossbow(Card::Diamond)
          << new DoubleSword
          << new QinggangSword
          << new Blade
          << new Spear
          << new Axe
          << new Halberd
          << new KylinBow

          << new EightDiagram(Card::Spade)
          << new EightDiagram(Card::Club)

          << new Horse("jueying", Card::Spade, 5, +1)
          << new Horse("dilu", Card::Club, 5, +1)
          << new Horse("zhuahuangfeidian", Card::Heart, 13, +1)
          << new Horse("chitu", Card::Heart, 5, -1)
          << new Horse("dayuan", Card::Spade, 13, -1)
          << new Horse("zixing", Card::Diamond, 13, -1)

          << new AmazingGrace(Card::Heart, 3)
          << new AmazingGrace(Card::Heart, 4)
          << new GodSalvation
          << new SavageAssault(Card::Spade, 7)
          << new SavageAssault(Card::Spade, 13)
          << new SavageAssault(Card::Club, 7)
          << new ArcheryAttack
          << new Duel(Card::Spade, 1)
          << new Duel(Card::Club, 1)
          << new Duel(Card::Diamond, 1)
          << new ExNihilo(Card::Heart, 7)
          << new ExNihilo(Card::Heart, 8)
          << new ExNihilo(Card::Heart, 9)
          << new ExNihilo(Card::Heart, 11)
          << new Snatch(Card::Spade, 3)
          << new Snatch(Card::Spade, 4)
          << new Snatch(Card::Spade, 11)
          << new Snatch(Card::Diamond, 3)
          << new Snatch(Card::Diamond, 4)
          << new Dismantlement(Card::Spade, 3)
          << new Dismantlement(Card::Spade, 4)
          << new Dismantlement(Card::Spade, 12)
          << new Dismantlement(Card::Club, 3)
          << new Dismantlement(Card::Club, 4)
          << new Dismantlement(Card::Heart, 12)
          << new Collateral(Card::Club, 12)
          << new Collateral(Card::Club, 13)
          << new Nullification(Card::Spade, 11)
          << new Nullification(Card::Club, 12)
          << new Nullification(Card::Club, 13)
          << new Indulgence(Card::Spade, 6)
          << new Indulgence(Card::Club, 6)
          << new Indulgence(Card::Heart, 6)
          << new Lightning(Card::Spade, 1)

          // EX cards
          << new IceSword(Card::Spade, 2)
          << new RenwangShield(Card::Club, 2)
          << new Lightning(Card::Heart, 12)
          << new Nullification(Card::Diamond, 12);

    foreach(Card *card, cards)
        card->setParent(this);

    t["basic"] = tr("basic");
    t["trick"] = tr("trick");
    t["equip"] = tr("equip");
    t["attack_card"] = tr("attack_card");
    t["defense_card"] = tr("defense_card");
    t["recover_card"] = tr("recover_card");
    t["disgusting_card"] = tr("disgusting_card");
    t["global_effect"] = tr("global_effect");
    t["aoe"] = tr("aoe");
    t["single_target_trick"] = tr("single_target_trick");
    t["delayed_trick"] = tr("delayed_trick");
    t["weapon"] = tr("weapon");
    t["armor"] = tr("armor");
    t["defensive_horse"] = tr("defensive_horse");
    t["offensive_horse"] = tr("offensive_horse");

    t["slash"] = tr("slash");
    t["jink"] = tr("jink");
    t["peach"] = tr("peach");


    t["crossbow"] = tr("crossbow");
    t["double_sword"] = tr("double_sword");
    t["qinggang_sword"] = tr("qinggang_sword");

    t["blade"] = tr("blade");
    t["spear"] = tr("spear");
    t["axe"] = tr("axe");
    t["halberd"] = tr("halberd");
    t["kylin_bow"] = tr("kylin_bow");

    t["eight_diagram"] = tr("eight_diagram");

    // ex
    t["renwang_shield"] = tr("renwang_shield");
    t["ice_sword"] = tr("ice_sword");

    t["jueying"] = tr("jueying");
    t["dilu"] = tr("dilu");
    t["zhuahuangfeidian"] = tr("zhuahuangfeidian");
    t["chitu"] = tr("chitu");
    t["dayuan"] = tr("dayuan");
    t["zixing"] = tr("zixing");

    t["amazing_grace"] = tr("amazing_grace");
    t["god_salvation"] = tr("god_salvation");
    t["savage_assault"] = tr("savage_assault");
    t["archery_attack"] = tr("archery_attack");
    t["collateral"] = tr("collateral");
    t["duel"] = tr("duel");
    t["ex_nihilo"] = tr("ex_nihilo");
    t["snatch"] = tr("snatch");
    t["dismantlement"] = tr("dismantlement");
    t["collateral"] = tr("collateral");
    t["nullification"] = tr("nullification");
    t["indulgence"] = tr("indulgence");
    t["lightning"] = tr("lightning");

    t["eight_diagram:yes"] = tr("eight_diagram:yes");

    t["slash-jink"] = tr("slash-jink");
    t["duel-slash"] = tr("duel-slash");
    t["savage-assault-slash"] = tr("savage-assault-slash");
    t["archery-attack-jink"] = tr("archery-attack-jink");
    t["collateral-slash"] = tr("collateral-slash");
    t["blade-slash"] = tr("blade-slash");
    t["double-sword-card"] = tr("double-sword-card");
    t["axe-card"] = tr("axe-card");

    // weapon prompt
    t["double_sword:yes"] = tr("double_sword:yes");
    t["ice_sword:yes"] = tr("ice_sword:yes");
    t["kylin_bow:dhorse"] = tr("kylin_bow:dhorse");
    t["kylin_bow:ohorse"] = tr("kylin_bow:ohorse");
    t["kylin_bow:no"] = tr("kylin_bow:no");

    skills << new SpearSkill << new AxeViewAsSkill;

    // translation for log

    t["#Slash"] = tr("#Slash");
    t["#Jink"] = tr("#Jink");
    t["#Peach"] = tr("#Peach");
}
