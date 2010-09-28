#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"

General::General(Package *package, const QString &name, const QString &kingdom, int max_hp, bool male)
    :QObject(package), kingdom(kingdom), max_hp(max_hp), male(male)
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

    last_word = MediaSource(QString("%1/generals/death/%2.wav").arg(getPackage()).arg(objectName()));
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

QString General::getPixmapPath(const QString &category) const{
    return QString("%1/generals/%2/%3.png").arg(getPackage()).arg(category).arg(objectName());
}

QString General::getKingdomPath() const{    
    return QString(":/kingdom/%1.png").arg(kingdom);
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
    return parent()->objectName();
}

QString General::getSkillDescription() const{
    QString description;

    QList<Skill *> skills = skill_map.values();
    foreach(const Skill *skill, skills){
        if(skill->objectName().startsWith("#"))
            continue;

        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/>").arg(skill_name).arg(desc));
    }

    return description;
}

void General::playEffect(const QString &skill_name) const
{
    Skill *skill = skill_map.value(skill_name, NULL);
    if(skill)
        skill->playEffect();
}

void General::lastWord() const{
    Sanguosha->playEffect(last_word);
}
