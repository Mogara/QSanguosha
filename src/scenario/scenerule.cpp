#include "engine.h"
#include "scenerule.h"

SceneRule::SceneRule(QObject *parent) : GameRule(parent) {
    events << GameStart;
}

int SceneRule::getPriority() const{
    return -2;
}

bool SceneRule::trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {    
    QStringList extensions = Sanguosha->getExtensions();
    if (!player && triggerEvent == GameStart){
        foreach(QString extension, extensions){
            QString skill = QString("#%1").arg(extension);
            if (extension.startsWith("scene") && Sanguosha->getSkill(skill)){
                foreach(ServerPlayer *p, room->getPlayers())
                    room->acquireSkill(p, skill);                
            }
        }
    }
    
    return GameRule::trigger(triggerEvent, room, player, data);
}
