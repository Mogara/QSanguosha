#ifndef CARDITEM_H
#define CARDITEM_H

#include "card.h"
#include "pixmap.h"

#include <QSize>

class FilterSkill;

class CardItem : public Pixmap
{
    Q_OBJECT

public:
    CardItem(const Card *card);
    CardItem(const QString &general_name);

    void filter(const FilterSkill *filter_skill);
    const Card *getFilteredCard() const;

    const Card *getCard() const;
    void setHomePos(QPointF home_pos);
    QPointF homePos() const;
    void goBack(bool kieru = false);
    const QPixmap &getSuitPixmap() const;
    const QPixmap &getIconPixmap() const;
    void setFrame(const QString &frame);
    void hideFrame();

    void select();
    void unselect();
    bool isPending() const;
    bool isEquipped() const;

    static const int NormalY = 36;
    static const int PendingY = NormalY - 40;
    static CardItem *FindItem(const QList<CardItem *> &items, int card_id);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);    

private:
    const Card *card, *filtered_card;
    QPixmap suit_pixmap, icon_pixmap;
    QPointF home_pos;
    QGraphicsPixmapItem *frame;

signals:
    void toggle_discards();
    void clicked();
    void double_clicked();
    void thrown();
    void grabbed();
};

class GuanxingCardItem : public CardItem {
    Q_OBJECT

public:
    GuanxingCardItem(const Card *card);

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void released();
};

#endif // CARDITEM_H
