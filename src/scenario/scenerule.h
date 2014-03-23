#ifndef SCENERULE_H
#define SCENERULE_H

#include "gamerule.h"


class SceneRule : public GameRule {
public:
    SceneRule(QObject *parent);
    virtual int getPriority() const;
    virtual bool effect(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
};

#endif // SCENERULE_H
