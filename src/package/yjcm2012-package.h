#ifndef YJCM2012PACKAGE_H
#define YJCM2012PACKAGE_H

#include "package.h"
#include "card.h"

class YJCM2012Package: public Package{
    Q_OBJECT

public:
    YJCM2012Package();
};

class ZhenlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhenlieCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class AnxuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnxuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChunlaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // YJCM2012PACKAGE_H
