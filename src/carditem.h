#ifndef CARDITEM_H
#define CARDITEM_H

#include "card.h"
#include "pixmap.h"

#include <QSize>
#include <QPropertyAnimation>

class CardItem : public Pixmap
{
    Q_OBJECT
public:
    CardItem(const Card *card);

    void setRealCard(const Card *real_card);
    const Card *getRealCard() const;

    const Card *getCard() const;
    void setHomePos(QPointF home_pos);
    void goBack(bool kieru = false);
    const QPixmap &getSuitPixmap() const;
    const QPixmap &getIconPixmap() const;

    void select();
    void unselect();
    bool isPending() const;
    bool isEquipped() const;

    virtual QRectF boundingRect() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);    

private:
    const Card *card, *real_card;
    QPixmap suit_pixmap, icon_pixmap;
    QPointF home_pos;

signals:
    void show_discards();
    void hide_discards();
    void clicked();
    void double_clicked();
    void thrown();
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
