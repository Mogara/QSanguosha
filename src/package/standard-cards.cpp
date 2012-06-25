#include "standard.h"
#include "standard-equips.h"
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

bool Slash::IsAvailable(const Player *player){
    if(player->hasFlag("tianyi_failed") || player->hasFlag("xianzhen_failed"))
        return false;

    return player->hasWeapon("crossbow") || player->canSlashWithoutCrossbow();
}

bool Slash::isAvailable(const Player *player) const{
    return IsAvailable(player) && BasicCard::isAvailable(player);;
}

QString Slash::getSubtype() const{
    return "attack_card";
}

void Slash::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(source->getPhase() == Player::Play
            && source->hasUsed("Slash")
            && source->hasWeapon("crossbow"))
        room->setEmotion(source,"weapon/crossbow");
    else if(this->isVirtualCard() && this->skill_name == "spear")
        room->setEmotion(source,"weapon/spear");
    else if(targets.length()>1
            && source->handCards().size() == 0
            && source->hasWeapon("halberd"))
        room->setEmotion(source,"weapon/halberd");
    else if(this->isVirtualCard() && this->skill_name == "fan")
        room->setEmotion(source,"weapon/fan");

    BasicCard::use(room, source, targets);

    if(source->hasFlag("drank")){
        LogMessage log;
        log.type = "#UnsetDrank";
        log.from = source;
        room->sendLog(log);
    }
}

void Slash::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();
    if(card_effect.from->hasFlag("drank")){
        room->setCardFlag(this, "drank");
        room->setPlayerFlag(card_effect.from, "-drank");
    }

    SlashEffectStruct effect;
    effect.from = card_effect.from;
    effect.nature = nature;
    effect.slash = this;

    effect.to = card_effect.to;
    effect.drank = this->hasFlag("drank");

    room->slashEffect(effect);
}

bool Slash::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return !targets.isEmpty();
}

bool Slash::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int slash_targets = 1;
    if(Self->hasWeapon("halberd") && Self->isLastHandCard(this)){
        slash_targets = 3;
    }
    bool distance_limit = true;
    int rangefix = 0;
    if(isVirtualCard() && getSubcards().length() > 0){
        foreach(int card_id, getSubcards()){
            if(Self->getWeapon())
                if(card_id == Self->getWeapon()->getId())
                    if(Self->getWeapon()->getRange() > 1)
                        rangefix = qMax((Self->getWeapon()->getRange()), rangefix);
            if(Self->getOffensiveHorse())
                if(card_id == Self->getOffensiveHorse()->getId())
                    rangefix = qMax(rangefix, 1);
        }
    }
    if(Self->hasFlag("tianyi_success")){
        distance_limit = false;
        slash_targets ++;
    }

    if(Self->hasSkill("lihuo") && inherits("FireSlash"))
        slash_targets ++;

    if(Self->hasSkill("shenji") && Self->getWeapon() == NULL)
        slash_targets = 3;

    if(targets.length() >= slash_targets)
        return false;

    if(inherits("WushenSlash")){
        distance_limit = false;
    }

    return Self->canSlash(to_select, distance_limit, rangefix);
}

Jink::Jink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("jink");

    target_fixed = true;
}

QString Jink::getSubtype() const{
    return "defense_card";
}

bool Jink::isAvailable(const Player *) const{
    return false;
}

Peach::Peach(Suit suit, int number):BasicCard(suit, number){
    setObjectName("peach");
    target_fixed = true;
}

QString Peach::getSubtype() const{
    return "recover_card";
}

void Peach::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    BasicCard::use(room, source, targets);
    if(targets.isEmpty())
        room->cardEffect(this, source, source);   
}

void Peach::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    room->broadcastInvoke("animate", QString("peach:%1:%2")
        .arg(effect.from->objectName())
        .arg(effect.to->objectName()));

    // recover hp
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    if(hasFlag("sweet")){
        room->setCardFlag(this, "-sweet");
        recover.recover = 2;

        LogMessage log;
        log.type = "#JiuyuanExtraRecover";
        log.from = effect.to;
        log.to << effect.from;
        log.arg = objectName();
        room->sendLog(log);
        if(effect.from->getGender() == effect.to->getGender())
            room->broadcastSkillInvoke("jiuyuan", 2);
        else
            room->broadcastSkillInvoke("jiuyuan", 3);
    }

    room->recover(effect.to, recover);
}

