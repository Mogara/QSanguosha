#include "standard.h"
#include "general.h"
#include "engine.h"
#include "client.h"

class Slash:public BasicCard{
public:
    Slash(enum Suit suit, int number):BasicCard(suit, number){
        setObjectName("slash");
    }

    virtual QString getSubtype() const{
        return "attack_card";
    }
};

class Jink:public BasicCard{
public:
    Jink(enum Suit suit, int number):BasicCard(suit, number){
        setObjectName("jink");
    }

    virtual QString getSubtype() const{
        return "defense_card";
    }

    virtual bool isAvailable(const Client *client) const{
        return false;
    }
};

class Peach:public BasicCard{
public:
    Peach(enum Suit suit, int number):BasicCard(suit, number){
        setObjectName("peach");
    }

    virtual bool isAvailable(const Client *client) const{
        return client->getPlayer()->isWounded();
    }

    virtual QString getSubtype() const{
        return "recover_card";
    }
};

class Crossbow:public Weapon{
public:
    Crossbow(enum Suit suit):Weapon(suit, 1, 1){
        setObjectName("crossbow");
    }
};

class DoubleSword:public Weapon{
public:
    DoubleSword():Weapon(Spade, 2, 2){
        setObjectName("double_sword");
    }
};

class QinggangSword:public Weapon{
public:
    QinggangSword():Weapon(Spade, 6, 2){
        setObjectName("qinggang_sword");
    }
};

class YitianSword:public Weapon{
public:
    YitianSword():Weapon(Spade, 6, 2){
        setObjectName("yitian_sword");
    }
};

class Blade:public Weapon{
public:
    Blade():Weapon(Spade, 5, 3){
        setObjectName("blade");
    }
};

class Spear:public Weapon{
public:
    Spear():Weapon(Spade, 12, 3){
        setObjectName("spear");
    }
};

class Axe:public Weapon{
public:
    Axe():Weapon(Diamond, 5, 3){
        setObjectName("axe");
    }
};

class Halberd:public Weapon{
public:
    Halberd():Weapon(Diamond, 12, 4){
        setObjectName("halberd");
    }
};

class KylinBow:public Weapon{
public:
    KylinBow():Weapon(Heart, 5, 5){
        setObjectName("kylin_bow");
    }
};

class EightDiagram:public Armor{
public:
    EightDiagram(enum Suit suit):Armor(suit, 2){
        setObjectName("eight_diagram");
    }
};

class AmazingGrace:public GlobalEffect{
public:
    AmazingGrace(int number):GlobalEffect(Heart, number){
        setObjectName("amazing_grace");
    }
};

class GodSalvation:public GlobalEffect{
public:
    GodSalvation():GlobalEffect(Heart, 1){
        setObjectName("god_salvation");
    }
};

class SavageAssault:public AOE{
public:
    SavageAssault(enum Suit suit, int number)
        :AOE(suit, number) {
        setObjectName("savage_assault");
    }
};

class Collateral:public SingleTargetTrick{
public:
    Collateral(enum Suit suit, int number):SingleTargetTrick(suit, number){
        setObjectName("collateral");
    }
};

class Nullification:public SingleTargetTrick{
public:
    Nullification(enum Suit suit, int number):SingleTargetTrick(suit, number){
        setObjectName("nullification");
    }

    virtual bool isAvailable(const Client *) const{
        return false;
    }
};

class ArcheryAttack:public AOE{
public:
    ArcheryAttack():AOE(Heart, 1){
        setObjectName("archery_attack");
    }
};

class Duel:public SingleTargetTrick{
public:
    Duel(enum Suit suit, int number):SingleTargetTrick(suit, number) {
        setObjectName("duel");
    }
};

class ExNihilo: public SingleTargetTrick{
public:
    ExNihilo(int number):SingleTargetTrick(Heart, number){
        setObjectName("ex_nihilo");
    }
};

class Snatch:public SingleTargetTrick{
public:
    Snatch(enum Suit suit, int number):SingleTargetTrick(suit, number) {
        setObjectName("snatch");
    }
};

class Dismantlement:public SingleTargetTrick{
public:
    Dismantlement(enum Suit suit, int number):SingleTargetTrick(suit, number) {
        setObjectName("dismantlement");
    }
};

class Indulgence:public DelayedTrick{
public:
    Indulgence(enum Suit suit, int number):DelayedTrick(suit, number){
        setObjectName("indulgence");
    }
};

class Lightning:public DelayedTrick{
public:
    Lightning(enum Suit suit, int number):DelayedTrick(suit, number){
        setObjectName("lightning");
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

          << new AmazingGrace(3)
          << new AmazingGrace(4)
          << new GodSalvation
          << new SavageAssault(Card::Spade, 7)
          << new SavageAssault(Card::Spade, 13)
          << new SavageAssault(Card::Club, 7)
          << new ArcheryAttack
          << new Duel(Card::Spade, 1)
          << new Duel(Card::Club, 1)
          << new Duel(Card::Diamond, 1)
          << new ExNihilo(7)
          << new ExNihilo(8)
          << new ExNihilo(9)
          << new ExNihilo(11)
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
