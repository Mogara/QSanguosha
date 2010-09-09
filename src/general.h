#ifndef GENERAL_H
#define GENERAL_H

class Skill;
class Package;

#include <QObject>
#include <QSet>
#include <MediaSource>

using Phonon::MediaSource;

class General : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int max_hp READ getMaxHp CONSTANT)
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
    QString getKingdomPath() const;
    QString getPackage() const;
    void playEffect(const QString &skill_name) const;
    void lastWord() const;

private:
    QString kingdom;
    int max_hp;
    bool male;
    bool lord;
    QMap<QString, Skill *> skill_map;
    MediaSource last_word;
};

#endif // GENERAL_H
