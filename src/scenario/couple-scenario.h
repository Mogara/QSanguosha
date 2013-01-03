#ifndef COUPLESCENARIO_H
#define COUPLESCENARIO_H

#include "scenario.h"
#include "roomthread.h"

class ServerPlayer;

class CoupleScenario : public Scenario
{
    Q_OBJECT

public:
    explicit CoupleScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

    void marryAll(Room *room) const;
    void setSpouse(ServerPlayer *player, ServerPlayer *spouse) const;
    ServerPlayer *getSpouse(const ServerPlayer *player) const;
    void remarry(ServerPlayer *enkemann, ServerPlayer *widow) const;
    bool isWidow(ServerPlayer *player) const;

private:
    QMap<QString, QString> map;
    QMap<QString, QString> full_map;

    void marry(ServerPlayer *husband, ServerPlayer *wife) const;
};

#endif // COUPLESCENARIO_H
