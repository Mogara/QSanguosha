#include "scenario.h"
#include "engine.h"

Scenario::Scenario(const QString &name)
    :Package(name), rule(NULL)
{
}

int Scenario::getPlayerCount() const{
    return role_map.size();
}

const ScenarioRule *Scenario::getRule() const{
    return rule;
}

void Scenario::assign(QStringList &generals, QStringList &roles) const{
    generals = role_map.keys();

    qShuffle(generals);

    foreach(QString general, generals){
        roles << role_map.value(general);
    }
}
