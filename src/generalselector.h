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

private:
    GeneralSelector();
    void loadFirstGeneralTable();
    void loadFirstGeneralTable(const QString &role);
    void loadSecondGeneralTable();
    void load3v3Table();

    QHash<QString, qreal> first_general_table;
    QHash<QString, int> second_general_table;
    QHash<QString, int> priority_3v3_table;
};

#endif // GENERALSELECTOR_H
