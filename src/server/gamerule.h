#ifndef _GAME_RULE_H
#define _GAME_RULE_H

#include "skill.h"

class GameRule: public TriggerSkill {
    Q_OBJECT

public:
    GameRule(QObject *parent);
    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const;
    virtual int getPriority() const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const;
    QString getWinner(ServerPlayer *victim) const;

private:
    void onPhaseProceed(ServerPlayer *player) const;
    void rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const;
};

#endif

