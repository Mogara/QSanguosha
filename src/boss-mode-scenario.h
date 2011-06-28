#ifndef BOSSCHALLENGE_H
#define BOSSCHALLENGE_H

#include "scenario.h"
#include "maneuvering.h"

class ImpasseScenario : public Scenario{
    Q_OBJECT

public:
    explicit ImpasseScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
};

#endif // BOSSCHALLENGE_H
