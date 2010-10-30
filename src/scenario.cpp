#include "scenario.h"

Scenario::Scenario(const QString &name)
    :Package(name), rule(NULL)
{
}

int Scenario::getPlayerCount() const{
    return role_map.size();
}

const TriggerSkill *Scenario::getRule() const{
    return rule;
}

