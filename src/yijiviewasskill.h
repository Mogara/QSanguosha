#ifndef YIJIVIEWASSKILL_H
#define YIJIVIEWASSKILL_H

#include "skill.h"

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

#endif // YIJIVIEWASSKILL_H
