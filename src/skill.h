#ifndef SKILL_H
#define SKILL_H

#include "room.h"

#include <QObject>

class Skill : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool compulsory READ isCompulsory CONSTANT)
    Q_PROPERTY(bool lord_skill READ isLordSkill CONSTANT)
    Q_PROPERTY(bool frequent READ isFrequent CONSTANT)

public:
    explicit Skill(const QString &name);
    bool isCompulsory() const;
    bool isLordSkill() const;
    bool isFrequent() const;
    bool isToggleable() const;
    QString getDescription() const;

    virtual void attachPlayer(Player *player) const;
    virtual void trigger(Client *client) const ;
    virtual void trigger(Room *room) const;

private:
    bool compulsory;
    bool lord_skill;
    bool frequent;
    bool toggleable;

    void setBooleanFlag(QString &str, QChar symbol, bool *flag);
};

#endif // SKILL_H
