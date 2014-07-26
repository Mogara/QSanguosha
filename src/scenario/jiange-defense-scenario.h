#ifndef _JIANGE_DEFENSE_H
#define _JIANGE_DEFENSE_H

#include "scenario.h"

class ServerPlayer;

class JiangeDefenseScenario: public Scenario {
    Q_OBJECT

public:
    explicit JiangeDefenseScenario();

    virtual void assign(QStringList &generals, QStringList &generals2, QStringList &kingdoms, Room *room) const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    virtual void onTagSet(Room *room, const QString &key) const;
    
    QString getRandomWeiGhost() const;
    QString getRandomWeiMachine() const;
    QString getRandomShuGhost() const;
    QString getRandomShuMachine() const;
};

#endif