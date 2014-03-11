#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"
#include "settings.h"

#include <QSize>
#include <QFile>

General::General(Package *package, const QString &name, const QString &kingdom,
                 int double_max_hp, bool male, bool hidden, bool never_shown)
    : QObject(package), kingdom(kingdom), double_max_hp(double_max_hp), gender(male ? Male : Female),
      hidden(hidden), never_shown(never_shown), head_max_hp_adjusted_value(0), deputy_max_hp_adjusted_value(0)
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

int General::getDoubleMaxHp() const{
    return double_max_hp;
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

int General::getMaxHpHead() const {
    return double_max_hp + head_max_hp_adjusted_value;
}

int General::getMaxHpDeputy() const {
    return double_max_hp + deputy_max_hp_adjusted_value;
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

QList<const Skill *> General::getSkillList(bool relate_to_place, bool head_only) const{
    QList<const Skill *> skills;
    foreach (QString skill_name, skillname_list) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        Q_ASSERT(skill != NULL);
        if (relate_to_place && !skill->relateToPlace(!head_only))
            skills << skill;
        else if (!relate_to_place) skills << skill;
    }
    return skills;
}

QList<const Skill *> General::getVisibleSkillList(bool relate_to_place, bool head_only) const{
    QList<const Skill *> skills;
    foreach (const Skill *skill, getSkillList(relate_to_place, head_only)) {
        if (skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const Skill *> General::getVisibleSkills(bool relate_to_place, bool head_only) const{
    return getVisibleSkillList(relate_to_place, head_only).toSet();
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

QString General::getCompanions() const{
    if (isLord())
        return tr("%1 Generals").arg(Sanguosha->translate(getKingdom()));
    QStringList name;
    foreach (QString general, companions)
        name << QString("%1").arg(Sanguosha->translate(general));
    foreach (QString gnr, Sanguosha->getGeneralNames()) {
        if (!Sanguosha->getGeneral(gnr))
            continue;
        if (Sanguosha->getGeneral(gnr)->companions.contains(objectName()))
            name << QString("%1").arg(Sanguosha->translate(gnr));
    }
    return name.join(" ");
}

QString General::getSkillDescription(bool include_name, bool inToolTip) const{
    QString description;

    foreach (const Skill *skill, getVisibleSkillList()) {
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription(inToolTip);
        desc.replace("\n", "<br/>");
        description.append(QString("<font color=%1><b>%2</b>:</font> %3 <br/> <br/>").arg(inToolTip ? Config.SkillDescriptionInToolTipColor.name() : Config.SkillDescriptionInOverviewColor.name()).arg(skill_name).arg(desc));
    }

    if (include_name) {
        QString color_str = Sanguosha->getKingdomColor(kingdom).name();
        QString name = QString("<font color=%1><b>%2</b></font>     ").arg(color_str).arg(Sanguosha->translate(objectName()));
        name.prepend(QString("<img src='image/kingdom/icon/%1.png'/>    ").arg(kingdom));
        int region = double_max_hp;
        int waken = 0;
        if (deputy_max_hp_adjusted_value != 0 || head_max_hp_adjusted_value != 0) {
            int waken1 = head_max_hp_adjusted_value;
            int waken2 = deputy_max_hp_adjusted_value;      // for wake-skill general
            region += qMax(waken1, waken2);
            waken = -qAbs(waken1 - waken2);
        }
        int i = 2;
        while (i <= region + waken) {
            name.append("<img src='image/system/magatamas/3.png' height = 12/>");
            i += 2;
        }
        if ((double_max_hp + waken) % 2) {
            if (waken == 0)
                name.append("<img src='image/system/magatamas/half.png' height = 12/>");
            else {
                name.append("<img src='image/system/magatamas/full-waken.png' height = 12/>");
                waken ++;
            }
        }
        if (waken < 0) {
            i = -2;
            while (i >= waken) {
                name.append("<img src='image/system/magatamas/3-waken.png' height = 12/>");
                i -= 2;
            }
            if (qAbs(waken) % 2)
                name.append("<img src='image/system/magatamas/half-waken.png' height = 12/>");
        }
        if (!getCompanions().isEmpty()) {
            name.append("<br/> <br/>");
            name.append(QString("<font color=%1><b>%2:</b></font>     ").arg(color_str).arg(Sanguosha->translate("Companions")));
            name.append(QString("<font color=%1>%2</font>").arg(color_str).arg(getCompanions()));
        }
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

void General::addCompanion(const QString &name) {
    this->companions << name;
}

bool General::isCompanionWith(const QString &name) const {
    const General *other = Sanguosha->getGeneral(name);
    Q_ASSERT(other);
    if (kingdom != other->kingdom)
        return false;
    return lord || other->lord || companions.contains(name)
           || other->companions.contains(objectName());
}

void General::setHeadMaxHpAdjustedValue(const int adjusted_value /* = -1 */) {
    this->head_max_hp_adjusted_value = adjusted_value;
}

void General::setDeputyMaxHpAdjustedValue(const int adjusted_value /* = -1 */) {
    this->deputy_max_hp_adjusted_value = adjusted_value;
}
