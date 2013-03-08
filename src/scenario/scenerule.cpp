#include "settings.h"
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
    QSet<QString> ban_packages = Config.BanPackages.toSet();
    
    if (!player && triggerEvent == GameStart){
        foreach(QString extension, extensions){
            bool forbid_package = Config.value("ForbidPackages").toStringList().contains(extension);
            if (ban_packages.contains(extension) || forbid_package) continue;

            QString skill = QString("#%1").arg(extension);
            if (extension.startsWith("scene") && Sanguosha->getSkill(skill)){
                foreach(ServerPlayer *p, room->getPlayers())
                    room->acquireSkill(p, skill);                
            }
        }
    }
    
    return GameRule::trigger(triggerEvent, room, player, data);
}
