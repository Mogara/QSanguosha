#ifndef _GENERAL_SELECTOR_H
#define _GENERAL_SELECTOR_H

#include <QObject>
#include <QHash>
#include <QGenericMatrix>

class ServerPlayer;

// singleton class
class GeneralSelector: public QObject {
    Q_OBJECT

public:
    static GeneralSelector *getInstance();
    QString selectFirst(ServerPlayer *player, const QStringList &candidates);
    QString selectSecond(ServerPlayer *player, const QStringList &candidates);

private:
    GeneralSelector();
    void loadFirstGeneralTable();
    void loadFirstGeneralTable(const QString &role);
    void loadSecondGeneralTable();
    QString selectHighest(const QHash<QString, int> &table, const QStringList &candidates, int default_value);

    QHash<QString, qreal> first_general_table;
    QHash<QString, int> second_general_table;
    QSet<QString> sacrifice;
};

#endif

