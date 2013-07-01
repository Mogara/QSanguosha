#ifndef NEWSTANDARDPACKAGE_H
#define NEWSTANDARDPACKAGE_H

#include "package.h"
#include "card.h"

class NewRendeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NewRendeCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class NewStandardPackage : public Package{
    Q_OBJECT

public:
    NewStandardPackage();
};

#endif // NEWSTANDARDPACKAGE_H
