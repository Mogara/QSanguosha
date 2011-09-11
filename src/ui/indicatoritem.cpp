#include "indicatoritem.h"

#include <QPainter>
#include <QGraphicsBlurEffect>

IndicatorItem::IndicatorItem(const QPointF &start)
    :start(start), finish(start)
{
    setGraphicsEffect(new QGraphicsBlurEffect);
}

QPointF IndicatorItem::getFinish() const{
    return finish;
}

void IndicatorItem::setFinish(const QPointF &finish){
    this->finish = finish;
    prepareGeometryChange();
}

void IndicatorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QPen pen(Qt::yellow);
    pen.setWidthF(8);

    painter->setPen(pen);
    painter->drawLine(mapFromScene(start), mapFromScene(finish));
}

QRectF IndicatorItem::boundingRect() const{
    qreal width = qAbs(start.x() - finish.x());
    qreal height = qAbs(start.y() - finish.y());

    return QRectF(0, 0, width, height);
}
