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
    PassiveSkill(const QString &name);

    virtual int getPriority(ServerPlayer *target, ServerPlayer *source) const;
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual void onOption(ServerPlayer *target, const QString &option) const;

    virtual void onDamage(ServerPlayer *target, ServerPlayer *source, int damage, const Card *card) const;
    virtual void onJudge(ServerPlayer *target) const;
    virtual void onJudgeOnEffect(ServerPlayer *target) const;
    virtual void onPhaseChange(ServerPlayer *target) const;
    virtual void onTargetSet(ServerPlayer *target, const Card *card) const;
    virtual void onCardUsed(ServerPlayer *target, const Card *card) const;
    virtual void onCardLost(ServerPlayer *target, const Card *card) const;

private:
    ViewAsSkill *view_as_skill;
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
