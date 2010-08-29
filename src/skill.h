#ifndef SKILL_H
#define SKILL_H

class Player;
class CardItem;
class Card;
class ServerPlayer;

#include "room.h"

#include <QObject>
#include <MediaSource>

class Skill : public QObject
{
    Q_OBJECT

public:
    explicit Skill(const QString &name);
    bool isLordSkill() const;
    QString getDescription() const;

    void initMediaSource();
    void playEffect(int index = -1) const;

private:
    bool lord_skill;
    QList<Phonon::MediaSource> sources;
};

class ViewAsSkill:public Skill{
    Q_OBJECT

public:
    ViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const = 0;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const = 0;

    bool isAvailable() const;
protected:

    virtual bool isEnabledAtPlay() const;
    virtual bool isEnabledAtResponse() const;
};

class ZeroCardViewAsSkill: public ViewAsSkill{
    Q_OBJECT

public:
    ZeroCardViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const;
    virtual const Card *viewAs() const = 0;
};

class FilterSkill: public ViewAsSkill{
    Q_OBJECT
public:
    FilterSkill(const QString &name);
};

class PassiveSkill:public Skill{
    Q_OBJECT

public:
    enum Frequency{
        Frequent,
        NotFrequent,
        Compulsory
    };

    PassiveSkill(const QString &name, Frequency frequency = NotFrequent);
    const ViewAsSkill *getViewAsSkill() const;

    virtual int getPriority(ServerPlayer *target) const;
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual void getTriggerEvents(QList<TriggerEvent> &events) const = 0;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const = 0;

    Frequency getFrequency() const;

protected:
    enum Frequency frequency;
    const ViewAsSkill *view_as_skill;
};

class MasochismSkill: public PassiveSkill{
public:
    MasochismSkill(const QString &name);

    virtual void getTriggerEvents(QList<TriggerEvent> &events) const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const;

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const = 0;
};

class PhaseChangeSkill: public PassiveSkill{
public:
    PhaseChangeSkill(const QString &name);

    virtual void getTriggerEvents(QList<TriggerEvent> &events) const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const;

    virtual bool onPhaseChange(ServerPlayer *target) const =0;
};

class EnvironSkill: public PassiveSkill{
    Q_OBJECT

public:
    EnvironSkill(const QString &name);
    virtual void getTriggerEvents(QList<TriggerEvent> &events) const;
};

#endif // SKILL_H
