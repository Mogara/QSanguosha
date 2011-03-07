#ifndef SCENARIO_H
#define SCENARIO_H

#include "package.h"

class Room;
class ScenarioRule;

#include <QMap>

class Scenario : public Package
{
    Q_OBJECT

public:
    explicit Scenario(const QString &name);    
    const ScenarioRule *getRule() const;    

    virtual bool exposeRoles() const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual void onTagSet(Room *room, const QString &key) const = 0;

protected:
    QString lord;
    QStringList loyalists, rebels, renegades;
    const ScenarioRule *rule;
};

#define ADD_SCENARIO(name) extern "C" { Q_DECL_EXPORT Scenario *New##name##Scenario() { return new name##Scenario; } }

#endif // SCENARIO_H
