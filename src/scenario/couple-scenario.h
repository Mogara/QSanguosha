#ifndef _COUPLE_SCENARIO_H
#define _COUPLE_SCENARIO_H

#include "scenario.h"
#include "roomthread.h"

class ServerPlayer;

class CoupleScenario: public Scenario {
    Q_OBJECT

public:
    explicit CoupleScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

    void loadCoupleMap();
    void marryAll(Room *room) const;
    void setSpouse(ServerPlayer *player, ServerPlayer *spouse) const;
    ServerPlayer *getSpouse(const ServerPlayer *player) const;
    void remarry(ServerPlayer *enkemann, ServerPlayer *widow) const;
    bool isWidow(ServerPlayer *player) const;

    QMap<QString, QStringList> getMap(bool isHusband) const;
    QMap<QString, QString> getOriginalMap(bool isHusband) const;

private:
    QMap<QString, QStringList> husband_map;
    QMap<QString, QStringList> wife_map;
    QMap<QString, QString> original_husband_map;
    QMap<QString, QString> original_wife_map;

    void marry(ServerPlayer *husband, ServerPlayer *wife) const;
};

#endif

