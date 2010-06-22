#include "pixmap.h"

#include <QPainter>

Pixmap::Pixmap(const QString &filename):pixmap(filename)
{
    setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);
}

QRectF Pixmap::boundingRect() const{
    return QRectF(0, 0, pixmap.width(), pixmap.height());
}

void Pixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawPixmap(0, 0, pixmap);
}
