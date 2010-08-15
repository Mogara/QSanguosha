#ifndef ENGINE_H
#define ENGINE_H

#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"
#include "cardpattern.h"

#include <QHash>
#include <QStringList>
#include <MediaObject>
#include <QMetaObject>

class Engine: public QObject
{
    Q_OBJECT

public:
    explicit Engine(QObject *parent);

    QString translate(const QString &to_translate) const;
    void addPackage(Package *package);
    Card *cloneCard(const QString &name, Card::Suit suit, int number) const;
    SkillCard *cloneSkillCard(const QString &name);
    CardPattern *cloneCardPattern(const QString &pattern_text);

    const General *getGeneral(const QString &name) const;
    int getGeneralCount() const;
    const Skill *getSkill(const QString &skill_name) const;

    int getCardCount() const;
    const Card *getCard(int index) const;

    QStringList getRandomLords(int lord_count) const;
    QStringList getRandomGenerals(int count) const;
    QList<int> getRandomCards() const;

    void playEffect(const Phonon::MediaSource &source);
    void playSkillEffect(const QString &skill_name, int index);

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, const Skill *> skills;

    QList<Card*> cards;
    QStringList lord_names;
    Phonon::MediaObject *effect;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
