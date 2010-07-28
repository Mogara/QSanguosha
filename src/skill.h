#ifndef SKILL_H
#define SKILL_H

#include "room.h"

#include <QObject>
#include <MediaSource>

class Skill : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool compulsory READ isCompulsory CONSTANT)
    Q_PROPERTY(bool lord_skill READ isLordSkill CONSTANT)
    Q_PROPERTY(bool frequent READ isFrequent CONSTANT)

    Q_ENUMS(TriggerReason)

public:
    enum TriggerReason {
        GameStart,
        PhaseChange,
        RequestForCard,
        UseCard,
        MoveCard,
        HpDamage,
        Judge
    };

    explicit Skill(const QString &name);
    bool isCompulsory() const;
    bool isLordSkill() const;
    bool isFrequent() const;
    bool isToggleable() const;
    QString getDescription() const;

    void initMediaSource();
    void playEffect() const;

    virtual void attachPlayer(Player *player) const;
    virtual void trigger(Client *client, TriggerReason reason, const QVariant &data) const ;
    virtual void trigger(Room *room) const;

private:
    bool compulsory;
    bool lord_skill;
    bool frequent;
    bool toggleable;
    QList<Phonon::MediaSource> sources;

    void setBooleanFlag(QString &str, QChar symbol, bool *flag);
};

#endif // SKILL_H
