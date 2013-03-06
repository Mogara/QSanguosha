#include "engine.h"
#include "scenerule.h"

SceneRule::SceneRule(QObject *parent) : GameRule(parent) {
    events << GameStart;
}

int SceneRule::getPriority() const{
    return -2;
}

bool SceneRule::trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
    if (!player && triggerEvent == GameStart  && Sanguosha->getSkill("#scenerule")){
        foreach(ServerPlayer *p, room->getPlayers()){
            room->acquireSkill(p, "#scenerule");
        }        
    }
    
    return GameRule::trigger(triggerEvent, room, player, data);
}