bool Peach::isAvailable(const Player *player) const{
    return player->isWounded();
}

Crossbow::Crossbow(Suit suit, int number)
    :Weapon(suit, number, 1)
{
    setObjectName("crossbow");
}

class DoubleSwordSkill: public WeaponSkill{
public:
    DoubleSwordSkill():WeaponSkill("double_sword"){
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.from->objectName() != player->objectName())
            return false;
        foreach(ServerPlayer *to, use.to){
            if(use.from->getGeneral()->isMale() != to->getGeneral()->isMale()
                && use.card->inherits("Slash")){
                if(use.from->askForSkillInvoke(objectName())){
                    to->getRoom()->setEmotion(use.from,"weapon/double_sword");
                    bool draw_card = false;

                    if(to->isKongcheng())
                        draw_card = true;
                    else{
                        QString prompt = "double-sword-card:" + use.from->getGeneralName();
                        const Card *card = room->askForCard(to, ".", prompt, QVariant(), CardDiscarded);
                        if(card){
                            room->throwCard(card, to);
                        }else
                            draw_card = true;
                    }

                    if(draw_card)
                       use.from->drawCards(1);
                }
            }
        }

        return false;
    }
};

DoubleSword::DoubleSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("double_sword");
    skill = new DoubleSwordSkill;
}

class QinggangSwordSkill: public WeaponSkill{
public:
    QinggangSwordSkill():WeaponSkill("qinggang_sword"){
        events << TargetConfirmed << Death;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *, QVariant &data) const{
        if(event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("Slash")){
                bool do_anim = false;
                foreach(ServerPlayer *p, use.to){
                    p->addMark("qinggang");
                    if (p->getArmor()  || p->hasSkill("bazhen"))  do_anim = true;
                }
                if (do_anim){
                    room->setEmotion(use.from, "weapon/qinggang_sword");
                }
            }
        }
        else{
            foreach(ServerPlayer *p,room->getAlivePlayers())
                p->setMark("qinggang", 0);
        }
        return false;
    }
};

QinggangSword::QinggangSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("qinggang_sword");

    skill = new QinggangSwordSkill;
}

class BladeSkill : public WeaponSkill{
public:
    BladeSkill():WeaponSkill("blade"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();


        if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
            return false;

        const Card *card = room->askForCard(player, "slash", "blade-slash:" + effect.to->objectName(), QVariant(), CardUsed);
        if(card){
            // if player is drank, unset his flag
            if(player->hasFlag("drank"))
                room->setPlayerFlag(player, "-drank");

            CardUseStruct use;
            use.card = card;
            use.from = player;
            use.to << effect.to;
            room->useCard(use, false);
            room->setEmotion(player,"weapon/blade");
        }

        return false;
    }
};

Blade::Blade(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("blade");
    skill = new BladeSkill;
}

class SpearSkill: public ViewAsSkill{
public:
    SpearSkill():ViewAsSkill("spear"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        const Card *first = cards.at(0)->getFilteredCard();
        const Card *second = cards.at(1)->getFilteredCard();

        Card::Suit suit = Card::NoSuit;
        if(first->isBlack() && second->isBlack())
            suit = Card::Club;
        else if(first->isRed() && second->isRed())
            suit = Card::Diamond;

        Slash *slash = new Slash(suit, 0);
        slash->setSkillName(objectName());
        slash->addSubcard(first);
        slash->addSubcard(second);

        return slash;
    }
};

Spear::Spear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("spear");
    attach_skill = true;
}

