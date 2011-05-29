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

private:
    GeneralSelector();
    void loadFirstGeneralTable();
    void loadFirstGeneralTable(const QString &role);
    void loadSecondGeneralTable();

    QHash<QString, int> first_general_table;
    QHash<QString, int> second_general_table;
};

#endif // GENERALSELECTOR_H
