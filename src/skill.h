#ifndef SKILL_H
#define SKILL_H

#include "room.h"

#include <QObject>
#include <MediaSource>

class CardItem;

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
        Judge,
        Nop
    };

    explicit Skill(const QString &name);
    bool isCompulsory() const;
    bool isLordSkill() const;
    bool isFrequent() const;
    QString getDescription() const;

    void initMediaSource();
    void playEffect() const;

    virtual void attachPlayer(Player *player) const;
    virtual void trigger(TriggerReason reason, const QVariant &data) const ;
    virtual void trigger(Room *room) const;

private:
    bool compulsory;
    bool lord_skill;
    bool frequent;
    QList<Phonon::MediaSource> sources;

    void setBooleanFlag(QString &str, QChar symbol, bool *flag);
};

class ViewAsSkill:public Skill{
    Q_OBJECT

public:
    ViewAsSkill(const QString &name, bool disable_after_use);
    virtual void attachPlayer(Player *player) const;
    bool isDisableAfterUse() const;

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const = 0;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const = 0;

private:
    bool disable_after_use;
};

#endif // SKILL_H
