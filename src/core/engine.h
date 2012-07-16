#ifndef ENGINE_H
#define ENGINE_H

#include "RoomState.h"
#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"
#include "exppattern.h"
#include "protocol.h"
#include "util.h"

#include <QHash>
#include <QStringList>
#include <QMetaObject>
#include <QThread>
#include <QList>

class AI;
class Scenario;

struct lua_State;

class Engine: public QObject
{
    Q_OBJECT

public:
    Engine();
    ~Engine();

    void addTranslationEntry(const char *key, const char *value);
    QString translate(const QString &to_translate) const;
    lua_State *getLuaState() const;

    void addPackage(Package *package);
    void addBanPackage(const QString &package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const QString &name, Card::Suit suit, int number) const;
    Card *cloneCard(const QString &name, Card::Suit suit, int number,
                    QStringList flags) const;
    SkillCard *cloneSkillCard(const QString &name) const;
    QString getVersionNumber() const;
    QString getVersion() const;
    QString getVersionName() const;
    QString getMODName() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QColor getKingdomColor(const QString &kingdom) const;
    QString getSetupString() const;

    QMap<QString, QString> getAvailableModes() const;
    QString getModeName(const QString &mode) const;
    int getPlayerCount(const QString &mode) const;
    QString getRoles(const QString &mode) const;
    QStringList getRoleList(const QString &mode) const;
    int getRoleIndex() const;

    const CardPattern *getPattern(const QString &name) const;
    QList<const Skill *> getRelatedSkills(const QString &skill_name) const;

    QStringList getScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const QString &name) const;

    void addPackage(const QString &name);
    void addScenario(const QString &name);

    const General *getGeneral(const QString &name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const QString &skill_name) const;
    QStringList getSkillNames() const;
    const TriggerSkill *getTriggerSkill(const QString &skill_name) const;
    const ViewAsSkill *getViewAsSkill(const QString &skill_name) const;
    QList<const DistanceSkill *> getDistanceSkills() const;
    QList<const MaxCardsSkill *> getMaxCardsSkills() const;
    void addSkills(const QList<const Skill *> &skills);

    int getCardCount() const;
    const Card *getEngineCard(int cardId) const;
    const Card *getCard(int cardId) const;

    QStringList getLords() const;
    QStringList getRandomLords() const;
    QStringList getRandomGenerals(int count, const QSet<QString> &ban_set = QSet<QString>()) const;
    QList<int> getRandomCards() const;
    QString getRandomGeneralName() const;
    QStringList getLimitedGeneralNames() const;

    void playSystemAudioEffect(const QString &name) const;
    void playAudioEffect(const QString &filename) const;
    void playSkillAudioEffect(const QString &skill_name, int index) const;

    const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card) const;
    int correctDistance(const Player *from, const Player *to) const;
    int correctMaxCards(const Player *target) const;

    void registerRoom(QObject* room);
    void unregisterRoom();
    QObject* currentRoom() const;

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals, hidden_generals;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, const Skill *> skills;
    QHash<QThread *, QObject *> m_rooms;
    QMap<QString, QString> modes;
    QMap<QString, const CardPattern *> patterns;
    QMultiMap<QString, QString> related_skills;

    // special skills
    QList<const ProhibitSkill *> prohibit_skills;
    QList<const DistanceSkill *> distance_skills;
    QList<const MaxCardsSkill *> maxcards_skills;

    QHash<QString, const Scenario *> scenarios;

    QList<Card*> cards;
    QStringList lord_list, nonlord_list;
    QSet<QString> ban_package;

    lua_State *lua;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
