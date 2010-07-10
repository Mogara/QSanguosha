#ifndef CARDITEM_H
#define CARDITEM_H

#include "card.h"

#include <QGraphicsObject>
#include <QSize>
#include <QPropertyAnimation>

class CardItem : public QGraphicsObject
{
    Q_OBJECT
public:
    CardItem(Card *card);

    const Card *getCard() const;
    void setHomePos(QPointF home_pos);
    void goBack(bool kieru = false);
    void viewAs(const QString &name);
    const QPixmap &getSuitPixmap() const;

    void select();
    void unselect();

    virtual QRectF boundingRect() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    Card *card;
    QPixmap pixmap;
    QPixmap suit_pixmap;
    QPointF home_pos;
    QGraphicsPixmapItem *view_card_item;

signals:
    void show_discards();
    void hide_discards();
};

#endif // CARDITEM_H
