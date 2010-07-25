#include "pixmap.h"

#include <QPainter>
#include <QGraphicsColorizeEffect>

Pixmap::Pixmap(const QString &filename, bool center_as_origin):pixmap(filename)
{
    if(center_as_origin)
        setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);
}

QRectF Pixmap::boundingRect() const{
    return QRectF(0, 0, pixmap.width(), pixmap.height());
}

void Pixmap::changePixmap(const QString &filename){
    pixmap.load(filename);
}

void Pixmap::shift(){
    moveBy(-pixmap.width()/2, -pixmap.height()/2);
}

void Pixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawPixmap(0, 0, pixmap);
}

QVariant Pixmap::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == ItemSelectedChange){
        if(value.toBool()){
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
            effect->setColor(QColor(0xCC, 0x00, 0x00));
            setGraphicsEffect(effect);
        }else
            setGraphicsEffect(NULL);
    }else if(change == ItemEnabledChange){
        if(value.toBool()){
            setOpacity(1.0);
        }else{
            setOpacity(0.7);
        }
    }

    return QGraphicsObject::itemChange(change, value);
}
