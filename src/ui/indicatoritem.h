#ifndef _INDICATOR_ITEM_H
#define _INDICATOR_ITEM_H

#include "player.h"

#include <QGraphicsObject>

class IndicatorItem: public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF finish READ getFinish WRITE setFinish)

public:
    IndicatorItem(const QPointF &start, const QPointF &real_finish, Player *from);
    void doAnimation();

    QPointF getFinish() const;
    void setFinish(const QPointF &finish);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual QRectF boundingRect() const;

private:
    QPointF start, finish, real_finish;
    QColor color;
    qreal width;
};

#endif

