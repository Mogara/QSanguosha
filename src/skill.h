#ifndef SKILL_H
#define SKILL_H

class Player;
class CardItem;
class Card;
class ServerPlayer;
struct ActiveRecord;

#include <QObject>
#include <MediaSource>

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

    // virtual handlers
    virtual ActiveRecord *onGameStart(ServerPlayer *target) const;
    virtual ActiveRecord *onPhaseChange(ServerPlayer *target) const;
//    virtual void onRequestForCard();
//    virtual void onUseCard();
//    virtual void onMoveCard();
//    virtual void onPredamage(ServerPlayer *source, int damage, const Card *card);
//    virtual void onDamage(ServerPlayer *source, int damage, const Card *card);
//    virtual void onJudge();
//    virtual void onJudgeOnEffect();
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
