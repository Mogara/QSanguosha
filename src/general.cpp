#include "general.h"
#include "engine.h"

General::General(const QString &name){
    setObjectName(name);

    // initialize other field using name
    General *copy = Sanguosha->getGeneral(name);
    kingdom = copy->kingdom;
    hp = max_hp = copy->max_hp;
    male = copy->max_hp;
    is_lord = false;
}

General::General(const QString &name, const QString &kingdom, int max_hp, bool male)
    :kingdom(kingdom), max_hp(max_hp), male(male), is_lord(false)
{
    setObjectName(name);
    hp = max_hp;

    // initialize related pixmaps
}

int General::getMaxHp() const{
    return max_hp;
}

int General::getHp() const{
    return hp;
}

void General::setHp(int hp){
    if(hp > 0 && hp <= max_hp)
        this->hp = hp;
}

QString General::getKingdom() const{
    return kingdom;
}

bool General::isMale() const{
    return male;
}

bool General::isFemale() const{
    return !male;
}

bool General::isWounded() const{
    return hp < max_hp;
}

void General::enthrone(){
    max_hp ++;
    hp = max_hp;
    is_lord = true;
}
