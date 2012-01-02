#ifndef CARDCONTAINER_H
#define CARDCONTAINER_H

class CardItem;
class ClientPlayer;

#include "pixmap.h"
#include "carditem.h"

class CloseButton: public Pixmap{
    Q_OBJECT

public:
    CloseButton();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void clicked();
};

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
    void addCloseButton(bool dispose = false);
    void view(const ClientPlayer *player);

public slots:
    void fillCards(const QList<int> &card_ids);
    void clear();

private:
    QList<GrabCardItem *> items;
    CloseButton* close_button;

    void addCardItem(int card_id, const QPointF &pos);

private slots:
    void grabItem();
    void chooseItem();
    void gongxinItem();

signals:
    void item_chosen(int card_id);
    void item_gongxined(int card_id);
};

class GuanxingBox: public Pixmap{
    Q_OBJECT

public:
    GuanxingBox();
    void clear();
    void reply();

public slots:
    void doGuanxing(const QList<int> &card_ids, bool up_only);
    void adjust();

private:
    QList<CardItem *> up_items, down_items;
    bool up_only;

    static const qreal start_x = 30;
    static const qreal start_y1 = 40;
    static const qreal start_y2 = 184;
    static const qreal middle_y = 157;
    static const qreal skip = 102;
    static const qreal card_width = 93;
};

#endif // CARDCONTAINER_H
