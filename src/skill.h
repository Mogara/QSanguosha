#ifndef SKILL_H
#define SKILL_H

#include "room.h"

#include <QObject>
#include <MediaSource>

class CardItem;

class Skill : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool lord_skill READ isLordSkill CONSTANT)

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
    bool isLordSkill() const;
    QString getDescription() const;

    void initMediaSource();
    void playEffect() const;

    virtual void attachPlayer(Player *player) const;
    virtual void trigger(TriggerReason reason, const QVariant &data) const ;
    virtual void trigger(Room *room) const;

private:
    bool lord_skill;
    QList<Phonon::MediaSource> sources;
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

class FilterSkill: public ViewAsSkill{
    Q_OBJECT
public:
    FilterSkill(const QString &name);
};

class PassiveSkill:public Skill{
    Q_OBJECT
public:
    PassiveSkill(const QString &name);
};

class FrequentPassiveSkill: public PassiveSkill{
    Q_OBJECT
public:
    FrequentPassiveSkill(const QString &name);
};

class EnvironSkill: public Skill{
    Q_OBJECT
public:
    EnvironSkill(const QString &name);
};

#endif // SKILL_H
