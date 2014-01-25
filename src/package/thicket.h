#ifndef _THICKET_H
#define _THICKET_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "structs.h"

class ThicketPackage: public Package {
    Q_OBJECT

public:
    ThicketPackage();
};

class SavageAssaultAvoid: public TriggerSkill {
public:
    SavageAssaultAvoid(const QString &);

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
private:
    QString avoid_skill;
};

#endif

