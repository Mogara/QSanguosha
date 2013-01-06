#ifndef SCENERULE_H
#define SCENERULE_H

#include "gamerule.h"

class Scene26Effect;
class SceneRule : public GameRule {
public:
	SceneRule(QObject *parent);

        virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const;
};

#endif // SCENERULE_H