class AxeViewAsSkill: public ViewAsSkill{
public:
    AxeViewAsSkill():ViewAsSkill("axe"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@axe";
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
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class AxeSkill: public WeaponSkill{
public:
    AxeSkill():WeaponSkill("axe"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        CardStar card = room->askForCard(player, "@axe", "@axe:" + effect.to->objectName(),data, CardDiscarded);
        if(card){
            room->setEmotion(player,"weapon/axe");

            QList<int> card_ids = card->getSubcards();
            foreach(int card_id, card_ids){
                LogMessage log;
                log.type = "$DiscardCard";
                log.from = player;
                log.card_str = QString::number(card_id);

                room->sendLog(log);
            }

            LogMessage log;
            log.type = "#AxeSkill";
            log.from = player;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
        }

        return false;
    }
};

Axe::Axe(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("axe");
    skill = new AxeSkill;
    attach_skill = true;
}

Halberd::Halberd(Suit suit, int number)
    :Weapon(suit, number, 4)
{
    setObjectName("halberd");
}

class KylinBowSkill: public WeaponSkill{
public:
    KylinBowSkill():WeaponSkill("kylin_bow"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        QStringList horses;
        if(damage.card && damage.card->inherits("Slash") && !damage.chain){
            if(damage.to->getDefensiveHorse())
                horses << "dhorse";
            if(damage.to->getOffensiveHorse())
                horses << "ohorse";

            if(horses.isEmpty())
                return false;

            if (player == NULL) return false;
            if(!player->askForSkillInvoke(objectName(), data))
                return false;

            room->setEmotion(player,"weapon/kylin_bow");

            QString horse_type;
            if(horses.length() == 2)
                horse_type = room->askForChoice(player, objectName(), horses.join("+"));
            else
                horse_type = horses.first();

            if(horse_type == "dhorse")
                room->throwCard(damage.to->getDefensiveHorse(), damage.to);
            else if(horse_type == "ohorse")
                room->throwCard(damage.to->getOffensiveHorse(), damage.to);
        }

        return false;
    }
};

KylinBow::KylinBow(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("kylin_bow");
    skill = new KylinBowSkill;
}

class EightDiagramSkill: public ArmorSkill{
private:
    EightDiagramSkill():ArmorSkill("eight_diagram"){
        events << CardAsked;
    }

public:
    static EightDiagramSkill *getInstance(){
        static EightDiagramSkill *instance = NULL;
        if(instance == NULL)
            instance = new EightDiagramSkill;

        return instance;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if (player == NULL) return false;
        if(asked == "jink"){

            if(room->askForSkillInvoke(player, objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);
                if(judge.isGood()){
                    room->setEmotion(player, "armor/eight_diagram");
                    Jink *jink = new Jink(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);
                    //room->setEmotion(player, "good");

                    return true;
                }else
                    room->setEmotion(player, "bad");
            }
        }
        return false;
    }
};



EightDiagram::EightDiagram(Suit suit, int number)
    :Armor(suit, number){
        setObjectName("eight_diagram");
        skill = EightDiagramSkill::getInstance();
}

AmazingGrace::AmazingGrace(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("amazing_grace");
    has_preact = true;
}

void AmazingGrace::doPreAction(Room *room, const CardUseStruct &card_use) const{
    QList<ServerPlayer *> players = card_use.to.isEmpty() ? room->getAllPlayers() : card_use.to;
    QList<int> card_ids = room->getNCards(players.length());
    room->fillAG(card_ids);

    QVariantList ag_list;
    foreach(int card_id, card_ids){
        room->setCardFlag(card_id, "visible");
        ag_list << card_id;
    }
    room->setTag("AmazingGrace", ag_list);
}

void AmazingGrace::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
    room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);

    QList<ServerPlayer *> players = targets.isEmpty() ? room->getAllPlayers() : targets;
    GlobalEffect::use(room, source, players);
    QVariantList ag_list;
    ag_list = room->getTag("AmazingGrace").toList();

    // throw the rest cards
    foreach(QVariant card_id, ag_list){
        room->takeAG(NULL, card_id.toInt());
    }

    room->broadcastInvoke("clearAG");
}

void AmazingGrace::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("AmazingGrace").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, ag_list)
        card_ids << card_id.toInt();

    int card_id = room->askForAG(effect.to, card_ids, false, objectName());
    card_ids.removeOne(card_id);

    room->takeAG(effect.to, card_id);
    ag_list.removeOne(card_id);

    room->setTag("AmazingGrace", ag_list);
}

GodSalvation::GodSalvation(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("god_salvation");
}

bool GodSalvation::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->isWounded();
}

void GodSalvation::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

SavageAssault::SavageAssault(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("savage_assault");
}

void SavageAssault::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *slash = room->askForCard(effect.to, "slash", "savage-assault-slash:"+ effect.from->objectName());
    if(slash)
        room->setEmotion(effect.to, "killer");
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;
        ServerPlayer *menghuo = room->findPlayerBySkillName("huoshou");
        bool hasmenghuo = room->getTag("Huoshou").toBool();
        if(hasmenghuo){
            if(menghuo)
                damage.from = menghuo;
            else
                damage.from = NULL;
        }      
        else{
            if(effect.from->isAlive())
                damage.from = effect.from;
            else
                damage.from = NULL;
        }

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
    const Card *jink = room->askForCard(effect.to, "jink", "archery-attack-jink:" + effect.from->objectName());
    if(jink && !(jink->getSkillName() == "eight_diagram" || jink->getSkillName() == "bazhen"))
        room->setEmotion(effect.to, "jink");
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }
}

void SingleTargetTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    if(!targets.isEmpty()){
        foreach(ServerPlayer *tmp, targets){
            effect.to = tmp;
            room->cardEffect(effect);
        }
    }
    else{
        effect.to = source;
        room->cardEffect(effect);
    }
    CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
    reason.m_skillName = this->getSkillName();
    if (targets.size() == 1) reason.m_targetId = targets.first()->objectName();
    room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);
}

Collateral::Collateral(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("collateral");
}

bool Collateral::isAvailable(const Player *player) const{
    foreach(const Player *p, player->getSiblings()){
        if(p->getWeapon() && p->isAlive())
            return true;
    }

    return false;
}


bool Collateral::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    foreach(const Player *p, to_select->getSiblings()){
        if(to_select->getWeapon() && to_select != Self && to_select->distanceTo(p) <= to_select->getAttackRange())
            return true;
    }
    return false;

}

void Collateral::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

    ServerPlayer *killer = targets[0];
    QList<ServerPlayer *> victims = room->getOtherPlayers(killer);
    foreach(ServerPlayer *p, victims){
        if(!killer->canSlash(p))
            victims.removeOne(p);
    }
    ServerPlayer *victim = room->askForPlayerChosen(source, victims,"collateral");
    const Weapon *weapon = killer->getWeapon();

    if(weapon == NULL)
        return;

    bool on_effect = room->cardEffect(this, source, killer);
    if(on_effect){
        QString prompt = QString("collateral-slash:%1:%2")
                .arg(source->objectName()).arg(victim->objectName());
        const Card *slash = NULL;
        int slash_targets = 1;
        if(killer->canSlash(victim))
            slash = room->askForCard(killer, "slash", prompt, QVariant(), CardUsed);
        if(slash){
            if(killer->hasWeapon("halberd") && killer->isLastHandCard(slash)){
                slash_targets = 3;
            }
            if(killer->hasSkill("shenji") && killer->getWeapon() == NULL)
                slash_targets = 3;
            bool distance_limit = true;
            int rangefix = 0;
            if(slash->isVirtualCard() && slash->getSubcards().length() > 0){
                foreach(int card_id, slash->getSubcards()){
                    if(Sanguosha->getCard(card_id)->inherits("Weapon")){
                        const Weapon *hisweapon = killer->getWeapon();
                        if(hisweapon->getRange() > 1){
                            rangefix = qMax((hisweapon->getRange()), rangefix);
                        }
                    }
                    if(Sanguosha->getCard(card_id)->inherits("OffensiveHorse")){
                        rangefix = qMax(rangefix, 1);
                    }
                }
            }
            if(killer->hasSkill("lihuo") && slash->inherits("FireSlash"))
                slash_targets ++;

            if(slash->inherits("WushenSlash")){
                distance_limit = false;
            }

            if(!killer->canSlash(victim, distance_limit, rangefix)){
                slash = NULL;
            }
        }
        if (victim->isDead()){
            if (source->isDead()){
                if(killer->isAlive() && killer->getWeapon()){
                    int card_id = weapon->getId();
                    room->throwCard(card_id, killer);
                }
            }
            else
            {
                if(killer->isAlive() && killer->getWeapon()){
                    source->obtainCard(weapon);
                }
            }
        }
        else if (source->isDead()){
            if (killer->isAlive()){
                if(slash){
                    CardUseStruct use;
                    use.card = slash;
                    use.from = killer;
                    use.to << victim;
                    if(slash_targets > 1){
                        victims = room->getOtherPlayers(killer);
                        victims.removeOne(victim);
                        foreach(ServerPlayer *p, victims){
                            if(killer->distanceTo(p) > killer->getAttackRange())
                                victims.removeOne(p);
                        }

                        while(slash_targets > 1 && victims.length() > 0
                                && room->askForChoice(killer, objectName(), "yes+no") == "yes"){
                            ServerPlayer *tmptarget = room->askForPlayerChosen(killer,victims,"halberd");
                            use.to << tmptarget;
                            victims.removeOne(tmptarget);
                            slash_targets--;
                        }
                    }
                    CardMoveReason reason(CardMoveReason::S_REASON_LETUSE, killer->objectName());
                    room->moveCardTo(slash, killer, NULL, Player::DiscardPile, reason);
                    room->useCard(use);
                }
                else{
                    if(killer->getWeapon()){
                        int card_id = weapon->getId();
                        room->throwCard(card_id, killer);
                    }
                }
            }
        }
        else{
            if(killer->isDead()) ;
            else if(!killer->getWeapon()){
                if(slash){
                    CardUseStruct use;
                    use.card = slash;
                    use.from = killer;
                    use.to << victim;
                    if(slash_targets > 1){
                        victims = room->getOtherPlayers(killer);
                        victims.removeOne(victim);
                        foreach(ServerPlayer *p, victims){
                            if(killer->distanceTo(p) > killer->getAttackRange())
                                victims.removeOne(p);
                        }

                        while(slash_targets > 1 && victims.length() > 0
                                && room->askForChoice(killer, objectName(), "yes+no") == "yes"){
                            ServerPlayer *tmptarget = room->askForPlayerChosen(killer,victims,"halberd");
                            use.to << tmptarget;
                            victims.removeOne(tmptarget);
                            slash_targets--;
                        }
                    }
                    CardMoveReason reason(CardMoveReason::S_REASON_LETUSE, killer->objectName());
                    room->moveCardTo(slash, killer, NULL, Player::DiscardPile, reason);
                    room->useCard(use);
                }
            }
            else{
                if(slash){
                    CardUseStruct use;
                    use.card = slash;
                    use.from = killer;
                    use.to << victim;
                    if(slash_targets > 1){
                        victims = room->getOtherPlayers(killer);
                        victims.removeOne(victim);
                        foreach(ServerPlayer *p, victims){
                            if(killer->distanceTo(p) > killer->getAttackRange())
                                victims.removeOne(p);
                        }

                        while(slash_targets > 1 && victims.length() > 0
                                && room->askForChoice(killer, objectName(), "yes+no") == "yes"){
                            ServerPlayer *tmptarget = room->askForPlayerChosen(killer,victims,"halberd");
                            use.to << tmptarget;
                            victims.removeOne(tmptarget);
                            slash_targets--;
                        }
                    }
                    CardMoveReason reason(CardMoveReason::S_REASON_LETUSE, killer->objectName());
                    room->moveCardTo(slash, killer, NULL, Player::DiscardPile, reason);
                    room->useCard(use);
                }
                else{
                    if(killer->getWeapon())
                        source->obtainCard(weapon);
                }
            }
        }
    }
}

