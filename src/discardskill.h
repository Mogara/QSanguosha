#ifndef DISCARDSKILL_H
#define DISCARDSKILL_H

#include "skill.h"

class DiscardSkill : public ViewAsSkill{
    Q_OBJECT
public:
    explicit DiscardSkill();
    void setNum(int num);

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const;
    virtual const Card *viewAs(const QList<CardItem *> &cards) const;

private:
    Card *card;
    int num;
};

#endif // DISCARDSKILL_H
