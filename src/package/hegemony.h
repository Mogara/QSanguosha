#ifndef HEGEMONY_H
#define HEGEMONY_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class HegemonyPackage: public Package{
    Q_OBJECT

public:
    HegemonyPackage();
};

class DuoshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuoshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FenxunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // HEGEMONY_H
