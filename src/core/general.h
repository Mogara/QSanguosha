#ifndef GENERAL_H
#define GENERAL_H

class Skill;
class TriggerSkill;
class Package;
class QSize;

#include <QObject>
#include <QSet>
#include <QMap>
#include <QStringList>

class General : public QObject
{
    Q_OBJECT
    Q_ENUMS(Gender)
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int maxhp READ getMaxHp CONSTANT)
    Q_PROPERTY(bool male READ isMale STORED false CONSTANT)
    Q_PROPERTY(bool female READ isFemale STORED false CONSTANT)
    Q_PROPERTY(Gender gender READ getGender CONSTANT)
    Q_PROPERTY(bool lord READ isLord CONSTANT)
    Q_PROPERTY(bool hidden READ isHidden CONSTANT)
    Q_PROPERTY(QString designer READ getDesigner CONSTANT)
    Q_PROPERTY(QString illustrator READ getIllustrator CONSTANT)
    Q_PROPERTY(QString cv READ getCV CONSTANT)

public:
    explicit General(Package *package, const QString &name, const QString &kingdom, int max_hp = 4, bool male = true, bool hidden = false, bool never_shown = false);

    // property getters/setters
    int getMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isNeuter() const;
    bool isLord() const;
    bool isHidden() const;
    bool isTotallyHidden() const;

    enum Gender {Male, Female, Neuter};
    Gender getGender() const;
    void setGender(Gender gender);

    void addSkill(Skill* skill);
    void addSkill(const QString &skill_name);
    bool hasSkill(const QString &skill_name) const;
    QList<const Skill *> getVisibleSkillList() const;
    QSet<const Skill *> getVisibleSkills() const;
    QSet<const TriggerSkill *> getTriggerSkills() const;

    void addRelateSkill(const QString &skill_name);
    QStringList getRelatedSkillNames() const;

    QString getLastEffectPath() const;
    QString getWinEffectPath() const;

    bool isCaoCao() const;
    bool isZhugeliang() const;
    bool nameContains(const QString &name) const;

    QString getDesigner() const;
    QString getIllustrator() const;
    QString getCV() const;

    static QSize BigIconSize;
    static QSize SmallIconSize;
    static QSize TinyIconSize;

public slots:
    QString getSkillDescription() const;
    QString getPixmapPath(const QString &category) const;
    QString getPackage() const;
    QString getGenderString() const;
    QString getLastWord() const;
    QString getWinWord() const;

    void playLastWord() const;
    void playWinWord() const;

private:
    QString kingdom;
    int max_hp;
    Gender gender;
    bool lord;
    QSet<QString> skill_set;
    QSet<QString> extra_set;
    QStringList related_skills;
    bool hidden;
    bool never_shown;
};

typedef QList<const General *> GeneralList;

#endif // GENERAL_H
