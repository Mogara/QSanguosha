#ifndef _GENERAL_H
#define _GENERAL_H

class Skill;
class TriggerSkill;
class Package;
class QSize;

#include <QObject>
#include <QSet>
#include <QMap>
#include <QStringList>

class General: public QObject {
    Q_OBJECT
    Q_ENUMS(Gender)
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int maxhp READ getDoubleMaxHp CONSTANT)
    Q_PROPERTY(bool male READ isMale STORED false CONSTANT)
    Q_PROPERTY(bool female READ isFemale STORED false CONSTANT)
    Q_PROPERTY(Gender gender READ getGender CONSTANT)
    Q_PROPERTY(bool lord READ isLord CONSTANT)
    Q_PROPERTY(bool hidden READ isHidden CONSTANT)

public:
    explicit General(Package *package, const QString &name, const QString &kingdom,
                     int double_max_hp = 4, bool male = true, bool hidden = false, bool never_shown = false);

    // property getters/setters
    int getDoubleMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isNeuter() const;
    bool isLord() const;
    bool isHidden() const;
    bool isTotallyHidden() const;

    int getMaxHpHead() const;
    int getMaxHpDeputy() const;

    enum Gender { Sexless, Male, Female, Neuter };
    Gender getGender() const;
    void setGender(Gender gender);

    void addSkill(Skill *skill);
    void addSkill(const QString &skill_name);
    bool hasSkill(const QString &skill_name) const;
    QList<const Skill *> getSkillList(bool relate_to_place = false, bool head_only = true) const;
    QList<const Skill *> getVisibleSkillList(bool relate_to_place = false, bool head_only = true) const;
    QSet<const Skill *> getVisibleSkills(bool relate_to_place = false, bool head_only = true) const;
    QSet<const TriggerSkill *> getTriggerSkills() const;

    void addRelateSkill(const QString &skill_name);
    QStringList getRelatedSkillNames() const;

    QString getPackage() const;
    QString getCompanions() const;
    QString getSkillDescription(bool include_name = false, bool inToolTip = true) const;

    inline QSet<QString> getExtraSkillSet() const{ return extra_set; }

    void addCompanion(const QString &name);
    bool isCompanionWith(const QString &name) const;

    void setHeadMaxHpAdjustedValue(const int adjusted_value = -1);
    void setDeputyMaxHpAdjustedValue(const int adjusted_value = -1);

public slots:
    void lastWord() const;

private:
    QString kingdom;
    int double_max_hp;
    Gender gender;
    bool lord;
    QSet<QString> skill_set;
    QSet<QString> extra_set;
    QStringList skillname_list;
    QStringList related_skills;
    bool hidden;
    bool never_shown;
    QStringList companions;
    int head_max_hp_adjusted_value;
    int deputy_max_hp_adjusted_value;
};

#endif

