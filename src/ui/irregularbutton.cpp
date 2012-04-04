#include "irregularbutton.h"

#include <QBitmap>
#include <QPainter>

static inline QString MakePath(const QString &name, const QString &state){
    return QString("image/system/button/irregular/%1-%2.png").arg(name).arg(state);
}

IrregularButton::IrregularButton(const QString &name)
{
    state = Normal;

    normal.load(MakePath(name, "normal"));
    hover.load(MakePath(name, "hover"));
    down.load(MakePath(name, "down"));
    disabled.load(MakePath(name, "down"));

    QBitmap mask_bitmap(MakePath(name, "mask"));
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

void IrregularButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    changeState(Normal);
}

void IrregularButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    changeState(Down);
}

void IrregularButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    changeState(Normal);
    emit clicked();
}
