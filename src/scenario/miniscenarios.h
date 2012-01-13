#ifndef MINISCENARIOS_H
#define MINISCENARIOS_H

#include "scenario.h"

class MiniScene : public Scenario
{
    Q_OBJECT

public:
    MiniScene(const QString &name)
        :Scenario(name){};
    void setupCustom(QString name);
    virtual void onTagSet(Room *room, const QString &key) const;
};

class CustomScenario : public MiniScene
{
    Q_OBJECT
public:
    CustomScenario()
        :MiniScene("custom_scenario")
    {
        setupCustom(NULL);
    }
};

class LoadedScenario : public MiniScene
{
    Q_OBJECT
public:
    LoadedScenario(const QString &name)
        :MiniScene(QString("_mini_%1").arg(name))
    {
        setupCustom(QString("customScenes/%1").arg(name));
    }
};

#endif // MINISCENARIOS_H
