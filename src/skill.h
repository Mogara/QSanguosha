#ifndef SKILL_H
#define SKILL_H

#include "room.h"

#include <QObject>
#include <QScriptValue>

class Skill : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool compulsory READ isCompulsory CONSTANT)
    Q_PROPERTY(bool lord_skill READ isLordSkill CONSTANT)
    Q_PROPERTY(bool frequent READ isFrequent CONSTANT)

public:
    explicit Skill(const QString &name, const QScriptValue &value, QObject *parent);
    void doCallback(Room *room);
    bool isCompulsory() const;
    bool isLordSkill() const;
    bool isFrequent() const;

private:
    QScriptValue value;
    bool compulsory;
    bool lord_skill;
    bool frequent;

    void setBooleanFlag(QString &str, QChar symbol, bool *flag);
};

#endif // SKILL_H
