#include "button.h"

extern audiere::AudioDevicePtr Device;

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

Button::Button(const QString &label)
    :label(label){

    QFontMetrics metrics(Config.BigFont);
    width = metrics.width(label);
    height = metrics.height();

    setFlags(QGraphicsItem::ItemIsFocusable);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    hover_effect = audiere::OpenSoundEffect(Device, "audio/button-hover.wav", audiere::MULTIPLE);
    down_effect = audiere::OpenSoundEffect(Device, "audio/button-down.wav", audiere::MULTIPLE);
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    setFocus(Qt::MouseFocusReason);

    hover_effect->play();
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    down_effect->play();

    emit clicked();
}

QRectF Button::boundingRect() const{
    return QRectF(-width/2 -2, -height/2 -8, width + 10, height + 10);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setFont(Config.BigFont);
    qreal font_size = Config.BigFont.pixelSize();

    if(hasFocus()){
        painter->setPen(QPen(Qt::black));
        painter->drawText(QPointF(8 - width/2, 8 + font_size/2), label);

        painter->setPen(QPen(Qt::white));
        painter->drawText(QPointF(-2 - width/2,-2 + font_size/2),label);
    }else{
        painter->setPen(QPen(Qt::black));
        painter->drawText(QPointF(5 - width/2, 5 + font_size/2), label);

        painter->setPen(QPen(Qt::white));
        painter->drawText(QPointF(-width/2, font_size/2),label);
    }
}
