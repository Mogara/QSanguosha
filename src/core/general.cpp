#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"

#include <QSize>
#include <QFile>

General::General(Package *package, const QString &name, const QString &kingdom,
                 int max_hp, bool male, bool hidden, bool never_shown)
    : QObject(package), kingdom(kingdom), max_hp(max_hp), gender(male ? Male : Female),
      hidden(hidden), never_shown(never_shown)
{
    static QChar lord_symbol('$');
    if (name.endsWith(lord_symbol)) {
        QString copy = name;
        copy.remove(lord_symbol);
        lord = true;
        setObjectName(copy);
    } else {
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

void General::setGender(Gender gender) {
    this->gender = gender;
}

General::Gender General::getGender() const{
    return gender;
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

void General::addSkill(Skill *skill) {
    skill->setParent(this);
    skill_set << skill->objectName();
    if (!skillname_list.contains(skill->objectName()))
        skillname_list << skill->objectName();
}

void General::addSkill(const QString &skill_name) {
    if (extra_set.contains(skill_name)) return;
    extra_set << skill_name;
    if (!skillname_list.contains(skill_name))
        skillname_list << skill_name;
}

bool General::hasSkill(const QString &skill_name) const{
    return skill_set.contains(skill_name) || extra_set.contains(skill_name);
}

QList<const Skill *> General::getSkillList() const{
    QList<const Skill *> skills;
    foreach (QString skill_name, skillname_list) {
        if (skill_name == "mashu" && ServerInfo.DuringGame
            && ServerInfo.GameMode == "02_1v1" && ServerInfo.GameRuleMode != "Classical")
            skill_name = "xiaoxi";
        const Skill *skill = Sanguosha->getSkill(skill_name);
        Q_ASSERT(skill != NULL);
        skills << skill;
    }
    return skills;
}

QList<const Skill *> General::getVisibleSkillList() const{
    QList<const Skill *> skills;
    foreach (const Skill *skill, getSkillList()) {
        if (skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const Skill *> General::getVisibleSkills() const{
    return getVisibleSkillList().toSet();
}

QSet<const TriggerSkill *> General::getTriggerSkills() const{
    QSet<const TriggerSkill *> skills;
    foreach (QString skill_name, skillname_list) {
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if (skill)
            skills << skill;
    }
    return skills;
}

void General::addRelateSkill(const QString &skill_name) {
    related_skills << skill_name;
}

QStringList General::getRelatedSkillNames() const{
    return related_skills;
}

QString General::getPackage() const{
    QObject *p = parent();
    if (p)
        return p->objectName();
    else
        return QString(); // avoid null pointer exception;
}

QString General::getSkillDescription(bool include_name) const{
    QString description;

    foreach (const Skill *skill, getVisibleSkillList()) {
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
    }

    if (include_name) {
        QString color_str = Sanguosha->getKingdomColor(kingdom).name();
        QString name = QString("<font color=%1><b>%2</b></font>     ").arg(color_str).arg(Sanguosha->translate(objectName()));
        name.prepend(QString("<img src='image/kingdom/icon/%1.png'/>    ").arg(kingdom));
        for (int i = 0; i < max_hp; i++)
            name.append("<img src='image/system/magatamas/5.png' height = 12/>");
        name.append("<br/> <br/>");
        description.prepend(name);
    }

    return description;
}

void General::lastWord() const{
    QString filename = QString("audio/death/%1.ogg").arg(objectName());
    bool fileExists = QFile::exists(filename);
    if (!fileExists) {
        QStringList origin_generals = objectName().split("_");
        if (origin_generals.length() > 1)
            filename = QString("audio/death/%1.ogg").arg(origin_generals.last());
    }
    Sanguosha->playAudioEffect(filename);
}

