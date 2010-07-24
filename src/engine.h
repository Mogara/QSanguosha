#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"
#include "skill.h"
#include "package.h"

#include <QHash>
#include <QEvent>
#include <QStringList>

class Engine: public QObject
{
    Q_OBJECT

public:
    explicit Engine(QObject *parent);

    QString translate(const QString &to_translate) const;
    void addPackage(Package *package);

    const General *getGeneral(const QString &name) const;
    int getGeneralCount() const;

    int getCardCount() const;
    Card *getCard(int index) const;
    Card *cloneCard(const QString &name, Card::Suit suit, int number) const;
    QEvent::Type getEventType() const;

    void attachBasicRule(Player *player);

    void getRandomLords(QStringList &lord_list, int lord_count = 5) const;
    void getRandomGenerals(QStringList &general_list, int count) const;
    void getRandomCards(QList<int> &list) const;

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QList<Card*> cards;
    QEvent::Type event_type;
    QStringList lord_names;
    Skill *basic_rule;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
