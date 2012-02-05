#ifndef MINISCENARIOS_H
#define MINISCENARIOS_H

#include "scenario.h"
#include "engine.h"
#include "room.h"

class MiniSceneRule : public ScenarioRule
{
    Q_OBJECT
public:
    MiniSceneRule(Scenario *scenario);

    void assign(QStringList &generals, QStringList &roles) const;

    QStringList existedGenerals() const;

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    void addNPC(QString feature);
    void setOptions(QStringList option);
    void setPile(QString cardList);
    void loadSetting(QString path);

private:
    QList< QMap<QString, QString> > players;
    QString setup;
    QMap<QString, QVariant> ex_options;
};

class MiniScene : public Scenario
{
    Q_OBJECT

public:
    MiniScene(const QString &name);
    void setupCustom(QString name) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual void assign(QStringList &generals, QStringList &roles) const
    {
        MiniSceneRule *rule = qobject_cast<MiniSceneRule*>(getRule());
        rule->assign(generals,roles);
    }
    virtual int getPlayerCount() const
    {
        QStringList generals,roles;
        assign(generals,roles);
        return roles.length();
    }
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
        setupCustom(QString("%1").arg(name));
    }
};

#endif // MINISCENARIOS_H
