#ifndef AUXSKILLS_H
#define AUXSKILLS_H

#include "skill.h"

class DiscardSkill : public ViewAsSkill{
    Q_OBJECT

public:
    explicit DiscardSkill();

    void setNum(int num);
    void setIncludeEquip(bool include_equip);

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const;

private:
    DummyCard *card;
    int num;
    bool include_equip;
};

class CardPattern;

class ResponseSkill: public OneCardViewAsSkill{
    Q_OBJECT

public:
    ResponseSkill();
    bool matchPattern(const Player *player, const Card *card) const;

    virtual void setPattern(const QString &pattern);
    virtual bool viewFilter(const CardItem *to_select) const;
    virtual const Card *viewAs(CardItem *card_item) const;

private:
    const CardPattern *pattern;
};

class FreeDiscardSkill: public ViewAsSkill{
    Q_OBJECT

public:
    explicit FreeDiscardSkill(QObject *parent);

    virtual bool isEnabledAtPlay(const Player *) const;

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const;

private:
    DummyCard *card;
};

class YijiViewAsSkill : public ViewAsSkill{
    Q_OBJECT

public:
    explicit YijiViewAsSkill();
    void setCards(const QString &card_str);

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const;

private:
    Card *card;
    QList<int> ids;
};

class ChoosePlayerCard;

class ChoosePlayerSkill: public ZeroCardViewAsSkill{
    Q_OBJECT

public:
    explicit ChoosePlayerSkill();
    void setPlayerNames(const QStringList &names);

    virtual const Card *viewAs() const;

private:
    ChoosePlayerCard *card;
};

#endif // AUXSKILLS_H