Nullification::Nullification(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("nullification");
}

void Nullification::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    // does nothing, just throw it
    CardMoveReason reason(CardMoveReason::S_REASON_RESPONSE, source->objectName());
    room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);
}

bool Nullification::isAvailable(const Player *) const{
    return false;
}

ExNihilo::ExNihilo(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("ex_nihilo");
    target_fixed = true;
}

void ExNihilo::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

Duel::Duel(Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("duel");
}

bool Duel::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;

    return targets.isEmpty();
}

void Duel::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    room->setEmotion(first, "duel");
    room->setEmotion(second, "duel");

    forever{
        if(second->hasFlag("WushuangTarget")){
            room->broadcastSkillInvoke("wushuang");
            const Card *slash = room->askForCard(first, "slash", "@wushuang-slash-1:" + second->objectName());
            if(slash == NULL)
                break;

            slash = room->askForCard(first, "slash", "@wushuang-slash-2:" + second->objectName());
            if(slash == NULL)
                break;

        }else{
            const Card *slash = room->askForCard(first, "slash", "duel-slash:" + second->objectName());
            if(slash == NULL)
                break;
        }

        qSwap(first, second);
    }

    DamageStruct damage;
    damage.card = this;
    if(second->isAlive())
        damage.from = second;
    else
        damage.from = NULL;
    damage.to = first;

    room->damage(damage);
}

Snatch::Snatch(Suit suit, int number):SingleTargetTrick(suit, number, true) {
    setObjectName("snatch");
}

bool Snatch::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
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
    if(effect.from->isDead())
        return;
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
    room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
}

Dismantlement::Dismantlement(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
        setObjectName("dismantlement");
}

