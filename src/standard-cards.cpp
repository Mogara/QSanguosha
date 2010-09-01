#include "standard.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "room.h"

Slash::Slash(Suit suit, int number):BasicCard(suit, number){
    setObjectName("slash");
}

bool Slash::isAvailableAtPlay() const{
    bool unlimited_slash = ClientInstance->tag.value("unlimited_slash", false).toBool();
    if(unlimited_slash)
        return true;
    else{
        int limited_slash_count = ClientInstance->tag.value("limited_slash_count", 1).toInt();
        return ClientInstance->tag.value("slash_count").toInt() < limited_slash_count;
    }
}

QString Slash::getSubtype() const{
    return "attack_card";
}

void Slash::use(const QList<const ClientPlayer *> &targets) const{
    BasicCard::use(targets);

    // increase slash count
    int slash_count = ClientInstance->tag.value("slash_count", 0).toInt();
    ClientInstance->tag.insert("slash_count", slash_count + 1);
}

void Slash::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    BasicCard::use(room, source, targets);

    room->playCardEffect(source, objectName());

    foreach(ServerPlayer *target, targets){
        room->requestForCard(target, "jink");
    }
}

bool Slash::targetsFeasible(const QList<const ClientPlayer *> &targets) const{   
    return !targets.isEmpty();
}

bool Slash::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    int slash_targets = ClientInstance->tag.value("slash_targets", 1).toInt();
    if(targets.length() >= slash_targets)
        return false;

    const ClientPlayer *self = ClientInstance->getPlayer();
    return self->distanceTo(to_select) <= self->getAttackRange();
}

Jink::Jink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("jink");
}

QString Jink::getSubtype() const{
    return "defense_card";
}

bool Jink::isAvailableAtPlay() const{
    return false;
}

Peach::Peach(Suit suit, int number):BasicCard(suit, number){
    target_fixed = true;
    setObjectName("peach");
}

QString Peach::getSubtype() const{
    return "recover_card";
}

void Peach::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(source, this);
    room->recover(source, 1);

    room->playCardEffect(source, objectName());
}

bool Peach::isAvailableAtPlay() const{
    return ClientInstance->getPlayer()->isWounded();
}

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(source, this);

    DamageStruct damage;
    damage.from = damage.to = source;
    damage.card = this;
    damage.damage = 1;
    damage.nature = Normal;

    room->damage(damage);
}

class Crossbow:public Weapon{
public:
    Crossbow(Suit suit, int number = 1):Weapon(suit, number, 1){
        setObjectName("crossbow");
    }
};

class DoubleSword:public Weapon{
public:
    DoubleSword(Suit suit = Spade, int number = 2):Weapon(suit, number, 2){
        setObjectName("double_sword");
    }
};

class QinggangSword:public Weapon{
public:
    QinggangSword(Suit suit = Spade, int number = 6):Weapon(suit, number, 2){
        setObjectName("qinggang_sword");
    }
};

class YitianSword:public Weapon{
public:
    YitianSword(Suit suit = Spade, int number = 6):Weapon(suit, number, 2){
        setObjectName("yitian_sword");
    }
};

class Blade:public Weapon{
public:
    Blade(Suit suit = Spade, int number = 5):Weapon(suit, number, 3){
        setObjectName("blade");
    }
};

class Spear:public Weapon{
public:
    Spear(Suit suit = Spade, int number = 12):Weapon(suit, number, 3){
        setObjectName("spear");
    }
};

class Axe:public Weapon{
public:
    Axe(Suit suit = Diamond, int number = 5):Weapon(suit, number, 3){
        setObjectName("axe");
    }
};

class Halberd:public Weapon{
public:
    Halberd(Suit suit = Diamond, int number = 12):Weapon(suit, number, 4){
        setObjectName("halberd");
    }
};

class KylinBow:public Weapon{
public:
    KylinBow(Suit suit = Heart, int number = 5):Weapon(suit, number, 5){
        setObjectName("kylin_bow");
    }
};

class EightDiagram:public Armor{
public:
    EightDiagram(Suit suit, int number = 2):Armor(suit, number){
        setObjectName("eight_diagram");
    }
};

