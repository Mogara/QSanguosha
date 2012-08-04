#ifndef MINISCENARIOS_H
#define MINISCENARIOS_H

#include "scenario.h"
#include "engine.h"
#include "room.h"

class MiniSceneRule : public ScenarioRule
{
    Q_OBJECT
public:
    static const char* S_EXTRA_OPTION_LOSE_ON_DRAWPILE_DRAIN;
    static const char* S_EXTRA_OPTION_RANDOM_ROLES;

    MiniSceneRule(Scenario *scenario);
    void assign(QStringList &generals, QStringList &roles) const;
    QStringList existedGenerals() const;

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const;

    void addNPC(QString feature);
    void setOptions(QStringList option);
    void setPile(QString cardList);
    void loadSetting(QString path);

protected:
    QList< QMap<QString, QString> > players;
    QString setup;
    QMap<QString, QVariant> ex_options;
    static const QString _S_DEFAULT_HERO;
    QList<int> m_fixedDrawCards;
};

class MiniScene : public Scenario
{
    Q_OBJECT

public:
    static const char* S_KEY_MINISCENE;
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
        :MiniScene(QString(MiniScene::S_KEY_MINISCENE).arg(name))
    {
        setupCustom(name);
    }
};

#endif // MINISCENARIOS_H
