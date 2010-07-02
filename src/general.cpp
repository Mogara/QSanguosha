#include "general.h"
#include "engine.h"

General *General::getInstance(const QString &name){
    General *copy = Sanguosha->getGeneral(name);

    if(copy){
        General *general = new General;
        general->setObjectName(name);
        general->kingdom = copy->kingdom;
        general->hp = general->max_hp = copy->max_hp;
        general->male = copy->max_hp;
        general->is_lord = false;
        return general;
    }else
        return NULL;
}

General::General(const QString &name, const QString &kingdom, int max_hp, bool male, const QString &pixmap_dir)
    :kingdom(kingdom), max_hp(max_hp), hp(max_hp), male(male), is_lord(false), pixmap_dir(pixmap_dir), leader(false)
{
    QChar leader_symbol('!');
    if(name.contains(leader_symbol)){
        QString copy = name;
        setObjectName(copy.remove(leader_symbol));
        leader = true;
    }else
        setObjectName(name);
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

bool General::isLeader() const{
    return leader;
}

void General::enthrone(){
    max_hp ++;
    hp = max_hp;
    is_lord = true;
}

QString General::getPixmapPath(const QString &category){
    return QString("%1/%2/%3.png").arg(pixmap_dir).arg(category).arg(objectName());
}