class AmazingGrace:public GlobalEffect{
public:
    AmazingGrace(Suit suit, int number):GlobalEffect(suit, number){
        setObjectName("amazing_grace");
    }
};

class GodSalvation:public GlobalEffect{
public:
    GodSalvation(Suit suit = Heart, int number = 1):GlobalEffect(suit, number){
        setObjectName("god_salvation");
    }
};

class SavageAssault:public AOE{
public:
    SavageAssault(Suit suit, int number)
        :AOE(suit, number) {
        setObjectName("savage_assault");
    }
};

class Collateral:public SingleTargetTrick{
public:
    Collateral(Suit suit, int number):SingleTargetTrick(suit, number){
        setObjectName("collateral");
    }

    virtual bool isAvailableAtPlay() const{
        Client *client = ClientInstance;
        QList<ClientPlayer*> players = client->findChildren<ClientPlayer*>();
        foreach(ClientPlayer *player, players){
            if(player->getWeapon() != NULL && player != client->getPlayer())
                return true;
        }

        return false;
    }
};

class Nullification:public SingleTargetTrick{
public:
    Nullification(Suit suit, int number):SingleTargetTrick(suit, number){
        setObjectName("nullification");
    }

    virtual bool isAvailableAtPlay() const{
        return false;
    }
};

class ArcheryAttack:public AOE{
public:
    ArcheryAttack(Suit suit = Heart, int number = 1):AOE(suit, number){
        setObjectName("archery_attack");
    }
};

class Duel:public SingleTargetTrick{
public:
    Duel(Suit suit, int number):SingleTargetTrick(suit, number) {
        setObjectName("duel");
    }
};

class ExNihilo: public SingleTargetTrick{
public:
    ExNihilo(Suit suit, int number):SingleTargetTrick(suit, number){
        setObjectName("ex_nihilo");
        target_fixed = true;
    }

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
        room->throwCard(source, this);
        room->drawCards(source, 2);
    }
};

Snatch::Snatch(Suit suit, int number):SingleTargetTrick(suit, number) {
    setObjectName("snatch");
}


bool Snatch::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasFlag("qianxun"))
        return false;

    if(to_select->hasFlag("weimu") && isBlack())
        return false;

    return true;
}

class Dismantlement:public SingleTargetTrick{
public:
    Dismantlement(Suit suit, int number):SingleTargetTrick(suit, number) {
        setObjectName("dismantlement");
    }
};

class Indulgence:public DelayedTrick{
public:
    Indulgence(Suit suit, int number):DelayedTrick(suit, number){
        setObjectName("indulgence");
    }
};

class Lightning:public DelayedTrick{
public:
    Lightning(Suit suit, int number):DelayedTrick(suit, number){
        setObjectName("lightning");
    }

    virtual bool targetFixed() const{
        return true;
    }
};

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

          << new Shit(Card::Club, 1)
          << new Shit(Card::Heart, 1)
          << new Shit(Card::Diamond, 1)

          << new Crossbow(Card::Club)
          << new Crossbow(Card::Diamond)
          << new DoubleSword
          << new QinggangSword
          << new YitianSword
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
          << new Horse("dawan", Card::Spade, 13, -1)
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
          << new Lightning(Card::Spade, 1);



    foreach(Card *card, cards)
        card->setParent(this);

    t["spade"] = tr("spade");
    t["club"] = tr("club");
    t["heart"] = tr("heart");
    t["diamond"] = tr("diamond");

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
    t["shit"] = tr("shit");

    t["crossbow"] = tr("crossbow");
    t["double_sword"] = tr("double_sword");
    t["qinggang_sword"] = tr("qinggang_sword");
    t["yitian_sword"] = tr("yitian_sword");
    t["blade"] = tr("blade");
    t["spear"] = tr("spear");
    t["axe"] = tr("axe");
    t["halberd"] = tr("halberd");
    t["kylin_bow"] = tr("kylin_bow");

    t["eight_diagram"] = tr("eight_diagram");

    t["jueying"] = tr("jueying");
    t["dilu"] = tr("dilu");
    t["zhuahuangfeidian"] = tr("zhuahuangfeidian");
    t["chitu"] = tr("chitu");
    t["dawan"] = tr("dawan");
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
}
