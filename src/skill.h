#ifndef SKILL_H
#define SKILL_H

class Player;
class CardItem;
class Card;
class ServerPlayer;

#include "room.h"

#include <QObject>

class Skill : public QObject
{
    Q_OBJECT
    Q_ENUMS(Frequency);

public:
    enum Frequency{
        Frequent,
        NotFrequent,
        Compulsory
    };

    explicit Skill(const QString &name, Frequency frequent = NotFrequent);
    bool isLordSkill() const;
    QString getDescription() const;
    QString getDefaultChoice() const;

    void initMediaSource();
    void playEffect(int index = -1) const;
    void setFlag(ServerPlayer *player) const;
    void unsetFlag(ServerPlayer *player) const;
    Frequency getFrequency() const;

protected:
    Frequency frequency;
    QString default_choice;

private:
    bool lord_skill;
    QStringList sources;
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

class TriggerSkill:public Skill{
    Q_OBJECT

public:
    TriggerSkill(const QString &name);
    const ViewAsSkill *getViewAsSkill() const;
    QList<TriggerEvent> getTriggerEvents() const;

    virtual int getPriority(ServerPlayer *target) const;
    virtual bool triggerable(const ServerPlayer *target) const;    
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const = 0;

protected:
    const ViewAsSkill *view_as_skill;
    QList<TriggerEvent> events;
    QString default_choice;
};

class MasochismSkill: public TriggerSkill{
public:
    MasochismSkill(const QString &name);

    virtual int getPriority(ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const = 0;
};

class PhaseChangeSkill: public TriggerSkill{
public:
    PhaseChangeSkill(const QString &name);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
    virtual bool onPhaseChange(ServerPlayer *target) const =0;
};

class SlashBuffSkill: public TriggerSkill{
public:
    SlashBuffSkill(const QString &name);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
    virtual bool buff(const SlashEffectStruct &effect) const = 0;
};

class GameStartSkill: public TriggerSkill{
public:
    GameStartSkill(const QString &name);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
    virtual void onGameStart(ServerPlayer *player) const = 0;
};

class WeaponSkill: public TriggerSkill{
public:
    WeaponSkill(const QString &name);

    virtual bool triggerable(const ServerPlayer *target) const;
};

class ArmorSkill: public TriggerSkill{
public:
    ArmorSkill(const QString &name);

    virtual bool triggerable(const ServerPlayer *target) const;
};

#endif // SKILL_H
