#include "general.h"

General::General(QObject *parent) :
    QObject(parent), _hp(5), _max_hp(5)
{
}

int General::max_hp() const{
    return _max_hp;
}

int General::hp() const{
    return _hp;
}

void General::setHp(int hp){
    if(hp > 0 && hp <= _max_hp)
        _hp = hp;
}

QString General::kingdom() const{
    return _kingdom;
}
