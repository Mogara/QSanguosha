#ifndef ENGINE_H
#define ENGINE_H

#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"

#include <QHash>
#include <QStringList>
#include <MediaObject>
#include <QMetaObject>

using Phonon::MediaObject;
using Phonon::MediaSource;

class AI;

class Engine: public QObject
{
    Q_OBJECT

public:
    explicit Engine(QObject *parent);

    QString translate(const QString &to_translate) const;
    void addPackage(Package *package);
    void addBanPackage(const QString &package_name);
    Card *cloneCard(const QString &name, Card::Suit suit, int number) const;
    SkillCard *cloneSkillCard(const QString &name);
    AI *cloneAI(ServerPlayer *player);

    const General *getGeneral(const QString &name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const QString &skill_name) const;

    int getCardCount() const;
    const Card *getCard(int index) const;

    QStringList getRandomLords() const;
    QStringList getRandomGenerals(int count, const QSet<QString> &ban_set = QSet<QString>()) const;
    QList<int> getRandomCards() const;

    void playEffect(const MediaSource &source);
    void playSkillEffect(const QString &skill_name, int index);
    void playCardEffect(const QString &card_name, bool is_male);

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, const Skill *> skills;
    QHash<QString, MediaSource> male_effects, female_effects;

    QList<MediaObject *> effects;

    QList<Card*> cards;
    QStringList lord_list, nonlord_list;
    QSet<QString> ban_package;

    QStringList getLimitedGeneralNames() const;
    void addEffect(const QString &package_name, const QString &effect_name);

private slots:
    void removeFromEffects();
};

extern Engine *Sanguosha;

#endif // ENGINE_H
