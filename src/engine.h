#ifndef ENGINE_H
#define ENGINE_H

#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"
#include "audiere.h"

#include <QHash>
#include <QStringList>
#include <QMetaObject>

class AI;
class Scenario;

class Engine: public QObject
{
    Q_OBJECT

public:
    explicit Engine();

    QString translate(const QString &to_translate) const;
    void addPackage(Package *package);
    void addBanPackage(const QString &package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const QString &name, Card::Suit suit, int number) const;
    SkillCard *cloneSkillCard(const QString &name) const;
    AI *cloneAI(ServerPlayer *player) const;
    QString getVersion() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QString getSetupString() const;

    QMap<QString, QString> getAvailableModes() const;
    QString getModeName(const QString &mode) const;
    int getPlayerCount(const QString &mode) const;

    QStringList getScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const QString &name) const;

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

    void playAudio(const QString &audio, const QString &suffix = QString());
    void playEffect(const QString &filename);
    void playSkillEffect(const QString &skill_name, int index);
    void playCardEffect(const QString &card_name, bool is_male);
    void playCardEffect(const QString &card_name, const QString &package, bool is_male);
    void removeFromPlaying(audiere::OutputStreamPtr stream);

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, const Skill *> skills;
    QHash<QString, const Scenario *> scenarios;
    QMap<QString, QString> modes;

    QHash<QString, audiere::OutputStreamPtr> effects;
    QHash<QString, audiere::OutputStreamPtr> playing;
    QMutex mutex;

    QList<Card*> cards;
    QStringList lord_list, nonlord_list;
    QSet<QString> ban_package;

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
