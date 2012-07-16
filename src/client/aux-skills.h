#ifndef AUXSKILLS_H
#define AUXSKILLS_H

#include "skill.h"

class DiscardSkill : public ViewAsSkill{
    Q_OBJECT

public:
    explicit DiscardSkill();

    void setNum(int num);
    void setMinNum(int minnum);
    void setIncludeEquip(bool include_equip);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;

private:
    DummyCard *card;
    int num;
    int minnum;
    bool include_equip;
};

class CardPattern;

class ResponseSkill: public OneCardViewAsSkill{
    Q_OBJECT

public:
    ResponseSkill();
    virtual bool matchPattern(const Player *player, const Card *card) const;

    virtual void setPattern(const QString &pattern);
    virtual bool viewFilter(const Card* to_select) const;
    virtual const Card *viewAs(const Card *originalCard) const;

protected:
    const CardPattern *pattern;
};

class ShowOrPindianSkill: public ResponseSkill{
    Q_OBJECT

public:
    ShowOrPindianSkill();
    virtual bool matchPattern(const Player *player, const Card *card) const;
};

class FreeDiscardSkill: public ViewAsSkill{
    Q_OBJECT

public:
    explicit FreeDiscardSkill(QObject *parent);

    virtual bool isEnabledAtPlay(const Player *) const;

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card* viewAs(const QList<const Card *> &cards) const;

private:
    DummyCard *card;
};

class YijiViewAsSkill : public ViewAsSkill{
    Q_OBJECT

public:
    explicit YijiViewAsSkill();
    void setCards(const QString &card_str);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card* viewAs(const QList<const Card *> &cards) const;

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
