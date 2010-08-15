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
    Room *getRoom(ServerPlayer *target) const;

    void initMediaSource();
    void playEffect(int index = -1) const;

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

class ZeroCardViewAsSkill: public ViewAsSkill{
    Q_OBJECT

public:
    ZeroCardViewAsSkill(const QString &name, bool disable_after_use);

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

    virtual int getPriority(ServerPlayer *target) const;
    virtual bool triggerable(const ServerPlayer *target) const;

    virtual void onOption(ServerPlayer *target, const QString &option) const;
    virtual void getTriggerEvents(QList<Room::TriggerEvent> &events) const = 0;
    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const = 0;

    Frequency getFrequency() const;

protected:
    enum Frequency frequency;
};

class MasochismSkill: public PassiveSkill{
public:
    MasochismSkill(const QString &name);

    virtual void getTriggerEvents(QList<Room::TriggerEvent> &events) const;
    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const;

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const = 0;
};

class PhaseChangeSkill: public PassiveSkill{
public:
    PhaseChangeSkill(const QString &name);

    virtual void getTriggerEvents(QList<Room::TriggerEvent> &events) const;
    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const;

    virtual bool onPhaseChange(ServerPlayer *target) const =0;
};

class EnvironSkill: public Skill{
    Q_OBJECT
public:
    EnvironSkill(const QString &name);
};

#endif // SKILL_H
