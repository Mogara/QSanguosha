#include "photoback.h"

Photo::Photo(QObject *parent) :
    QGraphicsObject(parent)
{
}

QRectF Photo::boundingRect() const{
    return QRectF(-photo_back.width()/2, -photo_back.height()/2, photo_back.width(), photo_back.height());
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->
}
