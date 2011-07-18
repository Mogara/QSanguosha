#ifndef MOUNTAINPACKAGE_H
#define MOUNTAINPACKAGE_H

#include "package.h"
#include "card.h"

class QiaobianCard: public SkillCard{
    Q_OBJECT

public:
    QiaobianCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MountainPackage : public Package
{
    Q_OBJECT

public:
    MountainPackage();
};

#endif // MOUNTAINPACKAGE_H
