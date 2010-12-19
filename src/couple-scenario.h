#ifndef COUPLESCENARIO_H
#define COUPLESCENARIO_H

#include "scenario.h"
#include "roomthread.h"

class ServerPlayer;

typedef QMap<PlayerStar, PlayerStar> SpouseMap;
typedef SpouseMap *SpouseMapStar;

Q_DECLARE_METATYPE(SpouseMapStar);

class CoupleScenario : public Scenario
{
    Q_OBJECT

public:
    explicit CoupleScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;

    void marryAll(Room *room) const;
    ServerPlayer *getSpouse(ServerPlayer *player) const;
    void remarry(ServerPlayer *enkemann, ServerPlayer *widow) const;
    void disposeSpouseMap(Room *room) const;
    bool isWidow(ServerPlayer *player) const;

private:
    QMap<QString, QString> map;

    void marry(ServerPlayer *husband, ServerPlayer *wife, SpouseMapStar map_star) const;
};

#endif // COUPLESCENARIO_H
