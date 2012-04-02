#include "irregularbutton.h"

#include <QBitmap>
#include <QPainter>

IrregularButton::IrregularButton(const QString &name)
{
    state = Normal;

    normal.load(QString("button/irregular/%1-normal.png").arg(name));
    hover.load(QString("button/irregular/%1-hover.png").arg(name));
    down.load(QString("button/irregular/%1-down.png").arg(name));
    disabled.load(QString("button/irregular/%1-disabled.png").arg(name));

    QBitmap mask_bitmap(QString("button/irregular/%1-mask.png").arg(name));
    mask = QRegion(mask_bitmap);

    setAcceptsHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF IrregularButton::boundingRect() const{
    return QRectF(normal.rect());
}

void IrregularButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QPixmap *to_draw = NULL;
    if(!isEnabled())
        to_draw = &disabled;
    else{
        switch(state){
        case Normal: to_draw = &normal; break;
        case Hover: to_draw = &hover; break;
        case Down: to_draw = &down; break;
        }
    }

    if(to_draw)
        painter->drawPixmap(0, 0, *to_draw);
}

void IrregularButton::changeState(IrregularButton::State state){
    if(this->state != state){
        this->state = state;
        update();
    }
}

void IrregularButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    changeState(Hover);
}

void IrregularButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    changeState(Down);
}

void IrregularButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    changeState(Normal);
    emit clicked();
}
