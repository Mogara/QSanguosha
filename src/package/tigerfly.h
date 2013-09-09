#ifndef _TIGERFLY_H
#define _TIGERFLY_H

#include "package.h"
#include "card.h"

class PozhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PozhenCard();
    virtual bool targetFilter(const QList<const Player *> &, const Player *, const Player *) const;
    virtual void onEffect(const CardEffectStruct &) const;

private:
    QString suittb(Card::Suit s) const;
};

class TushouGiveCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TushouGiveCard();
    virtual bool targetFilter(const QList<const Player *> &, const Player *, const Player *) const;
    virtual void onEffect(const CardEffectStruct &) const;

};

class TigerFlyPackage: public Package {
	Q_OBJECT

public:
	TigerFlyPackage();
};

#endif