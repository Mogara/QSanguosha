#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"

#include <QSize>

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
    skill_set << skill->objectName();
}

void General::addSkill(const QString &skill_name){
    extra_set << skill_name;
}

bool General::hasSkill(const QString &skill_name) const{
    return skill_set.contains(skill_name) || extra_set.contains(skill_name);
}

QSet<const Skill *> General::getVisibleSkills() const{
    QSet<const Skill *> skills;
    foreach(const Skill *skill, findChildren<const Skill *>()){
        if(skill->isVisible())
            skills << skill;
    }

    foreach(QString skill_name, extra_set){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const TriggerSkill *> General::getTriggerSkills() const{
    QSet<const TriggerSkill *> skills = findChildren<const TriggerSkill *>().toSet();

    foreach(QString skill_name, extra_set){
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if(skill)
            skills << skill;
    }

    return skills;
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

    foreach(const Skill *skill, getVisibleSkills()){
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


QSize General::TinyIconSize(42, 36);
