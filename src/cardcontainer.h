#ifndef CARDCONTAINER_H
#define CARDCONTAINER_H

class CardItem;
class ClientPlayer;

#include "pixmap.h"
#include "carditem.h"

class GrabCardItem: public CardItem{
    Q_OBJECT

public:
    GrabCardItem(const Card *card);

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void grabbed();
};

class CardContainer : public Pixmap
{
    Q_OBJECT

public:
    explicit CardContainer();
    CardItem *take(const ClientPlayer *taker, int card_id);
    int getFirstEnabled() const;
    void startChoose();
    void startGongxin();

public slots:
    void fillCards(const QList<int> &card_ids);
    void clear();

private:
    QList<GrabCardItem *> items;

    void addCardItem(int card_id, const QPointF &pos);

private slots:
    void grabItem();
    void chooseItem();
    void gongxinItem();

signals:
    void item_chosen(int card_id);
    void item_gongxined(int card_id);
};

#endif // CARDCONTAINER_H
