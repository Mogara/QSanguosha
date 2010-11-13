#include "scenario.h"
#include "engine.h"

Scenario::Scenario(const QString &name)
    :Package(name), rule(NULL)
{
}

int Scenario::getPlayerCount() const{
    return 1 + loyalists.length() + rebels.length() + renegades.length();
}

const ScenarioRule *Scenario::getRule() const{
    return rule;
}

void Scenario::assign(QStringList &generals, QStringList &roles) const{
    generals << lord << loyalists << rebels << renegades;
    qShuffle(generals);

    foreach(QString general, generals){
        if(general == lord)
            roles << "lord";
        else if(loyalists.contains(general))
            roles << "loyalist";
        else if(rebels.contains(general))
            roles << "rebel";
        else
            roles << "renegade";
    }
}
