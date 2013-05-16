#ifndef _BGM_H
#define _BGM_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class BGMPackage: public Package {
    Q_OBJECT

public:
    BGMPackage();
};

class LihunCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LihunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DaheCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DaheCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TanhuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TanhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShichouCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShichouCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YanxiaoCard: public DelayedTrick {
    Q_OBJECT

public:
    Q_INVOKABLE YanxiaoCard(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void takeEffect(ServerPlayer *) const;

    virtual QString getType() const{ return "skill_card"; }
    virtual QString getSubtype() const{ return "skill_card"; }
    virtual CardType getTypeId() const{ return TypeSkill; }
    virtual bool isKindOf(const char *cardType) const{
        if (strcmp(cardType, "SkillCard") == 0)
            return true;
        else
            return inherits(cardType);
    }

};

class YinlingCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE YinlingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BGMDIYPackage: public Package {
    Q_OBJECT

public:
    BGMDIYPackage();
};

class ZhaoxinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhaoxinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FuluanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE FuluanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuangenCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuangenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HantongCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HantongCard();
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class DIYYicongCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DIYYicongCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif

