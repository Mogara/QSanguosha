#ifndef RECORDANALYSIS_H
#define RECORDANALYSIS_H

#include "client.h"
#include "engine.h"
#include "serverplayer.h"

#include <QObject>

struct PlayerRecordStruct;

class RecAnalysis : public QObject{
    Q_OBJECT

public:
    explicit RecAnalysis(QString dir = QString());

    void initialize(QString dir = QString());
    PlayerRecordStruct *getPlayerRecord(const Player *player) const;
    QMap<QString, PlayerRecordStruct *> getRecordMap() const;
    QStringList getRecordPackages() const;
    QStringList getRecordWinners() const;
    QStringList getRecordGameMode() const;
    QString getRecordChat() const;

private:
    PlayerRecordStruct *getPlayer(QString object_name, const QString &addition_name = QString());

    QMap<QString, PlayerRecordStruct *> m_recordMap;
    QStringList m_recordPackages, m_recordWinners;
    QStringList m_recordGameMode;
    QString m_recordChat;
};

struct PlayerRecordStruct{
    PlayerRecordStruct();

    bool isNull();

    QString m_additionName;
    QString m_generalName, m_general2Name;
    QString m_screenName;
    QString m_statue;
    QString m_role;
    int m_recover;
    int m_damage;
    int m_damaged;
    int m_kill;
    bool m_isAlive;
    QStringList m_designation;
};

#endif // RECORDANALYSIS_H
