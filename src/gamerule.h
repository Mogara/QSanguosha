#ifndef GAMERULE_H
#define GAMERULE_H

#include "skill.h"

class GameRule : public TriggerSkill{
    Q_OBJECT

public:
    GameRule(QObject *parent);
    void setGameProcess(Room *room) const;

    virtual bool triggerable(const ServerPlayer *target) const;
    virtual int getPriority() const;
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

private:
    void onPhaseChange(ServerPlayer *player) const;
    void rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const;
    QString getWinner(ServerPlayer *victim) const;
};

class BossMode : public GameRule{
    Q_OBJECT

public:
    BossMode(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
};


#endif // GAMERULE_H
