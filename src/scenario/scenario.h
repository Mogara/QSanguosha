#ifndef SCENARIO_H
#define SCENARIO_H

#include "package.h"
#include "ai.h"

class Room;
class ScenarioRule;

#include <QMap>

class Scenario : public Package
{
    Q_OBJECT

public:
    explicit Scenario(const QString &name);    
    ScenarioRule *getRule() const;

    virtual bool exposeRoles() const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
    virtual void onTagSet(Room *room, const QString &key) const = 0;
    virtual bool generalSelection() const;

protected:
    QString lord;
    QStringList loyalists, rebels, renegades;
    ScenarioRule *rule;
};

class ScenarioAdder{
private:
    typedef QHash<QString, Scenario *> ScenarioHash;
    Q_GLOBAL_STATIC(ScenarioHash, Scenarios)

public:
    ScenarioAdder(const QString &name, Scenario *scenario){
        scenarios()[name] = scenario;
    }

    static ScenarioHash& scenarios(void);
};

#define ADD_SCENARIO(name) static ScenarioAdder name##ScenarioAdder(#name, new name##Scenario);

#endif // SCENARIO_H
