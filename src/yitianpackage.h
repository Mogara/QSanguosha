#ifndef YITIANPACKAGE_H
#define YITIANPACKAGE_H

#include "package.h"
#include "standard.h"

class YitianPackage: public Package{
    Q_OBJECT

public:
    YitianPackage();
};

class ChengxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChengxiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JuejiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JuejiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YitianSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE YitianSword(Card::Suit suit = Spade, int number = 6);

    virtual void onMove(const CardMoveStruct &move) const;
};

class LianliCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianliCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class LianliSlashCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianliSlashCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QiaocaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiaocaiCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuihanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuihanCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LexueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LexueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XunzhiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunzhiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // YITIANPACKAGE_H
