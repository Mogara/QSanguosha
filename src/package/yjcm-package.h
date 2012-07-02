#ifndef YJCMPACKAGE_H
#define YJCMPACKAGE_H

#include "package.h"
#include "card.h"

class YJCMPackage: public Package{
    Q_OBJECT

public:
    YJCMPackage();
};

class MingceCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MingceCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GanluCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GanluCard();
    void swapEquip(ServerPlayer *first, ServerPlayer *second) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XianzhenSlashCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianzhenSlashCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class XianzhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianzhenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuanfengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XuanfengCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JujianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JujianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuanhuoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XuanhuoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XinzhanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XinzhanCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PaiyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PaiyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif // YJCMPACKAGE_H
