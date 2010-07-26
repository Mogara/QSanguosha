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

    Q_ENUMS(TriggerReason)

public:
    enum TriggerReason {GameStart, PhaseChange, RequestForCard, UseCard};

    explicit Skill(const QString &name);
    bool isCompulsory() const;
    bool isLordSkill() const;
    bool isFrequent() const;
    bool isToggleable() const;
    QString getDescription() const;

    virtual void attachPlayer(Player *player) const;
    virtual void trigger(Client *client, TriggerReason reason, const QString &data = QString()) const ;
    virtual void trigger(Room *room) const;

private:
    bool compulsory;
    bool lord_skill;
    bool frequent;
    bool toggleable;

    void setBooleanFlag(QString &str, QChar symbol, bool *flag);
};

#endif // SKILL_H
