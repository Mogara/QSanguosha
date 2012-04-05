#ifndef GENERALSELECTOR_H
#define GENERALSELECTOR_H

#include <QObject>
#include <QHash>
#include <QGenericMatrix>

class ServerPlayer;

// singleton class
class GeneralSelector: public QObject{
    Q_OBJECT

public:
    static GeneralSelector *GetInstance();
    QString selectFirst(ServerPlayer *player, const QStringList &candidates);
    QString selectSecond(ServerPlayer *player, const QStringList &candidates);
    QString select3v3(ServerPlayer *player, const QStringList &candidates);
    QString select1v1(const QStringList &candidates);
    QStringList arrange3v3(ServerPlayer *player);
    QStringList arrange1v1(ServerPlayer *player);
    int get1v1ArrangeValue(const QString &name);

private:
    GeneralSelector();
    void loadFirstGeneralTable();
    void loadFirstGeneralTable(const QString &role);
    void loadSecondGeneralTable();
    void load3v3Table();
    void load1v1Table();
    QString selectHighest(const QHash<QString, int> &table, const QStringList &candidates, int default_value);

    QHash<QString, qreal> first_general_table;
    QHash<QString, int> second_general_table;
    QHash<QString, int> priority_3v3_table;
    QHash<QString, int> priority_1v1_table;
    QSet<QString> sacrifice;
};

#endif // GENERALSELECTOR_H
