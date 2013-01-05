#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"

#include <QSize>
#include <QFile>

General::General(Package *package, const QString &name, const QString &kingdom, int max_hp, bool male, bool hidden, bool never_shown)
    :QObject(package), kingdom(kingdom), max_hp(max_hp), gender(male ? Male : Female), hidden(hidden), never_shown(never_shown)
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
    return gender == Male;
}

bool General::isFemale() const{
    return gender == Female;
}

bool General::isNeuter() const{
    return gender == Neuter;
}

void General::setGender(Gender gender){
    this->gender = gender;
}

General::Gender General::getGender() const{
    return gender;
}

QString General::getGenderString() const{
    switch(gender){
    case Male: return "male";
    case Female: return "female";
    default:
        return "neuter";
    }
}

bool General::isLord() const{
    return lord;
}

bool General::isHidden() const{
    return hidden;
}

bool General::isTotallyHidden() const{
    return never_shown;
}

QString General::getPixmapPath(const QString &category) const{
    QString suffix = "png";
    if(category == "card")
        suffix = "jpg";

    return QString("image/generals/%1/%2.%3").arg(category).arg(objectName()).arg(suffix);
}

void General::addSkill(Skill *skill){
    skill->setParent(this);
    skill_set << skill->objectName();
}

void General::addSkill(const QString &skill_name){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill)
        extra_set << skill_name;
    else
        addSkill(new Skill(skill_name));
}

bool General::hasSkill(const QString &skill_name) const{
    return skill_set.contains(skill_name) || extra_set.contains(skill_name);
}

QList<const Skill *> General::getVisibleSkillList() const{
    QList<const Skill *> skills;
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

void General::addRelateSkill(const QString &skill_name){
    related_skills << skill_name;
}

QStringList General::getRelatedSkillNames() const{
    return related_skills;
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

    foreach(const Skill *skill, getVisibleSkillList()){
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
    }

    return description;
}

void General::lastWord() const{
    QString filename = QString("audio/death/%1.dat").arg(objectName());
    if(!QFile::exists(filename))
        filename = QString("audio/death/%1.ogg").arg(objectName());
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        QStringList origin_generals = objectName().split("_");
        if(origin_generals.length()>1){
            filename = QString("audio/death/%1.dat").arg(origin_generals.at(1));
            if(!QFile::exists(filename))
                filename = QString("audio/death/%1.ogg").arg(origin_generals.at(1));
        }
    }
    if(!file.open(QIODevice::ReadOnly) && objectName().endsWith("f")){
        QString origin_general = objectName();
        origin_general.chop(1);
        if(Sanguosha->getGeneral(origin_general)){
            filename = QString("audio/death/%1.dat").arg(origin_general);
            if(!QFile::exists(filename))
                filename = QString("audio/death/%1.ogg").arg(origin_general);
        }
    }
    Sanguosha->playEffect(filename);
}

QSize General::BigIconSize(94, 96);
QSize General::SmallIconSize(122, 50);
QSize General::TinyIconSize(42, 36);
