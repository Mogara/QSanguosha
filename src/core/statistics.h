#ifndef STATISTICS_H
#define STATISTICS_H
#include <QVariant>
#include <QStringList>

struct StatisticsStruct{
    StatisticsStruct();
    bool setStatistics(const QString &name, const QVariant &value);

    int kill;
    int damage;
    int save;
    int recover;
    QStringList designation;
};

#endif // STATISTICS_H
