#ifndef GENERAL_H
#define GENERAL_H

#include <QObject>

class General : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString kingdom READ kingdom CONSTANT)
    Q_PROPERTY(int max_hp READ max_hp CONSTANT)
    Q_PROPERTY(int hp READ hp WRITE setHp)

public:
    explicit General(QObject *parent = 0);
    int max_hp() const;
    int hp() const;
    void setHp(int hp);
    QString kingdom() const;

private:
    int _max_hp;
    int _hp;
    QString _kingdom;
};

#endif // GENERAL_H
