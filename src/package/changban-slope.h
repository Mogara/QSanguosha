#ifndef CHANGBANSLOPE_H
#define CHANGBANSLOPE_H

#include "standard.h"
#include "package.h"
#include "card.h"
#include "skill.h"
#include "player.h"
#include "socket.h"

class ChangbanSlopePackage: public Package{
    Q_OBJECT

public:
    ChangbanSlopePackage();
};

class CBLongNuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBLongNuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBYuXueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBYuXueCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBJuWuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBJuWuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class CBChanSheCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBChanSheCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBShiShenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBShiShenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // CHANGBANSLOPE_H
