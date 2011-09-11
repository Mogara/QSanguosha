#ifndef INDICATORITEM_H
#define INDICATORITEM_H

#include <QGraphicsObject>

class IndicatorItem: public QGraphicsObject{
    Q_OBJECT
    Q_PROPERTY(QPointF finish READ getFinish WRITE setFinish);

public:
    IndicatorItem(const QPointF &start);

    QPointF getFinish() const;
    void setFinish(const QPointF &finish);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


    virtual QRectF boundingRect() const;

private:
    QPointF start, finish;
};

#endif // INDICATORITEM_H
