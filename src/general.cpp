#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"

General::General(Package *package, const QString &name, const QString &kingdom, int max_hp, bool male, bool hidden)
    :QObject(package), kingdom(kingdom), max_hp(max_hp), male(male), hidden(hidden)
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

bool General::isHidden() const{
    return hidden;
}

QString General::getPixmapPath(const QString &category) const{
    QString suffix = "png";
    if(category == "card")
        suffix = "jpg";

    return QString("image/generals/%1/%2.%3").arg(category).arg(objectName()).arg(suffix);
}

void General::addSkill(Skill *skill){    
    skill->setParent(this);
    skill->initMediaSource();
    skill_map.insert(skill->objectName(), skill);
}

bool General::hasSkill(const QString &skill_name) const{
    return skill_map.contains(skill_name);
}

QString General::getPackage() const{
    QObject *p = parent();
    if(p)
        return p->objectName();
    else
        return QString(); // avoid null pointer exception;
}

QString General::getSkillDescription() const{
    QString description;

    QList<const Skill *> skills = findChildren<const Skill *>();
    foreach(const Skill *skill, skills){
        if(skill->objectName().startsWith("#"))
            continue;

        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
    }

    return description;
}

void General::lastWord() const{
    QString filename = QString("audio/death/%1.ogg").arg(objectName());
    Sanguosha->playEffect(filename);
}
