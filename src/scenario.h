#ifndef SCENARIO_H
#define SCENARIO_H

#include "package.h"

class TriggerSkill;
class Room;

#include <QMap>

class Scenario : public Package
{
    Q_OBJECT

public:
    explicit Scenario(const QString &name);
    int getPlayerCount() const;
    const TriggerSkill *getRule() const;

protected:
    QMap<QString, QString> role_map;
    const TriggerSkill *rule;
};

#define ADD_SCENARIO(name) extern "C" { Q_DECL_EXPORT Scenario *New##name##Scenario() { return new name##Scenario; } }

#endif // SCENARIO_H
