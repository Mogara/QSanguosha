#include "statistics.h"

StatisticsStruct::StatisticsStruct()
    :kill(0), damage(0), save(0), recover(0)
{
}

bool StatisticsStruct::setStatistics(const QString &name, const QVariant &value){
    if(name == "designation")
        designation << value.toString();
    else if(name == "kill")
        kill += value.toInt();
    else if(name == "damage")
        damage += value.toInt();
    else if(name == "save")
        save += value.toInt();
    else if(name == "recover")
        recover += value.toInt();
    else
        return false;

    return true;
}

