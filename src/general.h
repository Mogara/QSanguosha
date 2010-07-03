#ifndef GENERAL_H
#define GENERAL_H

class Skill;

#include <QObject>

class General : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int max_hp READ getMaxHp CONSTANT)
    Q_PROPERTY(bool male READ isMale STORED false CONSTANT)
    Q_PROPERTY(bool female READ isFemale STORED false CONSTANT)
    Q_PROPERTY(bool lord READ isLord CONSTANT)
    Q_PROPERTY(QObjectList skills READ getSkills WRITE setSkills)

public:
    static General *getInstance(const QString &name);
    explicit General(const QString &name, const QString &kingdom, int max_hp, bool male, const QString &pixmap_dir);

    // property getters/setters
    int getMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isLord() const;
    QObjectList getSkills() const;
    void setSkills(const QObjectList &skill_objs);

    QString getPixmapPath(const QString &category) const;
    QString getKingdomPath() const;

private:
    QString kingdom;
    int max_hp;
    bool male;
    QString pixmap_dir;
    bool lord;
    QObjectList skills;
};

#endif // GENERAL_H
