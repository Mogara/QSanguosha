#ifndef GENERAL_H
#define GENERAL_H

class Skill;
class Package;

#include <QObject>
#include <QSet>
#include <QMap>

class General : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int maxhp READ getMaxHp CONSTANT)
    Q_PROPERTY(bool male READ isMale STORED false CONSTANT)
    Q_PROPERTY(bool female READ isFemale STORED false CONSTANT)
    Q_PROPERTY(bool lord READ isLord CONSTANT)   

public:
    explicit General(Package *package, const QString &name, const QString &kingdom, int max_hp = 4, bool male = true);

    // property getters/setters
    int getMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isLord() const;

    void addSkill(Skill* skill);
    bool hasSkill(const QString &skill_name) const;

    QString getPixmapPath(const QString &category) const;    
    QString getPackage() const;
    QString getSkillDescription() const;

    void setHyde(General *hyde);
    const General *getHyde() const;

public slots:
    void lastWord() const;

private:
    QString kingdom;
    int max_hp;
    bool male;
    bool lord;
    QMap<QString, Skill *> skill_map;
    General *hyde;
};

#endif // GENERAL_H
