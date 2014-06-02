#ifndef _RECORD_ANALYSIS_H
#define _RECORD_ANALYSIS_H

#include "client.h"
#include "engine.h"
#include "serverplayer.h"

#include <QObject>

struct PlayerRecordStruct;

class RecAnalysis: public QObject {
    Q_OBJECT

public:
    explicit RecAnalysis(QString dir = QString());
    ~RecAnalysis();

    static const unsigned int M_ALL_PLAYER = 0xFFFF;
    enum DesignationType {
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
    QString getRecordGameMode() const;
    QStringList getRecordServerOptions() const;
    QString getRecordChat() const;

    void setDesignation();
    void addDesignation(const QString &designation,
                        unsigned long designation_union,
                        unsigned int data_requirement = M_ALL_PLAYER,
                        bool custom_condition = true,
                        const QString &addition_option_role = QString(),
                        bool need_alive = false,
                        bool need_dead = false,
                        bool need_win = false, bool need_lose = false);
    void initialDesignation();

private:
    PlayerRecordStruct *getPlayer(QString object_name, const QString &addition_name = QString());
    const unsigned int findPlayerOfDamage(int n) const;
    const unsigned int findPlayerOfDamaged(int n) const;
    const unsigned int findPlayerOfKills(int n) const;
    const unsigned int findPlayerOfRecover(int n) const;
    const unsigned int findPlayerOfDamage(int upper, int lower) const;
    const unsigned int findPlayerOfDamaged(int upper, int lower) const;
    const unsigned int findPlayerOfKills(int upper, int lower) const;
    const unsigned int findPlayerOfRecover(int upper, int lower) const;

    QMap<QString, PlayerRecordStruct *> m_recordMap;
    QStringList m_recordPackages, m_recordWinners;
    QString m_recordGameMode;
    QStringList m_recordServerOptions;
    QString m_recordChat;
    int m_recordPlayers;
    PlayerRecordStruct *m_currentPlayer;

    mutable QStringList m_tempSatisfiedObject;
};

struct PlayerRecordStruct {
    PlayerRecordStruct();

    bool isNull();

    QString m_additionName;
    QString m_generalName, m_general2Name;
    QString m_screenName;
    QString m_statue;
    QString m_role;
    int m_turnCount;
    int m_recover;
    int m_damage;
    int m_damaged;
    int m_kill;
    bool m_isAlive;
    QStringList m_designation;
    QList<RecAnalysis::DesignationType> m_designEnum;
};

#endif

