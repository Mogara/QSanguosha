#ifndef ENGINE_H
#define ENGINE_H

#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"
#include "irrKlang.h"

#include <QHash>
#include <QStringList>
#include <QMetaObject>

class AI;
class Scenario;
class ChallengeModeSet;
class ChallengeMode;

struct lua_State;

class Engine: public QObject
{
    Q_OBJECT

public:
    explicit Engine();
    ~Engine();

    void addTranslationEntry(const char *key, const char *value);
    QString translate(const QString &to_translate) const;

    void loadAIs() const;
    lua_State *createLuaThread() const;
    lua_State *getLuaState() const;

    void addPackage(Package *package);
    void addBanPackage(const QString &package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const QString &name, Card::Suit suit, int number) const;
    SkillCard *cloneSkillCard(const QString &name) const;
    QString getVersion() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QString getSetupString() const;    

    QMap<QString, QString> getAvailableModes() const;
    QString getModeName(const QString &mode) const;
    int getPlayerCount(const QString &mode) const;
    void getRoles(const QString &mode, char *roles) const;
    int getRoleIndex() const;

    QStringList getScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const QString &name) const;

    const ChallengeModeSet *getChallengeModeSet() const;
    const ChallengeMode *getChallengeMode(const QString &name) const;

    const General *getGeneral(const QString &name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const QString &skill_name) const;
    const TriggerSkill *getTriggerSkill(const QString &skill_name) const;

    int getCardCount() const;
    const Card *getCard(int index) const;

    QStringList getLords() const;
    QStringList getRandomLords() const;
    QStringList getRandomGenerals(int count, const QSet<QString> &ban_set = QSet<QString>()) const;
    QList<int> getRandomCards() const;
    QString getRandomGeneralName() const;

    void playAudio(const QString &name) const;
    void playEffect(const QString &filename) const;
    void playSkillEffect(const QString &skill_name, int index) const;
    void playCardEffect(const QString &card_name, bool is_male) const;

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, const Skill *> skills;    
    QMap<QString, QString> modes;

    QHash<QString, const Scenario *> scenarios;
    ChallengeModeSet *challenge_mode_set;

    QList<Card*> cards;
    QStringList lord_list, nonlord_list;
    QSet<QString> ban_package;

    lua_State *lua;

    QStringList getLimitedGeneralNames() const;
};

extern Engine *Sanguosha;

template<typename T>
void qShuffle(QList<T> &list){
    int i, n = list.length();
    for(i=0; i<n; i++){
        int r = qrand() % n;
        list.swap(i, r);
    }
}

#endif // ENGINE_H
