#ifndef BOTTOM_H
#define BOTTOM_H

#include "pixmap.h"
#include "card.h"
#include "general.h"

#include <QPushButton>

class Bottom : public Pixmap
{
    Q_OBJECT

public:
    Bottom();
    void addCard(Card *card);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QList<Card*> cards;
    General *general;
    QPixmap magatamas[5];
    QPixmap avatar;
    bool use_skill;

    void adjustCards();

private slots:
    void sortCards(int sort_type);
};

#endif // BOTTOM_H
