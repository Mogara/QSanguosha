#ifndef GENERAL_H
#define GENERAL_H

#include <QObject>

class General : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int max_hp READ getMaxHp CONSTANT)
    Q_PROPERTY(bool male READ isMale STORED false CONSTANT)
    Q_PROPERTY(bool female READ isFemale STORED false CONSTANT)
    Q_PROPERTY(bool leader READ isLeader CONSTANT)

public:
    static General *getInstance(const QString &name);
    explicit General(const QString &name, const QString &kingdom, int max_hp, bool male, const QString &pixmap_dir);

    // property getters/setters
    int getMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isLeader() const;

    QString getPixmapPath(const QString &category) const;
    QString getKingdomPath() const;

private:
    QString kingdom;
    int max_hp;
    bool male;
    QString pixmap_dir;
    bool leader;
};

#endif // GENERAL_H
