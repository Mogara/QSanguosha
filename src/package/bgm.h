#ifndef BGMPACKAGE_H
#define BGMPACKAGE_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class BGMPackage : public Package{
    Q_OBJECT

public:
    BGMPackage();
};

class LihunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LihunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DaheCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DaheCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TanhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TanhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanxiaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanxiaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YinlingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YinlingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

//bgm diy
class PasterPackage : public Package{
    Q_OBJECT

public:
    PasterPackage();
};

class FuluanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FuluanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Yic0ngCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE Yic0ngCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MingjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MingjianCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // BGMPACKAGE_H
