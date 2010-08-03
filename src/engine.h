#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"
#include "skill.h"
#include "package.h"

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

    const General *getGeneral(const QString &name) const;
    int getGeneralCount() const;

    int getCardCount() const;
    const Card *getCard(int index) const;
    const Skill *getBasicRule() const;

    void getRandomLords(QStringList &lord_list, int lord_count = 5) const;
    void getRandomGenerals(QStringList &general_list, int count) const;
    void getRandomCards(QList<int> &list) const;

    void playEffect(const Phonon::MediaSource &source);

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QHash<QString, const QMetaObject *> metaobjects;

    QList<Card*> cards;
    QStringList lord_names;
    Skill *basic_rule;
    Phonon::MediaObject *effect;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