bool Dismantlement::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isAllNude())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Dismantlement::onEffect(const CardEffectStruct &effect) const{
    if(effect.from->isDead())
        return;
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    CardMoveReason reason(CardMoveReason::S_REASON_DISMANTLE, effect.to->objectName());
    reason.m_playerId = effect.from->objectName();
    reason.m_targetId = effect.to->objectName();
    room->moveCardTo(Sanguosha->getCard(card_id), NULL, NULL, Player::DiscardPile, reason);
    // room->throwCard(card_id, room->getCardPlace(card_id) == Player::PlaceDelayedTrick ? NULL : effect.to);

    LogMessage log;
    log.type = "$Dismantlement";
    log.from = effect.to;
    log.card_str = QString::number(card_id);
    room->sendLog(log);
}

Indulgence::Indulgence(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("indulgence");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool Indulgence::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Indulgence::takeEffect(ServerPlayer *target) const{
    target->clearHistory();
    target->skip(Player::Play);
}

Disaster::Disaster(Card::Suit suit, int number)
    :DelayedTrick(suit, number, true)
{
    target_fixed = true;
}

bool Disaster::isAvailable(const Player *player) const{
    if(player->containsTrick(objectName()))
        return false;

    return ! player->isProhibited(player, this);
}

Lightning::Lightning(Suit suit, int number):Disaster(suit, number){
    setObjectName("lightning");

    judge.pattern = QRegExp("(.*):(spade):([2-9])");
    judge.good = false;
    judge.reason = objectName();
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


// EX cards

class IceSwordSkill: public WeaponSkill{
public:
    IceSwordSkill():WeaponSkill("ice_sword"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && !damage.to->isNude()
            && !damage.chain && player->askForSkillInvoke("ice_sword", data)){
            room->setEmotion(player,"weapon/ice_sword");
                int card_id = room->askForCardChosen(player, damage.to, "he", "ice_sword");
                CardMoveReason reason(CardMoveReason::S_REASON_DISMANTLE, damage.to->objectName());
                reason.m_playerId = damage.from->objectName();
                reason.m_targetId = damage.to->objectName();
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, NULL, Player::DiscardPile, reason);

                if(!damage.to->isNude()){
                    card_id = room->askForCardChosen(player, damage.to, "he", "ice_sword");
                    CardMoveReason reason(CardMoveReason::S_REASON_DISMANTLE, damage.to->objectName());
                    reason.m_playerId = damage.from->objectName();
                    reason.m_targetId = damage.to->objectName();
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, NULL, Player::DiscardPile, reason);
                }

                return true;
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isBlack()){
            LogMessage log;
            log.type = "#ArmorNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            player->getRoom()->sendLog(log);

            room->setEmotion(player, "armor/renwang_shield");

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

class HorseSkill: public DistanceSkill{
public:
    HorseSkill():DistanceSkill("horse"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->getOffensiveHorse())
            correct += from->getOffensiveHorse()->getCorrect();
        if(to->getDefensiveHorse())
            correct += to->getDefensiveHorse()->getCorrect();

        return correct;
    }
};

StandardCardPackage::StandardCardPackage()
    :Package("standard_cards")
{
    type = Package::CardPack;

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
        << new EightDiagram(Card::Club);

    skills << EightDiagramSkill::getInstance();

    {
        QList<Card *> horses;
        horses << new DefensiveHorse(Card::Spade, 5)
            << new DefensiveHorse(Card::Club, 5)
            << new DefensiveHorse(Card::Heart, 13)
            << new OffensiveHorse(Card::Heart, 5)
            << new OffensiveHorse(Card::Spade, 13)
            << new OffensiveHorse(Card::Diamond, 13);

        horses.at(0)->setObjectName("jueying");
        horses.at(1)->setObjectName("dilu");
        horses.at(2)->setObjectName("zhuahuangfeidian");
        horses.at(3)->setObjectName("chitu");
        horses.at(4)->setObjectName("dayuan");
        horses.at(5)->setObjectName("zixing");

        cards << horses;

        skills << new HorseSkill;
    }

    cards << new AmazingGrace(Card::Heart, 3)
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
        << new Lightning(Card::Spade, 1);

    foreach(Card *card, cards)
        card->setParent(this);

    skills << new SpearSkill << new AxeViewAsSkill;
}

StandardExCardPackage::StandardExCardPackage()
    :Package("standard_ex_cards")
{
    QList<Card *> cards;
    cards << new IceSword(Card::Spade, 2)
        << new RenwangShield(Card::Club, 2)
        << new Lightning(Card::Heart, 12)
        << new Nullification(Card::Diamond, 12);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(StandardCard)
    ADD_PACKAGE(StandardExCard)
