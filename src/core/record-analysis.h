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

    enum DesignationType{
        NoOption = 0x00,
        MostKill = 0x01,
        MostRecover = 0x02,
        MostDamage = 0x04,
        MostDamaged = 0x08,
        LeastKill = 0x10,
        LeastRecover = 0x20,
        LeastDamage = 0x40,
        LeastDamaged = 0x80,
        ZeroKill = 0x100,
        ZeroRecover = 0x200,
        ZeroDamage = 0x400,
        ZeroDamaged = 0x800
    };

    void initialize(QString dir = QString());
    PlayerRecordStruct *getPlayerRecord(const Player *player) const;
    QMap<QString, PlayerRecordStruct *> getRecordMap() const;
    QStringList getRecordPackages() const;
    QStringList getRecordWinners() const;
    QStringList getRecordGameMode() const;
    QString getRecordChat() const;

    void setDesignation();
    void addDesignation(const QString &designation,
                        unsigned long designation_union,
                        bool custom_condition = true,
                        const QString &addition_option_role = QString(),
                        bool need_alive = false,
                        bool need_dead = false,
                        bool need_win = false,
                        bool need_lose = false);
    void initialDesignation();

private:
    PlayerRecordStruct *getPlayer(QString object_name, const QString &addition_name = QString());
    const bool findPlayerOfDamage(int n) const;
    const bool findPlayerOfDamaged(int n) const;
    const bool findPlayerOfKills(int n) const;
    const bool findPlayerOfRecover(int n) const;
    const bool findPlayerOfDamage(int upper, int lower) const;
    const bool findPlayerOfDamaged(int upper, int lower) const;
    const bool findPlayerOfKills(int upper, int lower) const;
    const bool findPlayerOfRecover(int upper, int lower) const;

    QMap<QString, PlayerRecordStruct *> m_recordMap;
    QStringList m_recordPackages, m_recordWinners;
    QStringList m_recordGameMode;
    QString m_recordChat;
    int m_recordPlayers;

    bool m_recordHasCheat;
    mutable QStringList m_tempSatisfiedObject;
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
    QList<RecAnalysis::DesignationType> m_designEnum;
};

#endif // RECORDANALYSIS_H
