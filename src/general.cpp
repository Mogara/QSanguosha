#include "general.h"
#include "engine.h"
#include "skill.h"

General::General(const QString &name, const QString &kingdom, int max_hp, bool male, const QString &pixmap_dir)
    :kingdom(kingdom), max_hp(max_hp), male(male), pixmap_dir(pixmap_dir)
{
    static QChar lord_symbol('$');
    if(name.contains(lord_symbol)){
        QString copy = name;
        copy.remove(lord_symbol);
        lord = true;
        setObjectName(copy);
    }else{
        lord = false;
        setObjectName(name);
    }
}

int General::getMaxHp() const{
    return max_hp;
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

bool General::isLord() const{
    return lord;
}

QObjectList General::getSkills() const{
    return skills;
}

void General::setSkills(const QObjectList &skill_objs){
    skills.clear();

    foreach(QObject *skill_obj, skill_objs){
        Skill *skill = qobject_cast<Skill*>(skill_obj);
        if(skill)
            skills << skill;
    }
}

QString General::getPixmapPath(const QString &category) const{
    return QString("%1/%2/%3.png").arg(pixmap_dir).arg(category).arg(objectName());
}

QString General::getKingdomPath() const{
    return QString("%1/kingdom/%2").arg(pixmap_dir).arg(kingdom);
}

