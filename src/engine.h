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

    QString translate(const QString &to_translate);
    void addPackage(Package *package);

    const General *getGeneral(const QString &name);
    int getGeneralCount() const;

    int getCardCount() const;
    Card *getCard(int index);
    QEvent::Type getEventType() const;


    void getRandomLords(QStringList &lord_list, int lord_count = 5);
    void getRandomGenerals(QStringList &general_list, int count);
    void getRandomCards(QList<int> &list);

private:
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QList<Card*> cards;
    QEvent::Type event_type;
    QStringList lord_names;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
