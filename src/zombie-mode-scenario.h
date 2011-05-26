#ifndef DUMMYHONYANSCENATIO_H
#define DUMMYHONYANSCENATIO_H

#include "scenario.h"
#include "standard-skillcards.h"
#include "maneuvering.h"
#include "zombie-package.h"


class ZombieScenario : public Scenario{
    Q_OBJECT

public:

    explicit ZombieScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;

private:
    QStringList females;
    QStringList standard_roles;

};

#endif // DUMMYHONYANSCENATIO_H
