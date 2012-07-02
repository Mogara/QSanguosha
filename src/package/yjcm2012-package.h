#ifndef YJCM2012PACKAGE_H
#define YJCM2012PACKAGE_H

#include "package.h"
#include "card.h"
#include "wind.h"

#include <QMutex>
#include <QGroupBox>
#include <QAbstractButton>

class YJCM2012Package: public Package{
    Q_OBJECT

public:
    YJCM2012Package();
};

class QiceCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiceCard();

    virtual Card::Suit getSuit(QList<int> cardid_list) const;
    virtual int getNumber(QList<int> cardid_list) const;
    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(const CardUseStruct *card_use) const;
    virtual const Card *validateInResposing(ServerPlayer *user, bool *continuable) const;
};

class AnxuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnxuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChunlaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif // YJCM2012PACKAGE_H
